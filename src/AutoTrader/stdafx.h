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
#include <string.h>
#include <stddef.h>

#ifdef WIN32

#include <tchar.h>
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


#define instrument_1 "rb1601"
#define instrument_2 "rb1602"

#define SYNC_PRINT spdlog::get("console")->info()

#endif