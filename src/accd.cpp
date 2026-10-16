#include "./serv/service.h"
#include <print>

int main(){
    std::println("Starting daemon.");
    HTTP_Server server;
    std::println("Constructed server.");
    if(server.start_server() < 0) std::println("Failed to start server.");
    else std::println("Server started.");

    return 0;
}