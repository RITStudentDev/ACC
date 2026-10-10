#include "listener.h"

#define BUFFER_SIZE = 2048

int HTTP_Listener::init_socket(){
    int socket_fd;
    int opt = 1;
    struct sockaddr_in address;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket creation on listener failed.");
        return EXIT_FAILURE;
    }
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("Failed to set socket opt.");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("Failed bind.");
        close(socket_fd);
        return EXIT_FAILURE;
    }
    if (listen(socket_fd, 10) < 0){
        perror("Failed Listen.");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    while(true){
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0){
            perror("Failed to accept connection.");
            close(client_fd);
            continue;
        }
    }
    close(socket_fd);
    return 0;
}