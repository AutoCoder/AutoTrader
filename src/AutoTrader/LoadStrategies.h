#include <vector>
#ifdef WIN32
#include <Windows.h>
typedef void(*loadFunc)(void);


struct StrategyPluginsLoader
{
	StrategyPluginsLoader(){
		HMODULE  handle = LoadLibrary(L"MyStrategy.dll");

		m_handles.push_back(handle);

		for (HMODULE handle : m_handles){
			loadFunc funPtr = (loadFunc)GetProcAddress(handle, "RegisterAllStrategy");
			funPtr();
		}	
	}

	~StrategyPluginsLoader(){
		for (HMODULE handle : m_handles){
			loadFunc funPtr = (loadFunc)GetProcAddress(handle, "UnRegisterAllStrategy");
			funPtr();
			FreeLibrary(handle);
		}
	}

private:
	std::vector<HMODULE> m_handles;
};


#else


#endif