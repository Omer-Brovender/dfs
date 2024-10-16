cmake_minimum_required(VERSION 3.10)
project(DistributedFileSystem)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(common
    src/common/Socket.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(common ws2_32)
endif()

add_executable(master
    src/master/main.cpp
    src/master/MasterNode.cpp
)

target_link_libraries(master common)

add_executable(client
    src/client/main.cpp
    src/client/ClientNode.cpp
)

target_link_libraries(client common)
