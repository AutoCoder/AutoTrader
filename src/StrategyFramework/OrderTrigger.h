#ifndef POLICY_CLASS_H
#define POLICY_CLASS_H

#include "stdafx.h"
#include <memory>
#include <list>
#include <vector>
#include "AP_Mgr.h"
#include "Order.h"
#include "TickWrapper.h"
#include "KData.h"

class STRATEGY_API OrderTriggerBase
{
public:
	OrderTriggerBase(){};
	virtual ~OrderTriggerBase(){};
	virtual bool tryInvoke(const std::vector<TickWrapper>& data, TickWrapper& info, Order& order) = 0;
	virtual bool tryInvoke(const std::vector<TickWrapper>& tickdata, const std::vector<KData>& data, const std::vector<TickWrapper>& curmindata, TickWrapper& info, Order& order) = 0;

	virtual void BindWithAccount(AP::AccountDetailMgr*) = 0;
};


template <typename P, typename S, typename ... Args>
class OrderTrigger : public OrderTriggerBase
{
public:
	OrderTrigger(Args ... args)
	{
		//m_positionCtl = std::make_unique<P>();
		//m_strategy = std::make_unique<S>(args...);
		//to pass build in Clang 3.6
		m_positionCtl = std::unique_ptr<P>(new P());
		m_strategy = std::unique_ptr<S>(new S(args...));
	}
	virtual ~OrderTrigger(){}

	virtual bool tryInvoke(const std::vector<TickWrapper>& data, TickWrapper& info, Order& order){
		if (m_strategy->tryInvoke(data, info))
		{
			order = m_strategy->GetCurOrder();
			m_positionCtl->completeOrder(order);
			return true;
		}
		else
			return false;
	}

	virtual bool tryInvoke(const std::vector<TickWrapper>& tickdata, const std::vector<KData>& data, const std::vector<TickWrapper>& curmindata, TickWrapper& info, Order& order){
		if (m_strategy->tryInvoke(tickdata, data, curmindata, info))
		{
			order = m_strategy->GetCurOrder();
			return m_positionCtl->completeOrder(order);
		}
		else
			return false;
	}

	virtual void BindWithAccount(AP::AccountDetailMgr* mgr){
		m_positionCtl->BindAccount(mgr);
	}

private:
	std::unique_ptr<P>    m_positionCtl;
	std::unique_ptr<S>    m_strategy;
};

#endif