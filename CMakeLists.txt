cmake_minimum_required(VERSION 3.10)
project(DistributedFileSystem)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(common
    src/common/Socket.cpp
)

add_executable(master
    src/master/main.cpp
)

target_link_libraries(master common)

add_executable(client
    src/client/main.cpp
)

target_link_libraries(client common)