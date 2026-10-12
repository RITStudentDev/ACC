#pragma once

#include <cstring>
#include <string>
#include <string_view>

struct Header{
    std::string name;
    std::string value;
};

class HTTP_Parser{
    public:
        HTTP_Parser() = default;

        int parse();

    private:
        // Helpers to read deserialized message
        constexpr bool isDigit(char c);
        constexpr bool isAlpha(char c);
        constexpr bool isTchar(char c);
        constexpr bool isVchar(char c);
        bool consume(std::string_view in, std::string_view x);

        // Functions to parse the message
        int parse_method(std::string_view in);
        int parse_target(std::string_view in);
        int parse_version(std::string_view in, int* minor);
        int parse_header(std::string_view in, Header* h);
        int parse_header_list(std::string_view in, Header* h,int max_headers);


};