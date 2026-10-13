#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <cerrno>
#include <unordered_map>
#include "listener.h"
#include "parser.h"

/**
 * This header contains the declarations for the hosting service.
 * Definitions for the content of this file are included in service.cpp.
 * This file is used in the ACC daemon to persistantly run the web hosting service.
 */
class HTTP_Server{
    public:
        HTTP_Server() = default;

        int start_server();
        int run_loop();
    private:
        bool running_ = false;
        int listenfd_;
        int epollfd_;
        HTTP_Listener listener_;
        HTTP_Parser parser_;

        int init_epoll();
        int onRecv(int fd);
        int closeConn(int fd);
};
