#include "Pos20Precent.h"
#include "Order.h"
#include "PPMgr.h"
#include "crossplatform.h"

Pos20Precent::Pos20Precent()
{

}

bool Pos20Precent::CompleteOrders(OrderVec& orders){
	if (!m_ppMgr && orders.size() == 1)
		return false;

	Order& ord = orders[0];
	size_t long_pos = m_ppMgr->GetUnclosedPosition(ord.GetInstrumentId(), THOST_FTDC_D_Buy);
	size_t short_pos = m_ppMgr->GetUnclosedPosition(ord.GetInstrumentId(), THOST_FTDC_D_Sell);
	size_t long_ydpos = m_ppMgr->GetYDUnclosedPosition(ord.GetInstrumentId(), THOST_FTDC_D_Buy);
	size_t short_ydpos = m_ppMgr->GetYDUnclosedPosition(ord.GetInstrumentId(), THOST_FTDC_D_Sell);
	/*
	double pos_money = m_ppMgr->GetFrozenMargin() + m_ppMgr->GetUsedMargin();
	double available = m_ppMgr->GetAvailableMoney();
	double purchaseMoney = (available + pos_money) * 0.2; // 20% up-limit line

	auto gen_open_order = [&ord](double available, double pos_money, double purchaseMoney) -> bool {
		if (purchaseMoney > available / 4) // >20%
			return false;

		int vol = purchaseMoney / ord.GetRefExchangePrice();
		if (vol == 0)
			return false;

		ord.SetCombOffsetFlagType(THOST_FTDC_OF_Open);
		ord.SetVolume(vol);
		return true;
	};*/

	//200 up-limit
	const int uplimit = 200;
	auto gen_open_order = [&](int uplimit) -> bool {
		int vol = 0;
		if (ord.GetExchangeDirection() == THOST_FTDC_D_Buy){
			vol = uplimit - long_pos;
		}
		else if (ord.GetExchangeDirection() == THOST_FTDC_D_Sell){
			vol = uplimit - short_pos;
		}
		else
			assert(false);

		if (vol == 0)
			return false;

		ord.SetCombOffsetFlagType(THOST_FTDC_OF_Open);
		ord.SetVolume(vol);
		return true;
	};		

	if (long_pos == short_pos){  // current position is 0, open position
		return gen_open_order(uplimit);
	}
	else{ // close position

		TThostFtdcDirectionType pos_direction = long_pos < short_pos ? THOST_FTDC_D_Sell : THOST_FTDC_D_Buy;
		if (ord.GetExchangeDirection() == pos_direction){
			return gen_open_order(uplimit);
		}
		else{
			// if yd pos is empty, use THOST_FTDC_OF_CloseToday
			// if yd pos is not empty, use THOST_FTDC_OF_Close
			char combOffsetFlag = (short_ydpos == long_ydpos ? THOST_FTDC_OF_CloseToday : THOST_FTDC_OF_Close);

			if (pos_direction == THOST_FTDC_D_Sell){
				ord.SetCombOffsetFlagType(combOffsetFlag);
				ord.SetVolume(short_pos - long_pos);
			}
			else{ // THOST_FTDC_D_Buy
				ord.SetCombOffsetFlagType(combOffsetFlag);
				ord.SetVolume(long_pos - short_pos);
			}

			return true;
		}

	}
}
