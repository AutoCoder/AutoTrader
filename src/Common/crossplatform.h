#ifndef CROSS_PLATFORM_H
#define CROSS_PLATFORM_H

#ifdef WIN32

#include <tchar.h>
//#define   _WINSOCKAPI_//define this marco to prohibit including winsock.h in windows.h  -  please refer http://blog.csdn.net/mazhen1986/article/details/7681238
//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN  //please refer http://www.cppblog.com/ming81/archive/2013/02/17/197875.html
//#endif
//#define _WINSOCK2API_
#include "windows.h"
#define sleep(x) Sleep(x)
#define STRCPY strcpy_s
#define SPRINTF sprintf_s

#else

#include <unistd.h>
#define sleep(x) usleep(x*1000)
#define STRCPY strcpy
#define SPRINTF sprintf

#endif

#ifndef SYNC_PRINT
#include "spdlog/spdlog.h"
#define CONSOLE_LOGGER spdlog::get("console")
#define COMMON_LOGGER spdlog::get("file_logger")
#define DEBUG_LOGGER spdlog::get("file_debug_logger")
#define TRADE_LOGGER spdlog::get("file_trade_logger")
#define SYNC_PRINT CONSOLE_LOGGER->info()
#define SYNC_LOG COMMON_LOGGER->info()
#define SYNC_DEBUG_LOG DEBUG_LOGGER->info()
#define SYNC_TRADE_LOG TRADE_LOGGER->info()
#endif

#endif