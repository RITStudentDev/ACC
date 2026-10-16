#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <thread>

#include <print>

/**
 * This file contains the declarations for the HTTP listener.
 * Its purpose is to handle incoming TCP connections and andle input of HTTP traffic.
 */

class HTTP_Listener{
    public:
        HTTP_Listener(){
            if(init_socket() < 0)perror("Init_socket:");
            else std::println("Started listening");
        }

        ~HTTP_Listener(){
            stop();
        }

        HTTP_Listener& operator=(const HTTP_Listener&& origin){
            if (this == &origin){
                return *this;
            }
        }

        int socket_fd_ = -1;
        std::thread l_thread_;

        int init_socket();
        void stop();
        int listen_loop();
};