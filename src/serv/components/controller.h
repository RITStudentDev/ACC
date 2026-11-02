#pragma once
#include "../container/cont_mngr.h"
#include <string>
#include <vector>

enum ControlStatus {
    CONTROL_OK = 0,
    CONTROL_ERR_IO = -1,
    CONTROL_ERR_PATH = -2
};

/**
 * Controller is used in the ACC service to enable CLI usage for container
 * management. It acts as a producer server for the CLI to connect to the
 * docker daemon in order to execute ACC commands.
 */
class Controller{
    public:
        explicit Controller(ContainerManager& containers);
        ~Controller();
        Controller(const Controller&) = delete;
        Controller& operator=(const Controller&) = delete;

        int start(const std::string& path);
        void stop();
        int listen_fd() const { return listenfd_; }
        bool owns(int fd) const { return clients_.count(fd) != 0; }
        int accept_clients(int epollfd);
        int on_readable(int fd);

    private:
        static constexpr size_t MAX_LINE = 1024;
        static constexpr size_t MAX_CLIENTS = 32;
        static constexpr int SEND_TIMEOUT = 2;

        ContainerManager containers_;
        int listenfd_;
        std::string path_;
        std::unordered_map<int, std::string> clients_;

        static int send_all(int fd, std::string_view data);
        static std::string handle_line(std::string_view line);
        static std::vector<std::string_view> split(std::string_view line);
        void drop(int fd);

        std::string cmd_add(const std::vector<std::string_view>& t);
        std::string cmd_rm(const std::vector<std::string_view>& t);
        std::string cmd_up(const std::vector<std::string_view>& t);
        std::string cmd_down(const std::vector<std::string_view>& t);
        std::string cmd_status(const std::vector<std::string_view>& t);
        std::string cmd_list(const std::vector<std::string_view>& t);
        
};