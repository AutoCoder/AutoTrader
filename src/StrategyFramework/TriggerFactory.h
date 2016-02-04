#ifndef TRIGGER_FACTORY_H
#define TRIGGER_FACTORY_H

#include "stdafx.h"
#include <map>
#include <string>

class OrderTriggerBase;
class STRATEGY_API TriggerFactory{

public:
	static TriggerFactory* Instance();
	void RegisterTrigger(const std::string& ownerAccount, const std::string& strategyName, OrderTriggerBase*);
	void UnRegisterTrigger(const std::string& ownerAccount);
	OrderTriggerBase* GetTrigger(const std::string& ownerAccount, const std::string& strategyName) const;
	std::vector<std::string> GetStrategies(const std::string& ownerAccount) const;
private:
	static TriggerFactory* m_instance;
	std::map < std::string/*Account Id*/, std::map<std::string/*StrategyName*/, OrderTriggerBase*> > m_store;
};

#endif