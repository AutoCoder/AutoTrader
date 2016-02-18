// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef STDAFX_H_
#define STDAFX_H_

#include "targetver.h"

#include <stdio.h>
#include <assert.h>
#include <string>
#include <cstring>
#include <stddef.h>

#ifdef WIN32

//#define   _WINSOCKAPI_//define this marco to prohibit including winsock.h in windows.h  -  please refer http://blog.csdn.net/mazhen1986/article/details/7681238

#endif

//#ifdef WIN32
//
//#include <tchar.h>
////#define   _WINSOCKAPI_//define this marco to prohibit including winsock.h in windows.h  -  please refer http://blog.csdn.net/mazhen1986/article/details/7681238
////#ifndef WIN32_LEAN_AND_MEAN
////#define WIN32_LEAN_AND_MEAN  //please refer http://www.cppblog.com/ming81/archive/2013/02/17/197875.html
////#endif
//#define _WINSOCK2API_
////#include "windows.h"
//#define sleep(x) Sleep(x)
//#define STRCPY strcpy_s
//#define SPRINTF sprintf_s
//
//#else
//
//#include <unistd.h>
//#define sleep(x) usleep(x*1000)
//#define STRCPY strcpy
//#define SPRINTF sprintf
//
//#endif
//
//
//#define instrument_1 "rb1601"
//#define instrument_2 "rb1602"
//
//#define SYNC_PRINT spdlog::get("console")->info()

#endif