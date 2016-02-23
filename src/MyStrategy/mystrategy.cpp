#include "mystrategy.h"
#include "OrderTrigger.h"
#include "TriggerFactory.h"
#include "OrderTrigger.h"
#include "MACrossStratgy.h"
#include "Pos20Precent.h"
#include <string>
#include "AccountMgr.h"

OrderTriggerBase* st1;

std::string AccountId(){
	std::string ret = "9999";
	ret += "021510";
	return ret;
}

void RegisterAccountMeta(){
	Account::Meta m("9999", "021510", "wodemima", { "rb1605", "rb1606" });
	Account::Manager::Instance().AddAccontMeta(m);
}
void UnRegisterAccountMeta(){
	
	Account::Manager::Instance().RemoveAccontMeta(AccountId());
}

void RegisterAllStrategy(){
	auto factory = TriggerFactory::Instance();
	st1 = new OrderTrigger<Pos20Precent, MACrossStratgy, int, int>(3, 5);
	factory->RegisterTrigger(AccountId(), "Pos20Precent_3_5_MACrossStratgy", st1);
}

void UnRegisterAllStrategy(){
	auto factory = TriggerFactory::Instance();
	factory->UnRegisterTrigger(AccountId());
	delete st1;
}

void LoadPlugin(){
	RegisterAllStrategy();
	RegisterAccountMeta();
}

void FreePlugin(){
	UnRegisterAllStrategy();
	UnRegisterAccountMeta();
}
