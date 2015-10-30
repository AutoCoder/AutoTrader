
#ifdef WIN32
#include "windows.h"
#define sleep(x) Sleep(x)
#else
#include <unistd.h>
#define sleep(x) usleep(x*1000)
#endif