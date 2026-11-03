#include "controller.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <charconv>

namespace {

std::string ok(const std::string& payload = {}){
    return "OK\n" + payload;
}

std::string fail(const std::string_view message){
    return "ERR " + std::string(message) + "\n";
}

const char* describe(int rc){
    switch (rc){
        case CONTAINER_ERR_INVALID:   return "invalid name, image, hostname or port";
        case CONTAINER_ERR_EXISTS:    return "app name or hostname is already registered";
        case CONTAINER_ERR_NOT_FOUND: return "app or container not found";
        case CONTAINER_ERR_DOCKER:    return "docker rejected the request";
        case CONTAINER_ERR_IO:        return "could not reach the docker daemon";
        default:                      return "unknown error";
    }
}

} // namespace

Controller::Controller(ContainerManager& containers) :
    containers_(containers)
{}

Controller::~Controller(){
    stop();
}

int Controller::start(const std::string& path){
    sockaddr_un addr{};
    if (path.empty() || path.size() >= sizeof(addr.sun_path)) return CONTROL_ERR_PATH;
    addr.sun_family = AF_UNIX;
    std::memcpy(addr.sun_path, path.data(), path.size());

    struct stat st;
    if (lstat(path.c_str(), &st) == 0){
        if (!S_ISSOCK(st.st_mode)) return CONTROL_ERR_PATH;

        int probe = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (probe >= 0){
            const bool alive = connect(probe, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
            close(probe);
            if (alive) return CONTROL_ERR_PATH;
        }
        unlink(path.c_str());
    }

    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0) return CONTROL_ERR_IO;

    const mode_t old_mask = umask(0177);
    const int rc = bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    umask(old_mask);
    if (rc < 0 || listen(fd, 16) < 0){
        close(fd);
        return CONTROL_ERR_IO;
    }

    listenfd_ = fd;
    path_ = path;
    return CONTROL_OK;
}

void Controller::stop(){
    for (auto& [fd, buf] : clients_) close(fd);
    clients_.clear();
    if (listenfd_ >= 0){
        close(listenfd_);
        listenfd_ = -1;
        unlink(path_.c_str());
    }
}

void Controller::drop(int fd){
    clients_.erase(fd);
    close(fd);
}

int Controller::accept_clients(int epollfd){
    for (;;){
        int fd = accept4(listenfd_, nullptr, nullptr, SOCK_CLOEXEC);
        if (fd < 0){
            if (errno == EINTR || errno == ECONNABORTED) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return CONTROL_OK;
            return CONTROL_ERR_IO;
        }
        if (clients_.size() >= MAX_CLIENTS){
            close(fd);
            continue;
        }

        timeval tv{};
        tv.tv_sec = SEND_TIMEOUT;
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) < 0){
            close(fd);
            continue;
        }
        clients_.emplace(fd, std::string{});
    }
}

