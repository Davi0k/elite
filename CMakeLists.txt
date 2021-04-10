cmake_minimum_required(VERSION 3.13.4)

project(elite)
 
include_directories(include)
 
file(GLOB_RECURSE SOURCES "src/*.c") 
file(GLOB_RECURSE HELPERS "src/helpers/*.c")
file(GLOB_RECURSE UTILITIES "src/utilities/*.c")
file(GLOB_RECURSE NATIVES "src/natives/*.c")
file(GLOB_RECURSE TYPES "src/types/*.c")

add_definitions("-g")

add_executable(elite ${SOURCES} ${HELPERS} ${UTILITIES} ${NATIVES} ${TYPES})

target_link_libraries(elite gmp)