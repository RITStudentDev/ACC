#include "./serv/service.h"
#include <print>

int main(){
    std::println("Starting daemon.");
    HTTP_Server server;
    std::println("Constructed server.");
    if(server.start_server() < 0) std::println("Failed to start server.");
    std::println("Server started.");
    server.run_loop();
    std::println("Shut down.");
    return 0;
}