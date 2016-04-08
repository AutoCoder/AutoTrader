message(STATUS "AutoTrader cmake configuring")
message(STATUS "BoostDIR : ${BOOST_DIR}/stage/lib")

SET(EXECUTABLE_OUTPUT_PATH ${AUTOTRADE_SOURCE_DIR}/bin)

SET(SRC_LIST    ActionProcessor.cpp 
			 	AutoTrader.cpp 
			 	ClientSession.cpp
			 	BaseClientSession.cpp
			 	LocalClientSession.cpp
			 	LocalClientSession.cpp
			 	ClientSessionMgr.cpp
				InstrumentInfoMgr.cpp
				RealTimeDataProcessor.cpp
				RealTimeDataProcessorPool.cpp
				Utils.cpp
				fifo_action_queue.cpp
				mdspi.cpp
				tradespi.cpp
				remote_server_action.cpp
				remote_user_action.cpp
				socket_server.cpp
				socket_server_impl.cpp
				socket_session.cpp
				stdafx.cpp)

INCLUDE_DIRECTORIES(${HEADER_INCLUDE_DIR} 
					${AUTOTRADE_SOURCE_DIR}/src/lib_json/include 
					${BOOST_DIR} 
					${AUTOTRADE_SOURCE_DIR}/src/Common 
					${AUTOTRADE_SOURCE_DIR}/src/StrategyFramework)

LINK_DIRECTORIES(${LINK_LIB_DIR} ${EXECUTABLE_OUTPUT_PATH} ${BOOST_DIR}/stage/lib)

ADD_EXECUTABLE(AutoTrader ${SRC_LIST})

ADD_DEPENDENCIES(AutoTrader lib_json Common StrategyFramework)

TARGET_LINK_LIBRARIES(AutoTrader dl lib_json libmysql thostmduserapi thosttraderapi Common StrategyFramework boost_filesystem boost_system)
