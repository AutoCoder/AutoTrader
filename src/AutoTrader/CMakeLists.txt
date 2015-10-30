message(STATUS "AutoTrader cmake running")


SET(EXECUTABLE_OUTPUT_PATH ${AUTOTRADE_SOURCE_DIR}/bin)

SET(PLATFORM x64)

SET(SRC_LIST	AutoTrader.cpp 
			 	AP_Mgr.cpp 
				BOLLTech.cpp
				CommonUtils.cpp
				config.cpp
				ctpworker.cpp
				DBWrapper.cpp
				InstrumentInfoMgr.cpp
				KData.cpp
				MATech.cpp
				mdspi.cpp
				mysqlwrapper.cpp
				Order.cpp
				PositionControl.cpp
				PositionMgr.cpp
				RealTimeDataProcessor.cpp
				RealTimeDataProcessorPool.cpp
				stdafx.cpp
				TechUtils.cpp
				TickWrapper.cpp
				traderspi.cpp
				tradespi.cpp
				MACrossBOLLStrategy.cpp
				MACrossBOLLTech.cpp
				MACrossStratgy.cpp
			 	AMACrossStratgy.cpp
			 	EMACrossStratgy.cpp
			 	WMACrossStratgy.cpp )

INCLUDE_DIRECTORIES(${AUTOTRADE_SOURCE_DIR}/include ${AUTOTRADE_SOURCE_DIR}/src/lib_json/include)

LINK_DIRECTORIES(${AUTOTRADE_SOURCE_DIR}/lib ${EXECUTABLE_OUTPUT_PATH})

ADD_EXECUTABLE(AutoTrader ${SRC_LIST})

ADD_DEPENDENCIES(AutoTrader lib_json)

TARGET_LINK_LIBRARIES(AutoTrader lib_json libmysql thostmduserapi thosttraderapi)