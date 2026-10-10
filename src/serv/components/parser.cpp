#include "parser.h"

bool HTTP_Parser::isDigit(char c){
    return c >= '0' && c <= '9';
}
bool HTTP_Parser::isAlpha(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
bool HTTP_Parser::isTchar(char c){
    return c == '!' || c == '#' || c == '$' || c == '%' || c == '&'
        || c == '\'' || c == '*' || c == '+' || c == '-' || c == '.'
        || c == '^' || c == '_' || c == '`' || c == '|' || c == '~'
        || isDigit(c) || isAlpha(c);
}

int HTTP_Parser::parse_method(std::string_view method){
    const size_t len =  method.size();
    size_t i = 0;

    while(i < len && isTchar(method[i])) ++i;
    
    if (i == len) return -1;
    if (i == 0 || method[i] != ' ') return -2;
    return static_cast<int>(i);
}
