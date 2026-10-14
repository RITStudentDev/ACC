#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <cerrno>
#include <unordered_map>
#include <string>
#include "listener.h"
#include "parser.h"

/**
 * This header contains the declarations for the hosting service.
 * Definitions for the content of this file are included in service.cpp.
 * This file is used in the ACC daemon to persistantly run the web hosting service.
 */
class HTTP_Server{
    public:
        HTTP_Server();

        int start_server();
        int run_loop();
    private:
        bool running_ = false;
        int listenfd_;
        int epollfd_;
        std::unordered_map<int, std::string> conns_;
        HTTP_Listener listener_;
        HTTP_Parser parser_;

        int init_epoll();
        int onAccept();
        int onRecv(int fd);
        int closeConn(int fd);
};
