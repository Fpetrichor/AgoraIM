# examples/CMakeLists.txt

# EchoServer
add_executable(echo_server echo/echo_server.cpp)
target_link_libraries(echo_server PRIVATE agora_net agora_base)

# EchoClient
add_executable(echo_client echo/echo_client.cpp)
target_link_libraries(echo_client PRIVATE agora_net agora_base)

add_executable(static_file_server http/static_file_server.cpp)
target_link_libraries(static_file_server PRIVATE agora_base agora_net)