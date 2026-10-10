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