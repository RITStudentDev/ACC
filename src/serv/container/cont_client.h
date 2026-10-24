#pragma once

#include <string_view>
#include <string>

enum DockerStatus{
    DOCKER_OK = 0,
    DOCKER_ERR_IO = -1,
    DOCKER_ERR_PROTO = -2,
};

struct ContainerResponse{
    int status = 0;
    std::string body;
};

class ContainerClient{
    public:
        explicit ContainerClient(std::string socket_path = "/var/run/docker.sock");

        int request(
            std::string_view method, 
            std::string_view path, 
            std::string_view body, 
            ContainerResponse* out
        ) const;

    private:
        static constexpr int TIMEOUT = 10;
        static constexpr size_t MAX_RESPONSE = 4 * 1024 * 1024;

        std::string socket_path_;

        int connect_socket() const;
        static int send_all(int fd, std::string_view data);
        static int read_all(int fd, std::string* out);
        static int parse_response(std::string_view raw, ContainerResponse* res);
        int request(std::string_view method, std::string_view path, std::string_view body, ContainerResponse* res);
};