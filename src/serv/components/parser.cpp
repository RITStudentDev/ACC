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

bool HTTP_Parser::consume(std::string_view& in, std::string_view x){
    if (in.substr(0, x.size()) == x){
        in.remove_prefix(x.size());
        return true;
    }
    return false;
}

int HTTP_Parser::parse_method(std::string_view& in, std::string_view* method){
    const int len = in.size();
    int i = 0;

    while (i < len && isTchar(in[i])) ++i;
    if (i == len) return PARSE_INCOMPLETE;
    if (i == 0 || in[i] != ' ') return PARSE_MALFORMED;

    *method = in.substr(0, i);
    in.remove_prefix(i + 1);
    return PARSE_OK;
}

int HTTP_Parser::parse_target(std::string_view& in, std::string_view* target){
    const int len = in.size();
    int i = 0;

    while (i < len && in[i] > ' ' && in[i] <= '~') ++i;
    if (i == len) return PARSE_INCOMPLETE;
    if (i == 0 || in[i] != ' ') return PARSE_MALFORMED;

    *target = in.substr(0, i);
    in.remove_prefix(i + 1);
    return PARSE_OK;
}

int HTTP_Parser::parse_version(std::string_view& in, int* minor){
    if (consume(in, "HTTP/1.0\r\n")){
        *minor = 0;
        return PARSE_OK;
    }
    if (consume(in, "HTTP/1.1\r\n")){
        *minor = 1;
        return PARSE_OK;
    }
    return PARSE_INCOMPLETE;
}

int HTTP_Parser::parse_header(std::string_view& in, Header* h){
    const int len = in.size();
    int i = 0;

    while (i < len && isTchar(in[i])) ++i;
    if (i == len) return PARSE_INCOMPLETE;
    if (i == 0 || in[i] != ':') return PARSE_MALFORMED;
    const int n_end = i;
    ++i;

    while (i < len && (in[i] == ' ' || in[i] == '\t')) ++i;
    const int v_start = i;
    int v_end = i;
    while (i < len){
        const char c = in[i];
        if (isVchar(c) && c != ' ') v_end = ++i;
        else if (c == ' ' || c == '\t') ++i;
        else break;
    }

    if (i == len) return PARSE_INCOMPLETE;
    if (in[i] != '\r') return PARSE_MALFORMED;
    if (i + 1 == len) return PARSE_INCOMPLETE;
    if (in[i + 1] != '\n') return PARSE_MALFORMED;

    h->name = in.substr(0, n_end);
    h->value = in.substr(v_start, v_end - v_start);
    in.remove_prefix(i + 2);
    return PARSE_OK;
}

int HTTP_Parser::parse_header_list(std::string_view& in, Header* h, int max_headers){
    int n_headers = 0;
    while (!consume(in, "\r\n")){
        if (n_headers == max_headers) return PARSE_MALFORMED;
        int rc = parse_header(in, &h[n_headers]);
        if (rc < 0) return rc;
        n_headers++;
    }
    return n_headers;
}

int HTTP_Parser::parse_status(std::string_view& in, int* minor, int* status){
    if (consume(in, "HTTP/1.0 ")) *minor = 0;
    else if (consume(in, "HTTP/1.1 ")) *minor = 1;
    else return in.size() < 9 ? PARSE_INCOMPLETE : PARSE_MALFORMED;

    const int len = in.size();
    if (len < 4) return PARSE_INCOMPLETE;
    if (!isDigit(in[0]) || !isDigit(in[1]) || !isDigit(in[2]) || in[3] != ' ') return PARSE_MALFORMED;
    const int code = (in[0] - '0') * 100 + (in[1] - '0') * 10 + (in[2] -  '0');

    int i = 4;
    while (i < len && (isVchar(in[i]) || in[i] == '\t')) ++i;
    if (i == len) return PARSE_INCOMPLETE;
    if (in[i] != '\r') return PARSE_MALFORMED;
    if (i + 1 == len) return PARSE_INCOMPLETE;
    if (in[i + 1] != '\n') return PARSE_MALFORMED;

    *status = code;
    in.remove_prefix(i + 2);
    return PARSE_OK;
}

int HTTP_Parser::parse_request(std::string_view in, Request* req){
    int rc;
    if ((rc = parse_method(in, &req->method)) < 0) return rc;
    if ((rc = parse_target(in, &req->target)) < 0) return rc;
    if ((rc = parse_version(in, &req->minor)) < 0) return rc;

    rc = parse_header_list(in, req->headers, MAX_HEADERS);
    if (rc < 0) return rc;
    req->n_headers = rc;
    return PARSE_OK;
}

int HTTP_Parser::parse_response(std::string_view in, Response* res){
    int rc;
    if ((rc = parse_status(in, &res->minor, &res->status)) < 0) return rc;

    rc = parse_header_list(in, res->headers, MAX_HEADERS);
    if (rc < 0) return rc;
    res->n_headers = rc;
    res->body = in;
    return PARSE_OK;
}