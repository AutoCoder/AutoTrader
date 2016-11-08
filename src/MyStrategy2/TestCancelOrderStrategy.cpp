#include "TestCancelOrderStrategy.h"
#include "TickWrapper.h"

TestCancelOrderStrategy::TestCancelOrderStrategy()
{
}

TestCancelOrderStrategy::~TestCancelOrderStrategy()
{
}

bool TestCancelOrderStrategy::tryInvoke(const std::vector<TickWrapper>& data, TickWrapper& info){
	static int invokeIdx = 0;

	//invoke order every 10 tick
	if (++invokeIdx % 10 == 0)
	{
		Order order1;
		order1.SetInstrumentId(info.InstrumentId());
		order1.SetOrderType(Order::LimitPriceFOKOrder);
		order1.SetRefExchangePrice(info.LastPrice() - 10);
		order1.SetExchangeDirection(THOST_FTDC_D_Buy);
		m_pendingOrders.push_back(order1);
	}

	return true;
}

bool TestCancelOrderStrategy::tryInvoke(const std::vector<TickWrapper>& tickdata, const std::vector<KData>& data, const std::vector<TickWrapper>& curmindata, TickWrapper& info){
	return false;
}

OrderVec TestCancelOrderStrategy::pendingOrders() const{
	return m_pendingOrders;
}
