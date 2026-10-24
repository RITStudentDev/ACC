#include "cont_mngr.h"
#include <charconv>

namespace {

bool in_set(char c, std::string_view extra){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9') || extra.find(c) != std::string_view::npos;
}

std::string_view json_string_after(std::string_view body, std::string_view anchor, std::string_view key){
    const size_t a = body.find(anchor);
    if (a == std::string_view::npos) return {};

    const std::string pattern = "\"" + std::string(key) + "\":\"";
    const size_t k = body.find(pattern, a);
    if (k == std::string_view::npos) return {};

    const size_t start = k + pattern.size();
    const size_t end = body.find('"', start);
    if (end == std::string_view::npos) return {};
    return body.substr(start, end - start);
}

int map_docker_err(int rc){
    return rc == DOCKER_ERR_IO ? CONTAINER_ERR_IO : CONTAINER_ERR_DOCKER;
}

} // namespace