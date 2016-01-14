#include "stdafx.h"
#include "OrderTrigger.h"
#include "TriggerFactory.h"
#include "IPositionControl.h"
#include "MACrossStratgy.h"


TriggerFactory::TriggerFactory(){
	OrderTriggerBase* p1 = new OrderTrigger<Pos20Precent, MACrossStratgy, int, int>(nullptr, 3 ,5);
	m_factory["MACross,3,5"] = new OrderTrigger<Pos20Precent, MACrossStratgy, int, int>(nullptr, 3, 5);
}

OrderTriggerBase* TriggerFactory::GetTrigger(const std::string& name){
	if (m_factory.find(name) != m_factory.end()){
		return m_factory[name];
	}
	else
		return false;
}
