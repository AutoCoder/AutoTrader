#ifndef TRIGGER_FACTORY_H
#define TRIGGER_FACTORY_H

#include <map>
#include <string>


class OrderTriggerBase;

class TriggerFactory{
public:
	TriggerFactory();
	OrderTriggerBase* GetTrigger(const std::string& name);

private:
	std::map<std::string, OrderTriggerBase*> m_factory;
};

#endif