#include "TestCancelPosManage.h"
#include "Order.h"
#include "AP_Mgr.h"


TestCancelPosManage::TestCancelPosManage(int quantity)
	: m_quantity(quantity)
{

}

bool Pos1Shou::CompleteOrders(OrderVec& orders){

	for (auto& ord : orders){
		ord.SetCombOffsetFlagType(THOST_FTDC_OF_Open);
		ord.SetVolume(vol);
	}

	return true;
}
