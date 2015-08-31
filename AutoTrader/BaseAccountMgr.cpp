#include "stdafx.h"
#include "crossplatform.h"
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
	m_isAccountUpdated = true;
}

void BaseAccountMgr::update(const CThostFtdcInvestorPositionField& info){
	memcpy(&m_positionInfo, &info, sizeof(CThostFtdcInvestorPositionField));
	m_isPositionUpdated = true;
}

bool BaseAccountMgr::completeOrder(Order& ord){
	while (!isUpdated()){
		sleep(20);
	}

	double purchaseMoney = m_accountInfo.Available - (m_accountInfo.Balance * 0.8);
	int vol = purchaseMoney / ord.GetRefExchangePrice();

	//如果订单买卖方向与持仓买卖方向一致。
	if (ord.GetExchangeDirection() == m_positionInfo.PosiDirection){
		if (vol == 0){
			//可用资金 == 0， 则放弃该订单
			return false;
		}
		else{
			ord.SetCombOffsetFlagType(THOST_FTDC_OF_Open);
			ord.SetVolume(vol);
			return true;
		}
	}
	else{
		ord.SetCombOffsetFlagType(THOST_FTDC_OF_CloseToday);
		ord.SetVolume(m_positionInfo.Position);
		return true;
	}
}