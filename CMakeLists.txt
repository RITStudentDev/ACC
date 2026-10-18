cmake_minimum_required(VERSION 3.16)
project(ACC)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(_core src/cli/binds.cpp)
set_target_properties(_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src/cli)
install(TARGETS _core LIBRARY DESTINATION cli)

add_executable(ACC
    src/serv/components/listener.cpp
    src/serv/components/parser.cpp
    src/serv/service.cpp
    src/accd.cpp
)

target_include_directories(ACC
    PUBLIC  ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/serv/components
            /usr/local/include
)