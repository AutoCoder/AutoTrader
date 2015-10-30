cmake_minimum_required(VERSION 2.6)

PROJECT(AUTOTRADE)

ADD_SUBDIRECTORY(src)

SET(CMAKE_BUILD_TYPE "Debug")


if(USE_X64)

message(STATUS "Using 32bits")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-m32")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-m32")

else()

message(STATUS "Using 64bits")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-m64")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-m64")	

endif(USE_X64)



message(STATUS "->overview cmake running")

MESSAGE(STATUS "->This is BINARY dir " ${AUTOTRADE_BINARY_DIR})

MESSAGE(STATUS "->This is SOURCE dir " ${AUTOTRADE_SOURCE_DIR})

MESSAGE(STATUS "->Current platform is " ${CMAKE_CXX_FLAGS})