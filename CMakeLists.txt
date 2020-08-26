cmake_minimum_required (VERSION 3.14.5)

project (crochet)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
  set (CMAKE_C_FLAGS "-Weverything -Werror -Wpedantic -O2 -g")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set (CMAKE_C_FLAGS "-Wall -Wextra -Werror -Wpedantic -O2 -g")
  message (WARNING "gcc build will output a logs differently than clang")
  message (WARNING "the output logs will look terrible, consider building")
  message (WARNING "using clang")
else()
  message (FATAL_ERROR "supported compilers are GCC and CLANG only")
endif()

find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src/httpws)
add_subdirectory(src/hashmap)
add_subdirectory(src/ffjson)
add_subdirectory(src/exchanges)
add_subdirectory(src/finmath)
add_subdirectory(src/orderbooks)

add_library (pprint src/pprint.c)
add_library (globals src/globals.c)

add_executable (crochet src/main.c)
target_link_libraries (crochet globals pprint httpws ffjson hashmap exchanges
  orderbooks finmath OpenSSL::SSL OpenSSL::Crypto)

add_executable (crochet-test src/main.c)
target_compile_definitions(crochet-test PUBLIC DO_UNIT_TESTS)
target_link_libraries (crochet-test globals pprint httpws ffjson hashmap exchanges
  orderbooks finmath OpenSSL::SSL OpenSSL::Crypto)

set_target_properties(crochet PROPERTIES RULE_MESSAGES OFF)
set_target_properties(crochet-test PROPERTIES RULE_MESSAGES OFF)


