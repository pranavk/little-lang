cmake_minimum_required(VERSION 2.6)
project(lilang)

set(CMAKE_CXX_STANDARD 14)

find_package(FLEX REQUIRED)
message(STATUS "Found Flex ${FLEX_VERSION}")
FLEX_TARGET(lilFlex ${CMAKE_SOURCE_DIR}/lexer.l ${CMAKE_SOURCE_DIR}/scanner.cpp)

FILE(GLOB SRCFILES *.cpp)
add_library(flex STATIC IMPORTED GLOBAL)

add_executable(lilang ${SRCFILES} ${FLEX_lilFlex_OUTPUTS})
target_link_libraries(lilang)
