cmake_minimum_required (VERSION 3.14.5)

project (crochet)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_C_FLAGS "-Weverything -Werror -Wpedantic -O2 -g")

find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src/httpws)

add_library (pprint SHARED src/pprint.c)
add_executable (crochet src/main.c)

target_link_libraries (crochet pprint httpws OpenSSL::SSL OpenSSL::Crypto)
