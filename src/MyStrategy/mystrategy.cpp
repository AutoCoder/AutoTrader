#include "mystrategy.h"
#include "OrderTrigger.h"
#include "TriggerFactory.h"
#include "OrderTrigger.h"
#include "MACrossStratgy.h"
#include "Pos20Precent.h"
#include <string>

std::string AccountName(){
	std::string ret = "9999";
	ret += "021510";
	return ret;
}

void RegisterAllStrategy(){
	auto factory = TriggerFactory::Instance();
	OrderTriggerBase* st1 = new OrderTrigger<Pos20Precent,MACrossStratgy, int, int>(3, 5);
	factory->RegisterTrigger(AccountName(), "Pos20Precent_3_5_MACrossStratgy", st1);
}

void UnRegisterAllStrategy(){
	auto factory = TriggerFactory::Instance();
	factory->UnRegisterTrigger(AccountName());
}

//#ifdef WIN32
//#include <Windows.h>
//
//
//BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
//{
//	//printf("hModule.%p lpReserved.%p \n", hModule, lpReserved);
//
//	switch (ul_reason_for_call)
//	{
//	case DLL_PROCESS_ATTACH:
//		RegisterAllStrategy();
//		break;
//
//	case DLL_PROCESS_DETACH:
//		UnRegisterAllStrategy();
//		break;
//
//	case DLL_THREAD_ATTACH:
//		//printf("Thread attach. \n");
//		break;
//
//	case DLL_THREAD_DETACH:
//		//printf("Thread detach. \n");
//		break;
//	}
//
//	return (TRUE);
//}
//
//#else
//#endif