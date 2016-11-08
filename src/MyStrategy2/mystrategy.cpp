#include "mystrategy.h"
#include "OrderTrigger.h"
#include "TriggerFactory.h"
#include "OrderTrigger.h"
#include "BigProfitSmallLoseStrategy.h"
#include "TestCancelOrderStrategy.h"
#include "Pos1Shou.h"
#include "TestCancelPosManage.h"
#include <string>
#include "AccountMgr.h"

namespace {

OrderTriggerBase* st1;
OrderTriggerBase* st2;

std::string AccountId(){
	std::string ret = "9999";
	ret += "038775";
	return ret;
}

void RegisterAccountMeta(){
	Account::Meta m("9999", "038775", "wodemima", { "rb1701","RM1701", "FG1701", "i1701" });
	Account::Manager::Instance().AddAccontMeta(m);
}
void UnRegisterAccountMeta(){
	Account::Manager::Instance().RemoveAccontMeta(AccountId());
}

void RegisterAllStrategy(){
	auto factory = TriggerFactory::Instance();
	st1 = new OrderTrigger<Pos1Shou, BigProfitSmallLoseStrategy, double, double, int>(0.04, 0.02, 500);
	st2 = new OrderTrigger<TestCancelPosManage, TestCancelOrderStrategy>();
	factory->RegisterTrigger(AccountId(), "BPSL_4_2_500", st1);
	factory->RegisterTrigger(AccountId(), "TestCancelStg", st2);
}

void UnRegisterAllStrategy(){
	auto factory = TriggerFactory::Instance();
	factory->UnRegisterTrigger(AccountId());
	//factory->UnRegisterTrigger(AccountId2());
	delete st1;
	delete st2;
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
