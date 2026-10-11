#pragma once

#include <string>

struct HTTP_msg{
    // Method
    // Headers
    // Body - optional
    // Route
};

class HTTP_Parser{
    public:
        HTTP_Parser() = default;

        int parse();

    private:
        // Helpers to read deserialized 
        constexpr bool isDigit(char c);
        constexpr bool isAlpha(char c);
        constexpr bool isTchar(char c);

        // Determines parsing method
        int parse_method(std::string_view in);
        // Determines the target of the request
        int parse_target(std::string_view in);

};