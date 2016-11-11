project(codec)

cmake_minimum_required(VERSION 2.8)

add_executable(${PROJECT_NAME} main.c fwht.c frame.c rlc.c encoder.c quant.c decoder.c)

set(CMAKE_C_FLAGS "-O3")


find_package (SDL REQUIRED)
include_directories (${SDL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL_LIBRARY})
