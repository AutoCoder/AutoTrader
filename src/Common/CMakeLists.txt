message(STATUS "Common cmake configuring")

SET(LIBRARY_OUTPUT_PATH ${AUTOTRADE_SOURCE_DIR}/bin)

SET(SRC_LIST AccountMgr.cpp
			 CommonUtils.cpp 
			 Config.cpp
			 DBWrapper.cpp
			 mysqlwrapper.cpp)

INCLUDE_DIRECTORIES(${HEADER_INCLUDE_DIR} ${AUTOTRADE_SOURCE_DIR}/src/lib_json/include)

LINK_DIRECTORIES(${LINK_LIB_DIR} ${EXECUTABLE_OUTPUT_PATH})

ADD_LIBRARY(Common SHARED ${SRC_LIST})
