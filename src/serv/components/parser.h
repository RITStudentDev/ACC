#pragma once

#include <string>

struct HTTPMsg{
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
        int parse_method(std::string_view method);

};