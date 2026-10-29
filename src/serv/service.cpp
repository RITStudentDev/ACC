#include "service.h"
#include <print>

/**
 * This file contains the implementations for the content of the service header.
 */

 
namespace {

bool iequals(std::string_view a, std::string_view b){
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i])) return false;
    return true;
}

std::string_view host_of(const Request& req){
    for (int i = 0; i < req.n_headers; ++i){
        if (!iequals(req.headers[i].name, "host")) continue;
        std::string_view h = req.headers[i].value;
        return h.substr(0, h.find(':'));
    }
    return {};
}

} // namespace

HTTP_Server::HTTP_Server() :
    listener_(HTTP_Listener()),
    parser_(std::move(HTTP_Parser())),
    containers_()
    {}
HTTP_Server::~HTTP_Server(){
    clean();
}

int HTTP_Server::init_epoll(){
    epollfd_ = epoll_create1(0);
    if (epollfd_ < 0) return -1;

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd_;
    return epoll_ctl(epollfd_, EPOLL_CTL_ADD, listenfd_, &ev);
}

int HTTP_Server::start_server(){
    listenfd_ = listener_.init_socket();
    if (listenfd_ < 0) return -1;
    if (init_epoll() < 0) return -1;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigprocmask(SIG_BLOCK, &set, nullptr);
    sigfd_ = signalfd(-1, &set, SFD_NONBLOCK);
    if (sigfd_ < 0) return -1;

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = sigfd_;
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, sigfd_, &ev) < 0) return -1;

    running_ = true;
    return 0;
}

int HTTP_Server::onAccept(){
    for (;;){
        int fd = accept4(listenfd_, nullptr, nullptr, SOCK_NONBLOCK);
        if (fd < 0){
            if (errno == EINTR || errno == ECONNABORTED) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
            return -1;
        }

        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = fd;
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) < 0){
            close(fd);
            continue;
        }
        conns_.emplace(fd, std::string{});
    }
}

int HTTP_Server::onRecv(int fd){
    auto it = conns_.find(fd);
    if (it == conns_.end()) return -1;
    std::string& buf = it->second;

    char tmp[4096];
    for (;;){
        ssize_t n = recv(fd, tmp, sizeof(tmp), 0);
        if (n > 0){
            buf.append(tmp, n);
            if (buf.size() > 64 * 1024) return -1;
            continue;
        }
        if (n == 0) return -1;
        if (errno == EINTR) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
        return -1;  
    }
    
    std::println("Received bytes={} fd={}", buf.size(), fd);
    Request req;
    int rc = parser_.parse_request(buf, &req);
    if (rc == PARSE_INCOMPLETE) return 0;
    if (rc < 0) return -1;

    const App* app = containers_.find_by_host(host_of(req));
    if (!app){
        send_status(fd, 404, "Not Found");
        return -1;
    }

    send_response(fd);
    return -1;
}

int HTTP_Server::closeConn(int fd){
    conns_.erase(fd);
    close(fd);
    return 0;
}

void HTTP_Server::clean(){
    for (auto& [fd, buf] : conns_) close(fd);
    conns_.clear();
    if (sigfd_ >= 0)   { close(sigfd_);   sigfd_ = -1; }
    if (epollfd_ >= 0) { close(epollfd_); epollfd_ = -1; }
    listener_.stop();
}

int HTTP_Server::run_loop(){
    epoll_event events[128];
    while (running_){
        int n = epoll_wait(epollfd_, events, 128, -1);
        if (n < 0){
            if (errno == EINTR) continue;
            return -1;
        }
        for (int i = 0; i < n; ++i){
            int fd = events[i].data.fd;
            uint32_t ev = events[i].events;

            if (fd == sigfd_) running_ = false;
            else if (fd == listenfd_) onAccept();
            else if (ev & (EPOLLIN | EPOLLERR | EPOLLHUP)) if (onRecv(fd) < 0) closeConn(fd);
        }
    }
    return 0;
}

void HTTP_Server::send_response(int fd){
    static const char res[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 3\r\n"
        "Connection: close\r\n"
        "\r\n"
        "OK\n";
    send(fd, res, sizeof(res) - 1, MSG_NOSIGNAL);
}

void HTTP_Server::send_status(int fd, int code, const char* r){
    std::string res = "HTTP/1.1 " + std::to_string(code) + " " + r +"\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n";
    send(fd, res.data(), res.size(), MSG_NOSIGNAL);
}