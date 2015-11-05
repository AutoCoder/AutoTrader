
#ifdef WIN32
#include "windows.h"
#define sleep(x) Sleep(x)
#else
#include <unistd.h>
#define sleep(x) usleep(x*1000)
#endif

#ifdef WIN32
#define STRCPY strcpy_s
#define SPRINTF sprintf_s
#else
#define STRCPY strcpy
#define SPRINTF sprintf
#endif