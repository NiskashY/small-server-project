cmake_minimum_required(VERSION 3.22)
project(Chat)

set(CMAKE_CXX_STANDARD 20)

find_path(UWEBSOCKETS_INCLUDE_DIRS "uwebsockets/App.h")

add_executable(Chat server.cpp)

target_include_directories(Chat PRIVATE ${UWEBSOCKETS_INCLUDE_DIRS})
target_link_libraries(Chat /home/mint/CLionProjects/Chat/vcpkg/installed/x64-linux/lib/libuSockets.a z)

