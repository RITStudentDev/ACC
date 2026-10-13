#pragma once

#include <thread>
#include "listener.h"
#include "parser.h"

/**
 * This header contains the declarations for the hosting service.
 * Definitions for the content of this file are included in service.cpp.
 * This file is used in the ACC daemon to persistantly run the web hosting service.
 */
class HTTP_Server{
    public:
        HTTP_Server() = default;

        int start_server();
        int run_loop();
    private:
        bool running_ = false;
        HTTP_Listener listener_;
        HTTP_Parser parser_;

        int onRecv();
};
