#pragma once


struct HTTPMsg{
    // Method
    // Headers
    // Body - optional
    // Route
};

class HTTP_Parser{
    public:
        HTTP_Parser() = default;

    private:
        int parse();
        // Helpers to read deserialized 
        bool isDigit(char c);
        bool isAlpha(char c);
        bool isTchar(char c);

};