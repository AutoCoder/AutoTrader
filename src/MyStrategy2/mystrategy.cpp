#include "mystrategy.h"
#include "OrderTrigger.h"
#include "TriggerFactory.h"
#include "OrderTrigger.h"
#include "BigProfitSmallLoseStrategy.h"
#include "Pos1Shou.h"
#include <string>
#include "AccountMgr.h"

namespace {

OrderTriggerBase* st1;


std::string AccountId(){
	std::string ret = "9999";
	ret += "038775";
	return ret;
}

void RegisterAccountMeta(){
	Account::Meta m("9999", "038775", "wodemima", { "rb1605", "rb1606" });
	Account::Manager::Instance().AddAccontMeta(m);
}
void UnRegisterAccountMeta(){
	Account::Manager::Instance().RemoveAccontMeta(AccountId());
}

void RegisterAllStrategy(){
	auto factory = TriggerFactory::Instance();
	st1 = new OrderTrigger<Pos1Shou, BigProfitSmallLoseStrategy, double, double, int>(0.04, 0.02, 500);
	factory->RegisterTrigger(AccountId(), "BPSL_4_2_500", st1);
	// factory->RegisterTrigger(AccountId2(), "Pos20Precent_3_5_MACrossStratgy", st1);
}

void UnRegisterAllStrategy(){
	auto factory = TriggerFactory::Instance();
	// factory->UnRegisterTrigger(AccountId());
	// factory->UnRegisterTrigger(AccountId2());
	// delete st1;
}

}

void LoadPlugin(){
	RegisterAllStrategy();
	RegisterAccountMeta();
}

void FreePlugin(){
	UnRegisterAllStrategy();
	UnRegisterAccountMeta();
}
