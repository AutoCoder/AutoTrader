#include "OrderTrigger.h"
#include "TriggerFactory.h"

TriggerFactory* TriggerFactory::m_instance = nullptr;

TriggerFactory* TriggerFactory::Instance(){
	if (!m_instance){
		m_instance = new TriggerFactory();
	}
	return m_instance;

}
void TriggerFactory::RegisterTrigger(const std::string& ownerAccount, const std::string& strategyName, OrderTriggerBase* trigger){
	auto& stratgyMap = m_store[ownerAccount];
	stratgyMap[strategyName] = trigger;
}

void TriggerFactory::UnRegisterTrigger(const std::string& ownerAccount){
	auto iter = m_store.find(ownerAccount);
	if (iter != m_store.end()){
		m_store.erase(iter);
	}
}

OrderTriggerBase* TriggerFactory::GetTrigger(const std::string& ownerAccount, const std::string& strategyName){
	if (m_store.find(ownerAccount) != m_store.end()){
		auto stratgyMap = m_store[ownerAccount];
		if (stratgyMap.find(strategyName) != stratgyMap.end()){
			return stratgyMap[strategyName];
		}
	}

	return nullptr;
}