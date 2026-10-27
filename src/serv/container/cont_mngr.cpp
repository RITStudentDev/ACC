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

ContainerManager::ContainerManager(ContainerClient client) :
    container_(std::move(client))
    {}

bool ContainerManager::valid_name(std::string_view s){
    if (s.empty() || s.size() > 63) return false;
    for (char c : s) if (!(c >= 'a' && c <= 'z') && !(c >= '0' && c <= '9') && c != '-' && c != '_') return false;
    return true;
}

bool ContainerManager::valid_hostname(std::string_view s){
    if (s.empty() || s.size() > 253) return false;
    for (char c : s) if (!in_set(c, ".-")) return false;
    return true;
}

bool ContainerManager::valid_image(std::string_view s){
    if (s.empty() || s.size() > 255) return false;
    for (char c : s) if (!in_set(c, "._:/@-")) return false;
    return true;
}

std::string ContainerManager::container_name(std::string name){
    return "acc-" + name;
}

std::string ContainerManager::create_body(const App& app){
    const std::string port = std::to_string(app.port) + "/tcp";
    return "{\"Image\":\"" + app.image + "\","
           "\"Labels\":{\"acc.managed\":\"true\"},"
           "\"ExposedPorts\":{\"" + port + "\":{}},"
           "\"HostConfig\":{"
               "\"PortBindings\":{\"" + port + "\":[{\"HostIp\":\"127.0.0.1\",\"HostPort\":\"\"}]},"
               "\"RestartPolicy\":{\"Name\":\"unless-stopped\"}}}";
}

int ContainerManager::add_app(const App& app){
    if (!valid_name(app.name) || !valid_image(app.image) ||
        !valid_hostname(app.hostname) || app.port == 0) return CONTAINER_ERR_INVALID;
    if (apps_.count(app.name) || host_to_name_.count(app.hostname)) return CONTAINER_ERR_EXISTS;

    host_to_name_.emplace(app.hostname, app.name);
    apps_.emplace(app.name, app);
    return CONTAINER_OK;
}

const App* ContainerManager::find_by_name(std::string_view name) const {
    auto it = apps_.find(std::string(name));
    return it == apps_.end() ? nullptr : &it->second;
}

const App* ContainerManager::find_by_host(std::string_view host) const {
    auto it = host_to_name_.find(std::string(host));
    return it == host_to_name_.end() ? nullptr : find_by_name(it->second);
}