cmake_minimum_required(VERSION 3.28)

project(
    AgoraIM
    VERSION 0.1.0
    DESCRIPTION "A high-performance instant messaging system built with modern C++20."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_library(agora_base
    src/base/noncopyable.cpp
    src/base/timestamp.cpp
    src/base/current_thread.cpp
    src/base/log_level.cpp
    src/base/log_message.cpp
    src/base/logger.cpp
)

add_library(agora_net
    src/net/inet_address.cpp
    src/net/socket.cpp
    src/net/sockets_ops.cpp
    src/net/channel.cpp
    src/net/poller.cpp
    src/net/epoll_poller.cpp
    src/net/event_loop.cpp
    src/net/acceptor.cpp
    src/net/buffer.cpp
    src/net/tcp_connection.cpp
    src/net/tcp_server.cpp
    src/net/event_loop_thread.cpp
    src/net/event_loop_thread_pool.cpp
    src/net/connector.cpp
    src/net/tcp_client.cpp
)

target_link_libraries(agora_net PUBLIC agora_base)

add_executable(agora_server
    src/main.cpp
)

target_link_libraries(agora_server
    PRIVATE
    agora_net
    agora_base
)

# 新增：编译 examples 目录
add_subdirectory(examples)