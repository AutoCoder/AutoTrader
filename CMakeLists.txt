cmake_minimum_required(VERSION 2.8)

PROJECT(AUTOTRADE)

SET(BOOST_DIR ${AUTOTRADE_SOURCE_DIR}/boost_1_60_0)

if(UNIX)

message(STATUS "ON Unix")

SET(LINK_LIB_DIR ${AUTOTRADE_SOURCE_DIR}/linux_lib)

SET(HEADER_INCLUDE_DIR ${AUTOTRADE_SOURCE_DIR}/linux_include)

SET(CMAKE_C_COMPILER "clang")

SET(CMAKE_CXX_COMPILER "clang++")

else()

message(STATUS "ON Windows")

set(LINK_LIB_DIR ${AUTOTRADE_SOURCE_DIR}/lib)

SET(HEADER_INCLUDE_DIR ${AUTOTRADE_SOURCE_DIR}/include)

endif()

message(STATUS "CMAKE_C_COMPILER is " ${CMAKE_C_COMPILER} " AND " "CMAKE_CXX_COMPILER is " ${CMAKE_CXX_COMPILER})

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -std=c++11 -pthread -m64") # don't add "-stdlib=libc++" let clang find it by itself

message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")

message(STATUS "CMAKE_CXX_FLAGS don't add \"-stdlib=libc++\" let clang find it by itself")

endif()

ADD_SUBDIRECTORY(src)