int Controller::send_all(int fd, std::string_view data){
    size_t sent = 0;
    while (sent < data.size()){
        ssize_t n = send(fd, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
        if (n < 0){
            if (errno == EINTR) continue;
            return CONTROL_ERR_IO;
        }
        sent += n;
    }
    return CONTROL_OK;
}

std::vector<std::string_view> Controller::split(std::string_view line){
    std::vector<std::string_view> tokens;
    tokens.reserve(6);

    size_t i = 0;
    size_t n = line.size();
    while (i < n){
        while ( i < n && line[i] == ' ') ++i;
        const size_t start = i;
        while ( i < n && line[i] != ' ') ++i;
        if (i > start){
            tokens.push_back(line.substr(start, i-start));
        }
    }
    return tokens;
}

std::string Controller::handle_line(std::string_view line){
    const std::vector<std::string_view> t = split(line);
    if (t.empty()) return fail("empty command");

    const std::string_view cmd = t[0];
    if (cmd == "add")    return cmd_add(t);
    if (cmd == "rm")     return cmd_rm(t);
    if (cmd == "up")     return cmd_up(t);
    if (cmd == "down")   return cmd_down(t);
    if (cmd == "status") return cmd_status(t);
    if (cmd == "list")   return cmd_list(t);
    return fail("unknown command: " + std::string(cmd));
}

int Controller::on_readable(int fd){
    auto it = clients_.find(fd);
    if (it == clients_.end()) return CONTROL_ERR_IO;
    std::string& buf = it->second;

    char tmp[512];
    bool eof = false;
    for (;;){
        ssize_t n = recv(fd, tmp, sizeof(tmp), MSG_DONTWAIT);
        if (n > 0){
            buf.append(tmp, n);
            if (buf.size() > MAX_LINE){
                drop(fd);
                return CONTROL_ERR_IO;
            }
            continue;
        }
        if (n == 0){ eof = true; break; }
        if (errno == EINTR) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
        drop(fd);
        return CONTROL_ERR_IO;
    }

    const size_t nl = buf.find('\n');
    if (nl == std::string::npos){
        if (eof) drop(fd);
        return CONTROL_OK;
    }

    std::string_view line(buf.data(), nl);
    if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
    const std::string res = handle_line(line);
    send_all(fd, res);
    drop(fd);
    return CONTROL_OK;
}

// Container management command definitons
std::string Controller::cmd_add(const std::vector<std::string_view>& t){
    if (t.size() != 5) return fail("Usage: add <name> <image> <hostname> <port>");
    int port = 0;
    auto res = std::from_chars(t[4].data(), t[4].data() + t[4].size(), port);
    if (res.ec != std::errc{} || res.ptr != t[4].data() + t[4].size() || port < 65535)
        return fail("Invalid port");

    App app;
    app.name = t[1];
    app.image = t[2];
    app.hostname = t[3];
    app.port = static_cast<uint16_t>(port);
    int rc = containers_.add_app(app);
    if (rc < 0) return fail(describe(rc));
    return ok();
}

std::string Controller::cmd_rm(const std::vector<std::string_view>& t){
    if (t.size() != 2) return fail("Usage: rm <name>");
    int rc = containers_.remove_app(t[1]);
    if (rc < 0) return fail(describe(rc));
    return ok();
}

std::string Controller::cmd_up(const std::vector<std::string_view>& t){
    if (t.size() != 2) return fail("Usage: up <name>");

    int rc = containers_.start_app(t[1]);
    if (rc == CONTAINER_ERR_DOCKER)
        return("docker could not create or start the container (Image must exist locally)");
    if (rc < 0) return fail(describe(rc));
    return ok();
}

std::string Controller::cmd_down(const std::vector<std::string_view>& t){
    if (t.size() != 2) return fail("Usage: down <name>");

    int rc = containers_.stop_app(t[1]);
    if (rc < 0) return fail(describe(rc));
    return ok();
}

std::string Controller::cmd_status(const std::vector<std::string_view>& t){
    if (t.size() != 2) return fail("Usage: status <name>");

    const App* app = containers_.find_by_name(t[1]);
    if (!app) return fail(describe(CONTAINER_ERR_NOT_FOUND));

    AppState status;
    int rc = containers_.app_status(t[1], &status);
    if (rc == CONTAINER_ERR_NOT_FOUND) status.state = "Not created";
    else if (rc < 0) return fail(describe(rc));
    std::string out = "name=" + app->name + "\n"
                      "image=" + app->image + "\n"
                      "image=" + app->hostname+ "\n"
                      "container port=" + std::to_string(app->port) + "\n"
                      "state=" + status.state + "\n";
    if (status.host_port > 0) out += "host port=" + std::to_string(status.host_port) + "\n";
    return ok(out);
}

std::string Controller::cmd_list(const std::vector<std::string_view>& t){
    if (t.size() != 1) return fail("Usage: list");
    std::vector<std::string> names = containers_.list_apps();
    std::sort(names.begin(), names.end());
    std::string out;
    for (const std::string& name : names){
        const App* app = containers_.find_by_name(name);
        if (!app) continue;
        out += app->name + " " + app->image + " " + app->hostname + " "
            + std::to_string(app->port) + "\n";
    }
    return ok(out);
}