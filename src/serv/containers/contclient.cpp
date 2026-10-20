#include "contclient.h"

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