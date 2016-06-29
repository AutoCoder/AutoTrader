message(STATUS "UpdateInstruments cmake configuring")

SET(EXECUTABLE_OUTPUT_PATH ${AUTOTRADE_SOURCE_DIR}/bin)

SET(SRC_LIST    UpdateInstruments.cpp 
				tradespi.cpp)

INCLUDE_DIRECTORIES(${HEADER_INCLUDE_DIR} 
					${AUTOTRADE_SOURCE_DIR}/src/lib_json/include
					${AUTOTRADE_SOURCE_DIR}/src/Common
					${AUTOTRADE_SOURCE_DIR}/src/StrategyFramework )

LINK_DIRECTORIES(${LINK_LIB_DIR} ${EXECUTABLE_OUTPUT_PATH})

ADD_EXECUTABLE(update_instru ${SRC_LIST})

TARGET_LINK_LIBRARIES(update_instru dl Common lib_json libmysql thosttraderapi StrategyFramework)