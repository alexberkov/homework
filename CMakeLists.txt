cmake_minimum_required(VERSION 3.19)
project(server)

set(CMAKE_CXX_STANDARD 20)

add_executable(server main.cpp chatbot.h)

find_package(nlohmann_json 3.2.0 REQUIRED)
target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)

find_path(UWEBSOCKETS uwebsockets)
include_directories(${UWEBSOCKETS})
target_link_directories(server PRIVATE ${UWEBSOCKETS})

target_link_libraries(server PRIVATE /Users/alexberkov/vcpkg/installed/x64-osx/lib/libuSockets.a)
target_link_libraries(server PRIVATE /Users/alexberkov/vcpkg/installed/x64-osx/lib/liblibuv.a)
target_link_libraries(server PRIVATE /Users/alexberkov/vcpkg/installed/x64-osx/lib/libz.a)