#include "cont_client.h"
#include "parser.h"

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

ContainerClient::ContainerClient(std::string socket_path) :
    socket_path_(std::move(socket_path))
    {}

int ContainerClient::connect_socket() const {
    sockaddr_un addr{};
    if (socket_path_.size() >= sizeof(addr.sun_path)) return CONTAINER_ERR_IO;
    addr.sun_family = AF_UNIX;
    std::memcpy(addr.sun_path, socket_path_.data(), socket_path_.size());

    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return CONTAINER_ERR_IO;

    timeval tv{};
    tv.tv_sec = TIMEOUT;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    for (;;){
        if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0) return fd;
        if (errno == EINTR) continue;
        close(fd);
        return CONTAINER_ERR_IO;
    }
}

int ContainerClient::send_all(int fd, std::string_view data){
    size_t sent = 0;
    while (sent < data.size()){
        ssize_t n = send(fd, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
        if (n < 0){
            if (errno == EINTR) continue;
            return CONTAINER_ERR_IO;
        }
        sent += n;
    }
    return CONTAINER_OK;
}

int ContainerClient::read_all(int fd, std::string* out){
    char tmp[4096];
    for (;;){
        ssize_t n = recv(fd, tmp, sizeof(tmp), 0);
        if (n > 0){
            out->append(tmp, n);
            if (out->size() > MAX_RESPONSE) return CONTAINER_ERR_IO;
            continue;
        }
        if (n == 0) return CONTAINER_OK;
        if (errno == EINTR) continue;
        return CONTAINER_ERR_IO;
    }
}

int ContainerClient::parse_response(std::string_view raw, ContainerResponse* out){
    HTTP_Parser parser;
    Response res;
    if (parser.parse_response(raw, &res) < 0) return CONTAINER_ERR_PROTO;

    out->status = res.status;
    out->body.assign(res.body);
    return CONTAINER_OK;
}

int ContainerClient::request(std::string_view method, std::string_view path, std::string_view body, ContainerResponse* res){
    std::string req;
    req.append(method).append(" ").append(path).append(" HTTP/1.0\r\n");
    req.append("Host: docker\r\n");
    if (!body.empty()) req.append("Content-Type: application/json\r\n");
    req.append("Content-Length: ").append(std::to_string(body.size())).append("\r\n");
    req.append("\r\n");
    req.append(body);

    int fd = connect_socket();
    if (fd < 0 ) return fd;

    std::string raw;
    int rc = send_all(fd, req);
    if (rc == CONTAINER_OK) rc = read_all(fd, &raw);
    close(fd);
    if (rc < 0) return rc;
    return parse_response(raw, res);
}