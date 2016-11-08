#include "TestCancelPosManage.h"
#include "Order.h"
#include "AP_Mgr.h"


TestCancelPosManage::TestCancelPosManage()
{

}

bool TestCancelPosManage::CompleteOrders(OrderVec& orders){

	for (auto& ord : orders){
		ord.SetCombOffsetFlagType(THOST_FTDC_OF_Open);
		ord.SetVolume(1);
	}

	return true;
}
