project(codec)

cmake_minimum_required(VERSION 2.8)

add_executable(${PROJECT_NAME} main.c fwht.c nv12.c rlc.c encoder.c quant.c)

if(CMAKE_COMPILER_IS_GNUCXX)
	    set(CMAKE_CXX_FLAGS "-O3")        
endif()

find_package (SDL REQUIRED)
include_directories (${SDL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL_LIBRARY})

