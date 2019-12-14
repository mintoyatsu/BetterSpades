cmake_minimum_required(VERSION 2.8)
project(FourSpades C)

set(FOURSPADES_MAJOR 0)
set(FOURSPADES_MINOR 1)
set(FOURSPADES_PATCH 0)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		add_compile_options(-O3)
	else()
		add_compile_options(-O3 -g -march=native)
	endif()
endif()

add_subdirectory(src)
