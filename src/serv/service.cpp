#include "service.h"
#include <print>

/**
 * This file contains the implementations for the content of the service header.
 */
HTTP_Server::HTTP_Server() :
    listener_(HTTP_Listener()),
    parser_(std::move(HTTP_Parser()))
    {
        std::println("Server in construction"); // Debug
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
    std::println("Reached start server"); // Debug
    this->init_epoll();
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
    auto it = conns_.find(fd);
    if (it == conns_.end()) return -1;
    std::string& buf = it->second;

    char tmp[4096];
    for (;;){
        int n = recv(fd, tmp, sizeof(tmp), 0);
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

    while (!buf.empty()){
        Request req;
        int used = parser_.parse_request(buf, &req);
        if (used == -1) return 0;
        else return -1;

        buf.erase(0, used);
    }
    return 0;
}

int HTTP_Server::closeConn(int fd){
    close(fd);
    return 0;
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