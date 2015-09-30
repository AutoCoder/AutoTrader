#include "stdafx.h"
#include "crossplatform.h"
#include "BaseAccountMgr.h"
#include "Order.h"
#include "PositionMgr.h"
#include <memory>

BaseAccountMgr::BaseAccountMgr(TThostFtdcInstrumentIDType instr)
: m_isAccountUpdated(false)
, m_isPositionUpdated(false)
{
	memcpy(&m_instrument, &instr, sizeof(TThostFtdcInstrumentIDType));
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

	Position::PositionMgr& posMgr = Position::GetManager();
	double posMoney = 0.0;
	int volume = 0;
	Position::PositionDirection posDirection = Position::Buy;
	posMgr.GetPosition(posMoney, posDirection, volume);
	//如果订单买卖方向与持仓买卖方向一致。
	if (ord.GetExchangeDirection() == posDirection){
		if (posMoney > m_accountInfo.Available * 0.2){
			//仓位已经超过2成， 则放弃该订单
			return false;
		}
		else{
			double purchaseMoney = m_accountInfo.Available*0.2 - posMoney;
			int vol = purchaseMoney / ord.GetRefExchangePrice();

			ord.SetCombOffsetFlagType(THOST_FTDC_OF_Open);
			ord.SetVolume(vol);
			return true;
		}
	}
	else{
		ord.SetCombOffsetFlagType(THOST_FTDC_OF_CloseToday);
		ord.SetVolume(volume);
		return true;
	}
}