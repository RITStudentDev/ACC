#include "service.h"

/**
 * This file contains the implementations for the content of the service header.
 */
HTTP_Server::HTTP_Server() :
    listener_(HTTP_Listener()),
    parser_(HTTP_Parser())
    {}

int HTTP_Server::start_server(){
    running_ = true;
    std::thread l_thread{listener_.init_socket()};
    return 0;
}

int HTTP_Server::onRecv(){
    Request req;
    parser_.parse_request("temp", &req); // filler
}

int HTTP_Server::run_loop(){
    while (running_){
        onRecv();
    }
    return 0;
}