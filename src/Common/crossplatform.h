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
#define SYNC_PRINT spdlog::get("console")->info()
#define SYNC_LOG spdlog::get("file_logger")->info()
#define SYNC_DEBUG_LOG spdlog::get("file_debug_logger")->info()
#endif

#endif