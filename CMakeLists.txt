cmake_minimum_required(VERSION 3.14)
project(DistributedFileSystem)

find_package(OpenSSL REQUIRED) 
find_package(SQLite3 REQUIRED)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

add_library(common
    src/common/Socket.cpp
)

target_include_directories(common PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(common PUBLIC OpenSSL::SSL OpenSSL::Crypto) 

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(common ws2_32)
endif()

add_subdirectory(include/Crow)
target_link_libraries(common PUBLIC Crow)

add_executable(master
    src/master/main.cpp
    src/master/MasterNode.cpp
    src/master/WebServer.cpp
    src/master/Database.cpp
)

target_include_directories(master PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(master PRIVATE SQLite::SQLite3 common OpenSSL::SSL OpenSSL::Crypto) 

add_executable(client
    src/client/main.cpp
    src/client/ClientNode.cpp
)

target_include_directories(client PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(client common OpenSSL::SSL OpenSSL::Crypto) 
