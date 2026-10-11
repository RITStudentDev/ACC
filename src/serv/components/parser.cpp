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

int HTTP_Parser::parse_method(std::string_view in){
    const int len =  in.size();
    int i = 0;

    while(i < len && isTchar(in[i])) ++i;
    
    if (i == len) return -1;
    if (i == 0 || in[i] != ' ') return -2;
    return i;
}

int HTTP_Parser::parse_target(std::string_view in){
    const auto* p = reinterpret_cast<const unsigned char*>(in.data());
    const int len = in.size();
    int i = 0;

    while(i < len && static_cast<unsigned char>(p[i] - 0x21) < 0x5E) ++i;
    if(i == len) return -1;
    if(i == 0 || p[i] != ' ') return -2;
    return i;

}
