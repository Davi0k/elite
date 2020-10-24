cmake_minimum_required(VERSION 3.17.3)

project(elite)
 
include_directories(include)
 
file(GLOB SOURCES "src/*.cpp")
 
add_executable(elite ${SOURCES})