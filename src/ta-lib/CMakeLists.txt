cmake_minimum_required(VERSION 2.8)

PROJECT(ta-lib)

SET(HEADER_INCLUDE_DIR ${ta-lib_SOURCE_DIR}/include)

if(UNIX)

message(STATUS "ON Unix")

SET(CMAKE_C_COMPILER "clang")

SET(CMAKE_CXX_COMPILER "clang++")

else()

message(STATUS "ON Windows")

SET(CMAKE_C_COMPILER "cl")

SET(CMAKE_CXX_COMPILER "cl")

endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "cl")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -m64") # don't add "-stdlib=libc++" let clang find it by itself

endif()

ADD_SUBDIRECTORY(src)