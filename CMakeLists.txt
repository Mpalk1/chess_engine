cmake_minimum_required(VERSION 3.28)
project(chess_engine)

set(CMAKE_CXX_STANDARD 20)

option(TRACY_ENABLE "" ON)
add_subdirectory(third_party/tracy)

include_directories(include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(chess_engine ${SOURCES} third_party/tracy/public/tracy/Tracy.hpp)

target_include_directories(chess_engine PUBLIC third_party/tracy/public)
target_link_libraries(chess_engine PUBLIC TracyClient)
