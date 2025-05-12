cmake_minimum_required(VERSION 3.14)
project(DistributedFileSystem)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

# === SQLite3 (system)
# find_package(SQLite3 REQUIRED)
add_library(sqlite3 STATIC
    include/sqlite/sqlite3.c
)

target_include_directories(sqlite3 PUBLIC include/sqlite)
target_compile_definitions(sqlite3 PUBLIC SQLITE_THREADSAFE=1)

# === Crow (as submodule)
add_subdirectory(include/Crow)

# === ASIO (header-only, as submodule)
include_directories(${PROJECT_SOURCE_DIR}/include/asio/asio/include)
add_definitions(-DASIO_STANDALONE)

# === OpenSSL Platform-specific Handling ===
if (WIN32)
    message(STATUS "Using prebuilt OpenSSL (Windows)")

    set(OPENSSL_ROOT_DIR "${PROJECT_SOURCE_DIR}/third_party/openssl")
    set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
    set(OPENSSL_LIB_DIR "${OPENSSL_ROOT_DIR}/lib/MD")

    include_directories(${OPENSSL_INCLUDE_DIR})
    link_directories(${OPENSSL_LIB_DIR})

    add_library(OpenSSL::SSL UNKNOWN IMPORTED)
    set_target_properties(OpenSSL::SSL PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_LIB_DIR}/libssl.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
    )
else()
    message(STATUS "Using system OpenSSL (Linux/macOS)")
    find_package(OpenSSL REQUIRED)
endif()

# === Common library ===
add_library(common
    src/common/Socket.cpp
    src/common/FileUtils.cpp
)

target_include_directories(common PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(common PUBLIC OpenSSL::SSL Crow)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(common ws2_32)
endif()

# === Master executable ===
add_executable(master
    src/master/main.cpp
    src/master/MasterNode.cpp
    src/master/WebServer.cpp
    src/master/Database.cpp
)

target_include_directories(master PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(master PRIVATE SQLite::SQLite3 common OpenSSL::SSL)

# === Client executable ===
add_executable(client
    src/client/main.cpp
    src/client/ClientNode.cpp
)

target_include_directories(client PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(client PRIVATE common OpenSSL::SSL)

# === Copy DLLs for runtime (Windows only) ===
if (WIN32)
    foreach(target IN ITEMS master client)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OPENSSL_LIB_DIR}/libssl.dll"
                $<TARGET_FILE_DIR:${target}>
        )
    endforeach()
endif()
