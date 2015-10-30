message(STATUS "lib_json cmake running")

SET(LIBRARY_OUTPUT_PATH ${AUTOTRADE_SOURCE_DIR}/bin)

SET(PLATFORM x64)

SET(CMAKE_C_COMPILER g++)

SET(SRC_LIST json_reader.cpp json_value.cpp json_writer.cpp)

INCLUDE_DIRECTORIES($(CMAKE_CURRENT_SOURCE_DIR)/include)

ADD_LIBRARY(lib_json STATIC ${SRC_LIST})