// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifdef STDAFX_H_
#define STDAFX_H_

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <assert.h>


// TODO: reference additional headers your program requires here
#endif


#ifdef WIN32
#include "windows.h"
#define sleep(x) Sleep(x)
#else
#include <unistd.h>
#define sleep(x) usleep(x*1000)
#endif