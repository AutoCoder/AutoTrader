#include "stdafx.h"
#include "IPositionControl.h"
#include "Order.h"
#include "AP_Mgr.h"
#include "crossplatform.h"
#include "AP_Mgr.h"

Pos20Precent::Pos20Precent(AP::AccountDetailMgr* mgr)
	:m_detailMgr(mgr)
{

}

bool Pos20Precent::completeOrder(Order& ord){
	if (!m_detailMgr)
		return false;

	double posMoney = 0.0;
	double available = 0.0;

	AP::Direction posDirection = AP::Long;
	m_detailMgr->getPosition(posMoney, posDirection, available);

	int subPos = 0;
	int subtodayPos = 0;
	int subydPos = 0;
	AP::Direction subtodayDirection = AP::Long;
	AP::Direction subydDirection = AP::Long;
	subPos = m_detailMgr->getPositionVolume(ord.GetInstrumentId(), subtodayDirection, subtodayPos, subydDirection, subydPos);

	//如果订单买卖方向与持仓买卖方向一致,或者仓位为0。
	if (ord.GetExchangeDirection() == posDirection || posMoney < std::numeric_limits<double>::epsilon()){
		if (posMoney > available * 0.2){
			//仓位已经超过2成， 则放弃该订单
			return false;
		}
		else{
			double purchaseMoney = available*0.2 - posMoney;
			int vol = purchaseMoney / ord.GetRefExchangePrice();

			ord.SetCombOffsetFlagType(THOST_FTDC_OF_Open);
			ord.SetVolume(vol);
			return true;
		}
	}
	else{
		if (subydPos != 0 && subtodayPos != 0){
			if (subtodayDirection == subydDirection){
				ord.SetCombOffsetFlagType(THOST_FTDC_OF_Close);
				ord.SetVolume(subydPos + subtodayPos);
			}
			else{
				ord.SetCombOffsetFlagType(THOST_FTDC_OF_Close);
				ord.SetVolume(std::abs(subydPos-subtodayPos));
			}
		}
		else{
			if (subtodayPos != 0){
				ord.SetCombOffsetFlagType(THOST_FTDC_OF_CloseToday);
				ord.SetVolume(subtodayPos);
			}
			else if (subydPos != 0){
				ord.SetCombOffsetFlagType(THOST_FTDC_OF_Close);
				ord.SetVolume(subydPos);
			}
			else{ // subydPos == 0 && subtodayPos == 0
				return false;
			}
		}
		return true;
	}
}
