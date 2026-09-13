cmake_minimum_required(VERSION 3.10)
project(gitManager CXX)

add_compile_options(
	-fdiagnostics-color=always 
    -Wextra 
    -Wall 
    -Werror
    -fno-rtti 
    -fno-exceptions 
    -fno-stack-protector 
    -fno-unwind-tables 
    -fno-asynchronous-unwind-tables
)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/includes"
)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

add_subdirectory(src)

add_executable(gitManager src/main.cpp)

target_link_libraries(gitManager PRIVATE
    changeGit
    envGit
    ZLIB::ZLIB
    OpenSSL::Crypto
)