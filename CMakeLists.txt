cmake_minimum_required(VERSION 2.8)

PROJECT(AUTOTRADE)

if(UNIX)

message(STATUS "ON Unix")

SET(LINK_LIB_DIR ${AUTOTRADE_SOURCE_DIR}/linux_lib)

SET(HEADER_INCLUDE_DIR ${AUTOTRADE_SOURCE_DIR}/linux_include)

SET(CMAKE_C_COMPILER "clang")

SET(CMAKE_CXX_COMPILER "clang++")

else()

message(STATUS "ON Windows")

set(LINK_LIB_DIR ${AUTOTRADE_SOURCE_DIR}/lib)

SET(HEADER_INCLUDE_DIR ${AUTOTRADE_SOURCE_DIR}/linux_include)

endif()

message(STATUS "CMAKE_C_COMPILER is " ${CMAKE_C_COMPILER} " AND " "CMAKE_CXX_COMPILER is " ${CMAKE_CXX_COMPILER})

message(STATUS "Link lib directory is " ${LINK_LIB_DIR})
message(STATUS "Header directory is " ${HEADER_INCLUDE_DIR})

#if(USE_X64)

#message(STATUS "Using win32")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-m32")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-m32")

#else()

#message(STATUS "Using x64")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-m64")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-m64")	

#endif(USE_X64)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

message(STATUS "using Clang compiler")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -std=c++11") # don't add "-stdlib=libc++" let clang find it by itself

endif()

ADD_SUBDIRECTORY(src)