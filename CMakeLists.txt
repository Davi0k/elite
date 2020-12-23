cmake_minimum_required(VERSION 3.13.4)

project(elite)
 
include_directories(include)
 
file(GLOB SOURCES "src/*.c") 
file(GLOB HELPERS "src/helpers/*.c")
file(GLOB UTILITIES "src/utilities/*.c")
file(GLOB TYPES "src/types/*.c")

add_definitions("-O3")

add_executable(elite ${SOURCES} ${HELPERS} ${UTILITIES} ${TYPES})

target_link_libraries(elite gmp)