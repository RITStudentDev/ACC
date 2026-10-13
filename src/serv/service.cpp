#include "service.h"

/**
 * This file contains the implementations for the content of the service header.
 */
HTTP_Server::HTTP_Server() :
    listener_(HTTP_Listener()),
    parser_(HTTP_Parser())
    {}

int HTTP_Server::init_epoll(){
    epollfd_ = epoll_create1(0);
    if (epollfd_ < 0) return -1;

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd_;
    return epoll_ctl(epollfd_, EPOLL_CTL_ADD, listenfd_, &ev);
}

int HTTP_Server::start_server(){
    running_ = true;
    return 0;
}

int HTTP_Server::onAccept(){
    for (;;){
        int fd = accept4(listenfd_, nullptr, nullptr, SOCK_NONBLOCK);
        if (fd < 0) return -1;

        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = fd;
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) < 0){
            close(fd);
            continue;
        }
        conns_.emplace(fd, std::string{});
    }
    return 0;
}

int HTTP_Server::onRecv(int fd){
    char buf[4096];
    Request req;
    if(recv(fd, buf, sizeof(buf), 0) < 0) return -1;
    return parser_.parse_request(std::string_view(buf), &req);
}

int HTTP_Server::run_loop(){
    if (init_epoll() < 0) return -1;

    epoll_event events[128];
    while (running_){
        int n = epoll_wait(epollfd_, events, 128, -1);
        if (n < 0){
            if (errno == EINTR) continue;
            return -1;
        }
        for (int i = 0; i < n; ++i){
            int fd = events[i].data.fd;
            if (fd == listenfd_) onAccept();
            else if (events[i].events & (EPOLLERR | EPOLLHUP)) closeConn(fd);
            else if (events[i].events & EPOLLIN) if (onRecv(fd) < 0) closeConn(fd);
        }
    }
    return 0;
}