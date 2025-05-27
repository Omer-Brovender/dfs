cmake_minimum_required(VERSION 3.14)
project(DistributedFileSystem)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

include_directories(third_party/)

# === SQLCipher ===
#add_library(sqlcipher STATIC
#    third_party/sqlcipher/sqlite3.c
#)
#set_source_files_properties(third_party/sqlcipher/sqlite3.c PROPERTIES LANGUAGE C)
#target_include_directories(sqlcipher PUBLIC third_party/sqlcipher)
#target_compile_definitions(sqlcipher PUBLIC
#    SQLITE_HAS_CODEC=1
#    SQLITE_TEMP_STORE=2
#    SQLITE_THREADSAFE=1
#    SQLITE_OS_WIN=1
#    SQLITE_ENABLE_X86_PCLMUL=1
#    SQLITE_ENABLE_FTS5=1
#    SQLITE_ENABLE_RTREE=1
#    _XOPEN_SOURCE=700
#    SQLITE_EXTRA_INIT=sqlcipher_extra_init
#    SQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown
#)

# === ASIO ===
set(ASIO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/asio/asio/include")
include_directories(${ASIO_INCLUDE_DIR})
add_definitions(-DASIO_STANDALONE)

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

# === SQLite3 ===
add_library(sqlite3 STATIC
    third_party/sqlite/sqlite3.c
)

target_include_directories(sqlite3 PUBLIC third_party/sqlite)
target_compile_definitions(sqlite3 PUBLIC SQLITE_THREADSAFE=1)

# === Crow ===
add_subdirectory(include/Crow)
add_definitions(-DCROW_ENABLE_SSL)

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
    src/master/Encryption.cpp
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

        # Copy "web", "static" and "ssl" folders
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${PROJECT_SOURCE_DIR}/web"
                "$<TARGET_FILE_DIR:${target}>/web"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${PROJECT_SOURCE_DIR}/static"
                "$<TARGET_FILE_DIR:${target}>/static"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${PROJECT_SOURCE_DIR}/ssl"
                "$<TARGET_FILE_DIR:${target}>/ssl"
        )
    endforeach()
endif()
