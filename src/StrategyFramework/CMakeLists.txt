message(STATUS "StrategyFramework cmake configuring")

SET(LIBRARY_OUTPUT_PATH ${AUTOTRADE_SOURCE_DIR}/bin)

SET(SRC_LIST AP_Mgr.cpp 
			 PPMgr.cpp 
		     BOLLTech.cpp 
		     KData.cpp
		     Order.cpp
		     TickWrapper.cpp
		     TriggerFactory.cpp
		     InstrumentInfoMgr.cpp)

INCLUDE_DIRECTORIES(${HEADER_INCLUDE_DIR}
					${AUTOTRADE_SOURCE_DIR}/src/Common
					${AUTOTRADE_SOURCE_DIR}/src/lib_json/include
					${BOOST_DIR})

LINK_DIRECTORIES(${EXECUTABLE_OUTPUT_PATH})

ADD_LIBRARY(StrategyFramework SHARED ${SRC_LIST})
