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
 * This file contains the declarations for the control channel.
 * Its purpose is to let the CLI manage hosted apps. The daemon listens on a
 * unix socket; the CLI sends one command per connection as a single text line
 * and reads the reply until the daemon closes the connection.
 *
 * Request:  <command> [args...]\n
 * Reply:    "OK\n" followed by zero or more payload lines, or
 *           "ERR <message>\n"
 *
 * Commands:
 *   add <name> <image> <hostname> <port>   register an app
 *   up <name>                              create (if needed) and start its container
 *   down <name>                            stop its container
 *   rm <name>                              remove its container and unregister it
 *   status <name>                          show registration and container state
 *   list                                   show every registered app
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
        std::string handle_line(std::string_view line);
        static std::vector<std::string_view> split(std::string_view line);
        void drop(int fd);

        std::string cmd_add(const std::vector<std::string_view>& t);
        std::string cmd_rm(const std::vector<std::string_view>& t);
        std::string cmd_up(const std::vector<std::string_view>& t);
        std::string cmd_down(const std::vector<std::string_view>& t);
        std::string cmd_status(const std::vector<std::string_view>& t);
        std::string cmd_list(const std::vector<std::string_view>& t);
        
};