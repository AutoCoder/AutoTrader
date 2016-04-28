#ifndef BPSL_STRATEGY_H
#define BPSL_STRATEGY_H


#include "Order.h"
#include <vector>
#include "KData.h"
#include "Strategy.h"

class BigProfitSmallLoseStrategy
{
public:
	BigProfitSmallLoseStrategy(double max_profit_ratio, double max_lose_ratio, int threshold_volume);
	virtual ~BigProfitSmallLoseStrategy();

	bool tryInvoke(const std::vector<TickWrapper>& data, TickWrapper& info);
	bool tryInvoke(const std::vector<TickWrapper>& tickdata, const std::vector<KData>& data, const std::vector<TickWrapper>& curmindata, TickWrapper& info);

	OrderVec pendingOrders() const;

protected:
	double _max_profit_ratio;
	double _max_lose_ratio;
	int    _threshold_volume;
private:
	OrderVec m_pendingOrders;
};


#endif