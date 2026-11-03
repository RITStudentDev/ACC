#pragma once
#include <string>
#include <cstdlib>

inline std::string control_socket_path(){
    const char* env = std::getenv("ACC_SOCKET");
    return (env && *env) ? env : "/run/acc.sock";
}