#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <string>

int http_ping(int fd, const std::string& host, const std::string& path){
    const std::string req = 
        "GET " + path + " HTTP/1.1\r\n" +
        "Host: " + host + "\r\n" +
        "Connection: close\r\n" + 
        "\r\n";
    size_t sent = 0;
    while (sent < req.size()){
        ssize_t n = send(fd, req.data() + sent, req.size() - sent, MSG_NOSIGNAL);
        if ( n < 0){
            if (errno == EINTR) continue;
            return -1;
        }
        sent += n;
    }
    return 0;
}