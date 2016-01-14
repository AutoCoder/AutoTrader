#ifndef TRIGGER_FACTORY_H
#define TRIGGER_FACTORY_H

#include <map>
#include <string>

#ifndef MustImpl
class TriggerFactory{

public:
	static OrderTriggerBase* GetTrigger();
};
#else
class OrderTriggerBase;

class TriggerFactory{
public:
	TriggerFactory();
	OrderTriggerBase* GetTrigger(const std::string& name);

private:
	std::map<std::string, OrderTriggerBase*> m_factory;
};
#endif

#endif