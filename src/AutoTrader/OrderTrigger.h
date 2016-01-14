#ifndef POLICY_CLASS_H
#define POLICY_CLASS_H

#include <memory>
#include <list>
#include <vector>
#include "AP_Mgr.h"
#include "Order.h"
#include "TickWrapper.h"
#include "KData.h"

class OrderTriggerBase
{
public:
	OrderTriggerBase(){};
	virtual ~OrderTriggerBase(){};
	virtual bool tryInvoke(const std::list<TickWrapper>& data, TickWrapper& info, Order& order) = 0;
	virtual bool tryInvoke(const std::list<TickWrapper>& tickdata, const std::vector<KData>& data, std::vector<TickWrapper> curmindata, TickWrapper& info, Order& order) = 0;
};


template <typename P, typename S, typename ... Args>
class OrderTrigger : public OrderTriggerBase
{
public:
	OrderTrigger(AP::AccountDetailMgr* ap, Args ... args)
	{
		m_positionCtl = std::make_unique<P>(ap);
		m_strategy = std::make_unique<S>(args...);
	}
	virtual ~OrderTrigger(){}

	virtual bool tryInvoke(const std::list<TickWrapper>& data, TickWrapper& info, Order& order){
		if (m_strategy->tryInvoke(data, info))
		{
			order = m_strategy->GetCurOrder();
			m_positionCtl->completeOrder(order);
			return true;
		}
		else
			return false;
	}

	virtual bool tryInvoke(const std::list<TickWrapper>& tickdata, const std::vector<KData>& data, std::vector<TickWrapper> curmindata, TickWrapper& info, Order& order){
		if (m_strategy->tryInvoke(tickdata, data, curmindata, info))
		{
			order = m_strategy->GetCurOrder();
			m_positionCtl->completeOrder(order);
			return true;
		}
		else
			return false;
	}

private:
	std::unique_ptr<P>    m_positionCtl;
	std::unique_ptr<S>    m_strategy;
};
#endif