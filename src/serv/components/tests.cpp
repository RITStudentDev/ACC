#include <cstdio>
#include "parser.h"

int main(){
    std::string msg = "GET /index.html HTTP/1.1\r\nHost: coz.is\r\nUser-Agent: curl/7.81.0\r\n\r\n";
    Request req;

    HTTP_Parser parser{};
    if (parser.parse_request(msg, &req) < 0){
        std::printf("Parsing failed");
        return -1;
    }
    std::printf("Method: %.*s\n", (int)req.method.size(), req.method.data());
    std::printf("Target: %.*s\n", (int)req.target.size(), req.target.data());
    std::printf("Minor: %d\n", req.minor);
    std::printf("Num headers: %d\n", req.n_headers);
    for (int i = 0; i < req.n_headers; ++i){
        std::printf("Header: %s: %s\n", req.headers[i].name.c_str(), req.headers[i].value.c_str());
    }


    return 0;
}