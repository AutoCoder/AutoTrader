message(STATUS "AutoTrader cmake configuring")

SET(EXECUTABLE_OUTPUT_PATH ${AUTOTRADE_SOURCE_DIR}/bin)

SET(SRC_LIST	AutoTrader.cpp 
			 	AP_Mgr.cpp 
				BOLLTech.cpp
				CommonUtils.cpp
				config.cpp
				DBWrapper.cpp
				InstrumentInfoMgr.cpp
				KData.cpp
				MATech.cpp
				mdspi.cpp
				mysqlwrapper.cpp
				Order.cpp
				PositionControl.cpp
				RealTimeDataProcessor.cpp
				RealTimeDataProcessorPool.cpp
				stdafx.cpp
				TechUtils.cpp
				TickWrapper.cpp
				tradespi.cpp
				MACrossBOLLStrategy.cpp
				MACrossBOLLTech.cpp
				MACrossStratgy.cpp
			 	AMACrossStratgy.cpp
			 	EMACrossStratgy.cpp
			 	WMACrossStratgy.cpp )

INCLUDE_DIRECTORIES(${HEADER_INCLUDE_DIR} ${AUTOTRADE_SOURCE_DIR}/src/lib_json/include)

LINK_DIRECTORIES(${LINK_LIB_DIR} ${EXECUTABLE_OUTPUT_PATH})

ADD_EXECUTABLE(AutoTrader ${SRC_LIST})

ADD_DEPENDENCIES(AutoTrader lib_json)

TARGET_LINK_LIBRARIES(AutoTrader lib_json libmysql thostmduserapi thosttraderapi)
