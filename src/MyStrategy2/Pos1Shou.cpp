#include "Pos1Shou.h"
#include "Order.h"
#include "AP_Mgr.h"


Pos1Shou::Pos1Shou(int quantity)
	: m_quantity(quantity)
{

}

bool Pos1Shou::CompleteOrders(OrderVec& orders){
	if (!m_detailMgr && orders.size() == 2)
		return false;

	if (orders[0].GetExchangeDirection() == THOST_FTDC_D_Buy && orders[1].GetExchangeDirection() == THOST_FTDC_D_Sell){
		if (m_detailMgr->UnClosedVolumeOfLong(orders[0].GetInstrumentId()) == 0 &&
			m_detailMgr->UnClosedVolumeOfShort(orders[1].GetInstrumentId()) == 0)
		{
			orders[0].SetVolume(m_quantity);
			orders[0].SetVolume(m_quantity);
			return true;
		}
	}

	return false;
}
