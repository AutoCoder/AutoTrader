#ifndef TCO_STRATEGY_H
#define TCO_STRATEGY_H


#include "Order.h"
#include <vector>
#include "KData.h"
#include "Strategy.h"

class TestCancelOrderStrategy
{
public:
	TestCancelOrderStrategy();
	virtual ~TestCancelOrderStrategy();

	bool tryInvoke(const std::vector<TickWrapper>& data, TickWrapper& info);
	bool tryInvoke(const std::vector<TickWrapper>& tickdata, const std::vector<KData>& data, const std::vector<TickWrapper>& curmindata, TickWrapper& info);

	OrderVec pendingOrders() const;

protected:

private:
	OrderVec m_pendingOrders;
};
