#ifdef WIN32
#define dllExport __declspec(dllexport)
#else
#define dllExport
#endif
extern "C" dllExport void LoadPlugin();
extern "C" dllExport void FreePlugin();
