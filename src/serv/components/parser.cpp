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
bool HTTP_Parser::isVchar(char c){
    return c >= ' ' && c <= '~';
}

bool HTTP_Parser::consume(std::string_view in, std::string_view x){
    return !x.empty() && x.size() < in.size() && std::memcmp(in.data(), x.data(), x.size()) == 0;
}

int HTTP_Parser::parse_method(std::string_view in){
    const int len =  in.size();
    int i = 0;

    while(i < len && isTchar(in[i])) ++i;
    
    if (i == len) return -1;
    if (i == 0 || in[i] != ' ') return -2;
    return 0;
}

int HTTP_Parser::parse_target(std::string_view in){
    const auto* p = reinterpret_cast<const unsigned char*>(in.data());
    const int len = in.size();
    int i = 0;

    while(i < len && static_cast<unsigned char>(p[i] - 0x21) < 0x5E) ++i;
    if(i == len) return -1;
    if(i == 0 || p[i] != ' ') return -2;
    return 0;
}

int HTTP_Parser::parse_version(std::string_view in, int* minor){
    if (consume(in, "HTTP/1.0\r\n")){
        *minor = 0;
        return 0;
    }
    if (consume(in, "HTTP/1.1\r\n")){
        *minor = 1;
        return 0;
    }
    return -1;
}

int HTTP_Parser::parse_header(std::string_view in, Header* h){
    const int len = in.size();
    int i = 0;

    while(i < len && isTchar(in[i])) ++i;
    if (i == len) return -1;
    if (i == 0 || in[i] != ':') return -2;
    const int n_end = ++i;

    while (i < len && (in[i] == ' ' || in[i] == '\t')) ++i;
    const int v_start = i;
    int v_end = i;
    while (i < len){
        const char c = in[i];
        if (isVchar(c)) v_end = ++i;
        else if (c == ' ' || c == '\t') ++i;
        else break;
    }

    if (i == len) return -1;
    if (in[i] != '\r') return -2;
    if (i+1 == len) return -1;
    if (in[i+1] != '\n') return -2;
    h->name = in.substr(0, n_end);
    h->value = in.substr(v_start, v_end - v_start);
    return 0;
}