#pragma once

#include <cstring>
#include <string>
#include <string_view>

#define MAX_HEADERS 128
#define COUNT(X) (sizeof(X) / sizeof((X[0])))

enum ParseStatus {
    PARSE_OK = 0,
    PARSE_INCOMPLETE = -1,
    PARSE_MALFORMED  = -2,
};

struct Header{
    std::string name;
    std::string value;
};

struct Request{
    std::string_view method;
    std::string_view target;
    Header headers[MAX_HEADERS];
    int minor;
    int n_headers;
};

struct Response{
    int minor;
    int status;
    Header headers[MAX_HEADERS];
    int n_headers;
    std::string_view body;
};

class HTTP_Parser{
    public:
        HTTP_Parser() = default;

        int parse_request(std::string_view in, Request* req);
        int parse_response(std::string_view raw, Response* res);

    private:
        // Helpers to read deserialized message
        static bool isDigit(char c);
        static bool isAlpha(char c);
        static bool isTchar(char c);
        static bool isVchar(char c);
        static bool consume(std::string_view& in, std::string_view x);

        // Functions to parse the message
        int parse_method(std::string_view& in, std::string_view* method);
        int parse_target(std::string_view& in, std::string_view* target);
        int parse_version(std::string_view& in, int* minor);
        int parse_status(std::string_view& in, int* minor, int* status);
        int parse_header(std::string_view& in, Header* h);
        int parse_header_list(std::string_view& in, Header* h, int max_headers);
};