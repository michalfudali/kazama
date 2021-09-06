cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project ("kazama")
add_executable (server server.cpp message.cpp writer.cpp)

find_package(Threads REQUIRED)
target_link_libraries(server Threads::Threads)

find_path(Asio_INCLUDE_DIR asio.hpp REQUIRED)
target_include_directories(server PRIVATE SYSTEM ${Asio_INCLUDE_DIR})