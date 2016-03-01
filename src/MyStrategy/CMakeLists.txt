message(STATUS "MyStrategy cmake configuring")

SET(LIBRARY_OUTPUT_PATH ${AUTOTRADE_SOURCE_DIR}/bin)

SET(SRC_LIST AMACrossStratgy.cpp 
			 EMACrossStratgy.cpp 
			 MACrossBOLLStrategy.cpp
			 MACrossBOLLTech.cpp
			 MACrossStratgy.cpp
			 MATech.cpp
			 Pos20Precent.cpp
			 TechUtils.cpp
			 WMACrossStratgy.cpp
			 mystrategy.cpp)

INCLUDE_DIRECTORIES(${HEADER_INCLUDE_DIR} ${AUTOTRADE_SOURCE_DIR}/src/Common ${AUTOTRADE_SOURCE_DIR}/src/StrategyFramework ${BOOST_DIR})

ADD_DEPENDENCIES(StrategyFramework Common)

ADD_LIBRARY(MyStrategy SHARED ${SRC_LIST})

SET_TARGET_PROPERTIES(MyStrategy PROPERTIES SUFFIX ".stg")