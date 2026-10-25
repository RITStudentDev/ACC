#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <cont_client.h>

enum ContainerStatus{
    CONTAINER_OK = 0,
    CONTAINER_ERR_INVALID = -1,
    CONTAINER_ERR_EXISTS = -2,
    CONTAINER_ERR_NOT_FOUND = -3,
    CONTAINER_ERR_DOCKER = -4,
    CONTAINER_ERR_IO = -5
};

struct App{
    std::string name;
    std::string hostname;
    std::string image;
    uint16_t container_port = 0;
};

struct AppState{
    std::string state;
    int host_port = -1;
};

class ContainerManager{
    public:
        explicit ContainerManager(ContainerClient client = ContainerClient{});

        int add_app(const App& app);
        const App* find_by_name(std::string_view name) const;
        const App* find_by_host(std::string_view host) const;
        std::vector<std::string> list_apps() const;

        int start_app(std::string_view name);
        int stop_app(std::string_view name);
        int remove_app(std::string_view name);
        int app_status(std::string_view name, AppState* out) const;

    private:
        ContainerClient container_;
        std::unordered_map<std::string, App> apps_;
        std::unordered_map<std::string, std::string> host_to_name_;

        static bool valid_name(std::string_view s);
        static bool valid_hostname(std::string_view s);
        static std::string container_name(std::string name);
        static std::string create_body(const App& app);
        int create_container(const App& app) const;
        int send_lifecycle(std::string_view method, const std::string& path, int ok_a, int ok_b) const;
};