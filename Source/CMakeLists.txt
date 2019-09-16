#cmake_minimum_required(VERSION 3.8)
cmake_minimum_required(VERSION 3.7)
project(conman)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c processor.h utils.h utils.c common.h common.c processor.c loadsave.h loadsave.c)
add_executable(conman ${SOURCE_FILES})