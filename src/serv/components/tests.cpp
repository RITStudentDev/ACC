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
    std::printf("Method: ", req.method);
    std::printf("Target: ", req.target);
    std::printf("Minor: ", req.minor);
    std::printf("Num headers: ", req.n_headers);
    for (int i = 0; i < req.n_headers; ++i){
        std::printf("Header: ", req.headers[i]);
    }


    return 0;
}