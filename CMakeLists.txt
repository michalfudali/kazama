cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project ("kazama")
add_executable (server "server.cpp")

find_package(Threads REQUIRED)
target_link_libraries(server Threads::Threads)

find_path(Asio_INCLUDE_DIR "asio.hpp" REQUIRED)
include_directories(${Asio_INCLUDE_DIR})