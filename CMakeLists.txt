cmake_minimum_required(VERSION 3.14)
project(DistributedFileSystem)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

include_directories(third_party/)

# === SQLite3 (from amalgamation source) ===
add_library(sqlite3 STATIC
    third_party/sqlite/sqlite3.c
)
target_include_directories(sqlite3 PUBLIC third_party/sqlite)
target_compile_definitions(sqlite3 PUBLIC SQLITE_THREADSAFE=1)

# === ASIO (header-only, as submodule) ===
set(ASIO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/asio/asio/include")
include_directories(${ASIO_INCLUDE_DIR})
add_definitions(-DASIO_STANDALONE)

# === Crow (as submodule) ===
add_subdirectory(include/Crow)

# === OpenSSL ===
if (WIN32)
    message(STATUS "Using prebuilt OpenSSL (Windows)")

    set(OPENSSL_ROOT_DIR "${PROJECT_SOURCE_DIR}/third_party/openssl")
    set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}")
    set(OPENSSL_LIB_DIR "${OPENSSL_ROOT_DIR}/lib")
    set(OPENSSL_BIN_DIR "${OPENSSL_ROOT_DIR}/bin")

    include_directories(${OPENSSL_INCLUDE_DIR})
    link_directories(${OPENSSL_LIB_DIR})

    # SSL library
    add_library(OpenSSL::SSL UNKNOWN IMPORTED)
    set_target_properties(OpenSSL::SSL PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_LIB_DIR}/libssl.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
    )

    # Crypto library (needed for SHA256_*)
    add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
    set_target_properties(OpenSSL::Crypto PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_LIB_DIR}/libcrypto.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
    )
else()
    message(STATUS "Using system OpenSSL (Linux/macOS)")
    find_package(OpenSSL REQUIRED)
endif()

# === Common Library ===
add_library(common
    src/common/Socket.cpp
    src/common/FileUtils.cpp
)
target_include_directories(common PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(common PUBLIC OpenSSL::SSL OpenSSL::Crypto Crow)

if (WIN32)
    target_link_libraries(common PUBLIC ws2_32)
endif()

# === Master Executable ===
add_executable(master
    src/master/main.cpp
    src/master/MasterNode.cpp
    src/master/WebServer.cpp
    src/master/Database.cpp
)
target_include_directories(master PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(master PRIVATE sqlite3 common OpenSSL::SSL OpenSSL::Crypto)

# === Client Executable ===
add_executable(client
    src/client/main.cpp
    src/client/ClientNode.cpp
)
target_include_directories(client PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(client PRIVATE common OpenSSL::SSL OpenSSL::Crypto)

# === Copy OpenSSL DLLs on Windows ===
if (WIN32)
    foreach(target IN ITEMS master client)
        # Copy DLLs
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OPENSSL_BIN_DIR}/libssl-3-x64.dll"
                $<TARGET_FILE_DIR:${target}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OPENSSL_BIN_DIR}/libcrypto-3-x64.dll"
                $<TARGET_FILE_DIR:${target}>
        )

        # Copy "web" and "static" folders
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${PROJECT_SOURCE_DIR}/web"
                "$<TARGET_FILE_DIR:${target}>/web"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${PROJECT_SOURCE_DIR}/static"
                "$<TARGET_FILE_DIR:${target}>/static"
        )
    endforeach()
endif()
