cmake_minimum_required(VERSION 3.17.3)

project(elite)

set(CMAKE_C_EXTENSIONS ON)
 
include_directories(include)
 
file(GLOB SOURCES "src/*.c")
file(GLOB HELPERS "src/helpers/*.c")
file(GLOB UTILS "src/utils/*.c")

add_executable(elite ${SOURCES} ${HELPERS} ${UTILS})