#include "listener.h"

#define BUFFER_SIZE = 2048

// Creates a listening socket on the server
int HTTP_Listener::init_socket(){
    int socket_fd;
    int opt = 1;
    struct sockaddr_in address;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket creation on listener failed");
        return EXIT_FAILURE;
    }
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("Failed to set socket opt");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("Failed bind");
        close(socket_fd);
        return EXIT_FAILURE;
    }
    if (listen(socket_fd, 10) < 0){
        perror("Failed Listen");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    socket_fd_ = socket_fd;
    l_thread_ = std::thread{&HTTP_Listener::listen_loop, this};
    return 0;
}

int HTTP_Listener::listen_loop(){
    while (true){
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int client_fd = accept(socket_fd_, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0){
            if (errno == EINTR || errno == ECONNABORTED) continue;
            break; 
        }
        continue;
    }
    return 0;
}

void HTTP_Listener::stop(){
    shutdown(socket_fd_, SHUT_RDWR);
    if (l_thread_.joinable()) l_thread_.join();
    close(socket_fd_);
    socket_fd_ = -1;
}