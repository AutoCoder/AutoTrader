message(STATUS "MyStrategy2 cmake configuring")

SET(LIBRARY_OUTPUT_PATH ${AUTOTRADE_SOURCE_DIR}/bin)

SET(SRC_LIST BigProfitSmallLoseStrategy.cpp 
						 TestCancelOrderStrategy.cpp
			 Pos1Shou.cpp 
			 mystrategy.cpp)

INCLUDE_DIRECTORIES(${HEADER_INCLUDE_DIR} ${AUTOTRADE_SOURCE_DIR}/src/Common ${AUTOTRADE_SOURCE_DIR}/src/StrategyFramework ${BOOST_DIR})

ADD_DEPENDENCIES(StrategyFramework Common)

ADD_LIBRARY(MyStrategy2 SHARED ${SRC_LIST})

SET_TARGET_PROPERTIES(MyStrategy2 PROPERTIES SUFFIX ".stg")