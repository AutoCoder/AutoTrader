#include "Pos1Shou.h"
#include "Order.h"
#include "AP_Mgr.h"


Pos1Shou::Pos1Shou(int quantity)
	: m_quantity(quantity)
{

}

bool Pos1Shou::CompleteOrders(OrderVec& orders){
	if (!m_ppMgr && orders.size() == 2){
		return false;
	}
	

	return false;
}
