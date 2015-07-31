#include "stdafx.h"
#include "BaseAccountMgr.h"
#include "Order.h"
#include <memory>

BaseAccountMgr::BaseAccountMgr()
{
}


BaseAccountMgr::~BaseAccountMgr()
{
}


void BaseAccountMgr::update(const CThostFtdcTradingAccountField& info){
	memcpy(&m_accountInfo, &info, sizeof(CThostFtdcTradingAccountField));
	m_isUpdated = true;
}

bool BaseAccountMgr::completeOrder(Order& ord){
	while (!m_isUpdated){
		sleep(20);
	}
	double purchaseMoney = m_accountInfo.Available - (m_accountInfo.Balance * 0.8);
	//Get price == ord.GetInstrumentId()
	if (purchaseMoney > 0){
		int vol = purchaseMoney / ord.GetRefExchangePrice();

		char flag = (vol == 0) ? THOST_FTDC_OF_CloseToday : THOST_FTDC_OF_Open;
		ord.SetCombOffsetFlagType(flag);
		ord.SetVolume(vol);
		return true;
	}
	else{
		return false;
	}

}