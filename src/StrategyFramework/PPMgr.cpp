#include <assert.h>
#include <algorithm>
#include <sstream>
#include <string.h>
#include "PPMgr.h"
#include "InstrumentInfoMgr.h"
#include "CommonUtils.h"

namespace PP {

	CThostFtdcInvestorPositionFieldWrapper::CThostFtdcInvestorPositionFieldWrapper(){
		memset(&m_LongPos, 0, sizeof(CThostFtdcInvestorPositionField));
		memset(&m_ShortPos, 0, sizeof(CThostFtdcInvestorPositionField));
		m_LongPos.PosiDirection = THOST_FTDC_PD_Net;
		m_ShortPos.PosiDirection = THOST_FTDC_PD_Net;

	}

	bool CThostFtdcInvestorPositionFieldWrapper::IsLongPosEmpty(){
		return m_LongPos.PosiDirection == THOST_FTDC_PD_Net;;
	}

	bool CThostFtdcInvestorPositionFieldWrapper::IsShortPosEmpty(){
		return m_ShortPos.PosiDirection == THOST_FTDC_PD_Net;
	}

	std::string CThostFtdcInvestorPositionFieldWrapper::ToString(){
		std::stringstream result;

		result << "-----------------------------------" << std::endl;
		if (!IsLongPosEmpty()){
			result << "LongPositon =>{ " << CommonUtils::ConvertPositionFieldToString(m_LongPos) << " }" << std::endl;
		}

		if (!IsShortPosEmpty()){
			result << "ShortPositon =>{ " << CommonUtils::ConvertPositionFieldToString(m_ShortPos) << " }" << std::endl;
		}

		result << "-----------------------------------" << std::endl;

		return result.str();
	}

	CThostFtdcInvestorPositionFieldWrapper& CThostFtdcInvestorPositionFieldWrapper::operator +=(const CThostFtdcInvestorPositionField& other){
		if (other.PosiDirection == THOST_FTDC_PD_Long)
		{
			//初始化中，可能会有同一个合约的多次回报
			if (IsLongPosEmpty()){
				memcpy(&m_LongPos, &(other), sizeof(CThostFtdcInvestorPositionField));
			}
			else{
				m_LongPos.Position += other.Position;
				m_LongPos.TodayPosition += other.TodayPosition;
				//m_LongPos.YdPosition += other.YdPosition;
				//##According debuging, I found YdPosition may not correct when tradepi callback
				m_LongPos.YdPosition = m_LongPos.Position - m_LongPos.TodayPosition;
				m_LongPos.LongFrozen += other.LongFrozen;
				m_LongPos.FrozenMargin += other.FrozenMargin;
				m_LongPos.FrozenCommission += other.FrozenCommission;
				m_LongPos.LongFrozenAmount += other.LongFrozenAmount;
				m_LongPos.PositionCost = (m_LongPos.PositionCost * m_LongPos.OpenVolume + other.PositionCost * other.OpenVolume) / (m_LongPos.OpenVolume + other.OpenVolume);
				m_LongPos.OpenVolume += other.OpenVolume;
				m_LongPos.CloseVolume += other.CloseVolume;
				m_LongPos.OpenAmount += other.OpenAmount;
				m_LongPos.CloseAmount += other.CloseAmount;
				m_LongPos.UseMargin += other.UseMargin;
				m_LongPos.Commission += other.Commission;
				m_LongPos.PosiDirection = other.PosiDirection;
			}
		}
		else if (other.PosiDirection == THOST_FTDC_PD_Short)
		{
			if (IsShortPosEmpty()){
				memcpy(&m_ShortPos, &(other), sizeof(CThostFtdcInvestorPositionField));
			}
			else{
				m_ShortPos.Position += other.Position;
				m_ShortPos.TodayPosition += other.TodayPosition;
				//m_ShortPos.YdPosition += other.YdPosition;
				//##According debuging, I found YdPosition may not correct when tradepi callback
				m_ShortPos.YdPosition = m_ShortPos.Position - m_ShortPos.TodayPosition;
				m_ShortPos.ShortFrozen += other.ShortFrozen;
				m_ShortPos.FrozenCommission += other.FrozenCommission;
				m_ShortPos.ShortFrozenAmount += other.ShortFrozenAmount;
				m_ShortPos.PositionCost = (m_ShortPos.PositionCost * m_ShortPos.OpenVolume + other.PositionCost * other.OpenVolume) / (m_ShortPos.OpenVolume + other.OpenVolume);
				m_ShortPos.OpenVolume += other.OpenVolume;
				m_ShortPos.CloseVolume += other.CloseVolume;
				m_ShortPos.OpenAmount += other.OpenAmount;
				m_ShortPos.CloseAmount += other.CloseAmount;
				m_ShortPos.UseMargin += other.UseMargin;
				m_ShortPos.Commission += other.Commission;
				m_ShortPos.PosiDirection = other.PosiDirection;
			}
		}
		else{
			//do nothing
		}

		return *this;
	}

	CThostFtdcInvestorPositionFieldWrapper& CThostFtdcInvestorPositionFieldWrapper::operator +=(const CThostFtdcTradeField& trade){

		auto initPosFieldFunc = [&](const CThostFtdcTradeField& tradeField, CThostFtdcInvestorPositionField& posField) -> void {
			//!!!Note:如果当前仓位为空，那么必然是开仓
			assert(tradeField.OffsetFlag == THOST_FTDC_OF_Open);
			//Original Position is empty, so should be initialize here
			strcpy(posField.InstrumentID, tradeField.InstrumentID);
			strcpy(posField.BrokerID, tradeField.BrokerID);
			strcpy(posField.InvestorID, tradeField.InvestorID);
			posField.PosiDirection = tradeField.Direction + 2;
			posField.HedgeFlag = tradeField.HedgeFlag;
			posField.PositionDate = THOST_FTDC_PSD_Today;
			posField.YdPosition = 0;
			posField.TodayPosition = tradeField.Volume;
			posField.Position = posField.TodayPosition; //仓位＝开仓量－平仓量
			posField.LongFrozen = 0;
			posField.LongFrozenAmount = 0;
			posField.OpenVolume = tradeField.Volume; //开仓量
			posField.CloseVolume = 0; //平仓量
			posField.OpenAmount = tradeField.Price * tradeField.Volume; //开仓金额
			posField.CloseAmount = 0; //平仓金额
			//!!!Note: 放弃维护这四个字段，只维护当前账户总冻结手数和冻结金额
			// ///多头冻结
			// TThostFtdcVolumeType	LongFrozen;
			// ///空头冻结
			// TThostFtdcVolumeType	ShortFrozen;
			// ///开仓冻结金额
			// TThostFtdcMoneyType	LongFrozenAmount;
			// ///开仓冻结金额
			// TThostFtdcMoneyType	ShortFrozenAmount;
			posField.PositionCost = tradeField.Price;
			posField.PreMargin = 0;
			posField.UseMargin += InstrumentManager.GetMargin(tradeField.InstrumentID, tradeField.Volume, tradeField.Price, tradeField.Direction);
			posField.Commission += InstrumentManager.GetCommission(tradeField.InstrumentID, tradeField.Volume, tradeField.Price, trade.OffsetFlag);
			
			strcpy(posField.TradingDay, tradeField.TradeDate);
		};

		auto appendPosFunc = [&](const CThostFtdcTradeField& tradeField, CThostFtdcInvestorPositionField& posField) -> void {
			double amount = posField.PositionCost * posField.Position;
			double delta_amount = tradeField.Volume * tradeField.Price;
			posField.PreMargin = posField.UseMargin;

			//update Position
			if (tradeField.OffsetFlag == THOST_FTDC_OF_Open )
			{
				posField.TodayPosition += tradeField.Volume;
				posField.Position = posField.TodayPosition + posField.YdPosition;
				posField.OpenVolume += tradeField.Volume; //更新开仓量
				posField.OpenAmount += tradeField.Price * tradeField.Volume;
				posField.PositionCost = (amount + delta_amount) / posField.Position;
			}
			else if (THOST_FTDC_OF_Close == tradeField.OffsetFlag || THOST_FTDC_OF_ForceClose == tradeField.OffsetFlag || 
				THOST_FTDC_OF_CloseToday == tradeField.OffsetFlag || THOST_FTDC_OF_CloseYesterday == tradeField.OffsetFlag){
				posField.TodayPosition -= tradeField.Volume;
				posField.Position = posField.TodayPosition + posField.YdPosition;
				posField.CloseVolume += tradeField.Volume; //更新平仓量
				posField.CloseAmount += tradeField.Price * tradeField.Volume;
				posField.PositionCost = (amount - delta_amount) / posField.Position;
			}

			posField.UseMargin += InstrumentManager.GetMargin(tradeField.InstrumentID, tradeField.Volume, tradeField.Price, tradeField.Direction);
			posField.Commission += InstrumentManager.GetCommission(tradeField.InstrumentID, tradeField.Volume, tradeField.Price, trade.OffsetFlag);
			
		};

		if (trade.Direction == THOST_FTDC_D_Buy)
		{
			if (IsLongPosEmpty())
				initPosFieldFunc(trade, m_LongPos);
			else{
				//开多仓 传入m_LongPos  平空仓 传入m_ShortPos
				appendPosFunc(trade, trade.OffsetFlag == THOST_FTDC_OF_Open ? m_LongPos : m_ShortPos);
			}
		}
		else if (trade.Direction == THOST_FTDC_D_Sell)
		{	
			if (IsShortPosEmpty())
				initPosFieldFunc(trade, m_ShortPos);				
			else{
				//开空仓 传入m_ShortPos 平多仓 传入m_LongPos 
				appendPosFunc(trade, trade.OffsetFlag == THOST_FTDC_OF_Open ? m_ShortPos : m_LongPos);
			}
		}
		else{
			assert(false);
		}

		return *this;
	}

	void CThostFtdcInvestorPositionFieldWrapper::OnOrder(const CThostFtdcOrderField& orderField, OrderCallBackType ordCBType){
		if (OrderCallBackType::FinishOrder == ordCBType || OrderCallBackType::CancellOrder == ordCBType || OrderCallBackType::InsertOrder == ordCBType){

			///double commission = commratio * orderField.VolumeTotalOriginal;
			//Only THOST_FTDC_OPT_LimitPrice is permitted for now.
			assert(orderField.OrderPriceType == THOST_FTDC_OPT_LimitPrice);
			double commission = InstrumentManager.GetCommission(orderField.InstrumentID, orderField.VolumeTotalOriginal, orderField.LimitPrice, orderField.CombOffsetFlag[0]);
			double margin = InstrumentManager.GetCommission(orderField.InstrumentID, orderField.VolumeTotalOriginal, orderField.LimitPrice, orderField.Direction);

			switch(ordCBType){
				case OrderCallBackType::InsertOrder:
				{
					if (orderField.Direction == THOST_FTDC_D_Buy){
						m_LongPos.LongFrozen += orderField.VolumeTotalOriginal;
						m_LongPos.FrozenMargin += margin;
						m_LongPos.FrozenCommission += commission;
					}
					else{ // THOST_FTDC_D_Sell
						m_ShortPos.ShortFrozen += orderField.VolumeTotalOriginal;
						m_ShortPos.FrozenMargin += margin;
						m_ShortPos.FrozenCommission += commission;
					}				
				}
				break;
				case OrderCallBackType::FinishOrder:
				case OrderCallBackType::CancellOrder:
				{
					if (orderField.Direction == THOST_FTDC_D_Buy){
						m_LongPos.LongFrozen -= orderField.VolumeTotalOriginal;
						m_LongPos.FrozenMargin -= margin;
						m_LongPos.FrozenCommission -= commission;
					}
					else{ //THOST_FTDC_D_Sell
						m_ShortPos.ShortFrozen -= orderField.VolumeTotalOriginal;
						m_ShortPos.FrozenMargin -= margin;
						m_ShortPos.FrozenCommission -= commission;
					}
				}
				default:
					break;
			}
		}		
	}

	double CThostFtdcInvestorPositionFieldWrapper::GetMargin() const{
		return std::max(m_LongPos.UseMargin, m_ShortPos.UseMargin);
	}


	double CThostFtdcInvestorPositionFieldWrapper::GetFrozenMargin() const{
		return std::max(m_LongPos.FrozenMargin, m_ShortPos.FrozenMargin);
	}


	double CThostFtdcInvestorPositionFieldWrapper::GetCommission() const{
		return m_LongPos.Commission + m_ShortPos.Commission;
	}
		
	double CThostFtdcInvestorPositionFieldWrapper::GetFrozenCommission() const{
		return m_LongPos.FrozenCommission + m_ShortPos.FrozenCommission;
	}

	PositionProfitMgr::PositionProfitMgr()
		:m_acccountInfoInitialized(false)
	{
		memset(&m_accountInfo, 0, sizeof(CThostFtdcTradingAccountField));
	}


	PositionProfitMgr::~PositionProfitMgr()
	{
	}


	bool PositionProfitMgr::PushOrder(const CThostFtdcOrderField& orderField){
		//假设上一交易日的报单，会被自动取消。
		if (m_acccountInfoInitialized){
			// Update m_orderFieldVec
			auto iter = std::find_if(m_orderFieldVec.begin(), m_orderFieldVec.end(), IsSameOrder<CThostFtdcOrderField>(orderField));
			OrderCallBackType ordCBType = OrderCallBackType::Unknown;
			if (orderField.OrderStatus == THOST_FTDC_OST_AllTraded)
				ordCBType = OrderCallBackType::FinishOrder;
			else if (orderField.OrderStatus == THOST_FTDC_OST_Canceled)
				ordCBType = OrderCallBackType::CancellOrder;
			else if (iter == m_orderFieldVec.end())
				ordCBType = OrderCallBackType::InsertOrder;
			else 
				ordCBType = OrderCallBackType::Other;

			switch(ordCBType)
			{
				case OrderCallBackType::InsertOrder:
					m_orderFieldVec.push_back(orderField);
					break;
				case OrderCallBackType::CancellOrder:
				case OrderCallBackType::FinishOrder:
					m_orderFieldVec.erase(iter);
					break;
				case OrderCallBackType::Unknown:
				case OrderCallBackType::Other:
					break;
				default:
					break;
			}

			//update frozon part of position.
			m_posFieldMap[orderField.InstrumentID].OnOrder(orderField, ordCBType);
		}

		return m_acccountInfoInitialized;
	}


	void PositionProfitMgr::PushTrade(const CThostFtdcTradeField& tradeField){

		if (m_acccountInfoInitialized){			
			m_posFieldMap[tradeField.InstrumentID] += tradeField;
		}
	}

	// CThostFtdcInvestorPositionField PositionProfitMgr::ToPositionInfo(const CThostFtdcTradeField& tradeField)
	// {
	// 	CThostFtdcInvestorPositionField newPosInfo;
	// 	memset(&newPosInfo, 0, sizeof(CThostFtdcInvestorPositionField));

	// 	memcpy(newPosInfo.BrokerID, tradeField.BrokerID, sizeof(newPosInfo.BrokerID));
	// 	memcpy(newPosInfo.InvestorID, tradeField.InvestorID, sizeof(newPosInfo.InvestorID));
	// 	memcpy(newPosInfo.InstrumentID, tradeField.InstrumentID, sizeof(newPosInfo.InstrumentID));
	// 	newPosInfo.HedgeFlag = tradeField.HedgeFlag;
	// 	newPosInfo.TodayPosition = tradeField.Volume;

	// 	/* PosiDirection
	// 	///¾»
	// 	#define THOST_FTDC_PD_Net '1'
	// 	///¶àÍ·
	// 	#define THOST_FTDC_PD_Long '2'
	// 	///¿ÕÍ·
	// 	#define THOST_FTDC_PD_Short '3'
	// 	*/

	// 	/*  Direction
	// 	///Âò
	// 	#define THOST_FTDC_D_Buy '0'
	// 	///Âô
	// 	#define THOST_FTDC_D_Sell '1'
	// 	*/
	// 	newPosInfo.PosiDirection = tradeField.Direction + 2;
	// 	newPosInfo.YdPosition = 0;
	// 	newPosInfo.Position = newPosInfo.TodayPosition + newPosInfo.YdPosition;
	// 	newPosInfo.PositionCost = tradeField.Price;

	// 	//double exchangeMoney = tradeField.Price * tradeField.Volume;

	// 	if (tradeField.Direction == THOST_FTDC_D_Buy){
	// 		newPosInfo.UseMargin = InstrumentManager.Get(tradeField.InstrumentID).MgrRateField.LongMarginRatioByVolume * tradeField.Volume;
	// 	}
	// 	else if (tradeField.Direction == THOST_FTDC_D_Sell){
	// 		newPosInfo.UseMargin = InstrumentManager.Get(tradeField.InstrumentID).MgrRateField.ShortMarginRatioByVolume * tradeField.Volume;
	// 	}

	// 	/*
	// 			///¿ª²Ö
	// 	#define THOST_FTDC_OF_Open '0'
	// 			///Æ½²Ö
	// 	#define THOST_FTDC_OF_Close '1'
	// 			///Ç¿Æ½
	// 	#define THOST_FTDC_OF_ForceClose '2'
	// 			///Æ½½ñ
	// 	#define THOST_FTDC_OF_CloseToday '3'
	// 			///Æ½×ò
	// 	#define THOST_FTDC_OF_CloseYesterday '4'
	// 			///Ç¿¼õ
	// 	#define THOST_FTDC_OF_ForceOff '5'
	// 			///±¾µØÇ¿Æ½
	// 	#define THOST_FTDC_OF_LocalForceClose '6'
	// 	*/
	// 	if (tradeField.OffsetFlag == THOST_FTDC_OF_Open){
	// 		newPosInfo.Commission = InstrumentManager.Get(tradeField.InstrumentID).ComRateField.OpenRatioByVolume * tradeField.Volume;
	// 	}
	// 	else if (THOST_FTDC_OF_Close == tradeField.OffsetFlag && THOST_FTDC_OF_CloseYesterday == tradeField.OffsetFlag)
	// 	{
	// 		newPosInfo.Commission = InstrumentManager.Get(tradeField.InstrumentID).ComRateField.CloseRatioByVolume * tradeField.Volume;
	// 	}
	// 	else if (tradeField.OffsetFlag == THOST_FTDC_OF_CloseToday)
	// 	{
	// 		newPosInfo.Commission = InstrumentManager.Get(tradeField.InstrumentID).ComRateField.CloseTodayRatioByVolume * tradeField.Volume;
	// 	}
	// 	else{
	// 		assert(false);
	// 	}

	// 	return newPosInfo;
	// }

	void PositionProfitMgr::SetAccountInfo(const CThostFtdcTradingAccountField& info) {
		memcpy(&m_accountInfo, &info, sizeof(CThostFtdcTradingAccountField));
	}

	void PositionProfitMgr::PushInvestorPosition(const CThostFtdcInvestorPositionField& posInfo) {
		m_posFieldMap[posInfo.InstrumentID] += posInfo;
	}

	void PositionProfitMgr::PushInvestorPositionDetail(const CThostFtdcInvestorPositionDetailField& posDetail) {

	}

	size_t PositionProfitMgr::GetUnclosedPosition(const std::string& instrumentId, TThostFtdcDirectionType type) const{
		if (m_posFieldMap.find(instrumentId) != m_posFieldMap.end()){
			if (type == THOST_FTDC_D_Buy){
				return m_posFieldMap.at(instrumentId).GetLongPos();
			}
			else if (type == THOST_FTDC_D_Sell){
				return m_posFieldMap.at(instrumentId).GetShortPos();
			}
			else{
				assert(false);
				return 0;
			}
		}
		else{
			return 0;
		}
	}

	size_t PositionProfitMgr::GetYDUnclosedPosition(const std::string& instrumentId, TThostFtdcDirectionType type) const{
		if (m_posFieldMap.find(instrumentId) != m_posFieldMap.end()){
			if (type == THOST_FTDC_D_Buy){
				return m_posFieldMap.at(instrumentId).GetLongPos();
			}
			else if (type == THOST_FTDC_D_Sell){
				return m_posFieldMap.at(instrumentId).GetShortPos();
			}
			else{
				assert(false);
				return 0;
			}
		}
		else{
			return 0;
		}
	}

	double PositionProfitMgr::GetAvailableMoney() const{
		//todo: loop all positionField and deduct the margin and commission from them.
		double aMoney = m_accountInfo.Available;
		for (auto item : m_posFieldMap){
			aMoney -= item.second.GetMargin();
			aMoney -= item.second.GetFrozenMargin();
			aMoney -= item.second.GetCommission();
			aMoney -= item.second.GetFrozenCommission();
		}
		return aMoney;
	}

	double PositionProfitMgr::GetBalanceMoney() const {
		double bMoney = m_accountInfo.Balance;
		for (auto item : m_posFieldMap){
			bMoney -= item.second.GetMargin();
			bMoney -= item.second.GetFrozenMargin();
			bMoney -= item.second.GetCommission();
			bMoney -= item.second.GetFrozenCommission();
		}
		return bMoney;
	}

	double PositionProfitMgr::GetFrozenMargin() const{
		double ret = 0.0;
		for (auto item : m_posFieldMap){
			ret += item.second.GetFrozenMargin();
		}		
		return ret;
	}

	double PositionProfitMgr::GetUsedMargin() const{
		double ret = 0.0;
		for (auto item : m_posFieldMap){
			ret += item.second.GetMargin();
		}		
		return ret;
	}

	double PositionProfitMgr::GetCommission() const{
		double ret = 0.0;
		for (auto item : m_posFieldMap){
			ret += item.second.GetCommission();
		}		
		return ret;
	}

	double PositionProfitMgr::GetFrozenCommission() const{
		double ret = 0.0;
		for (auto item : m_posFieldMap){
			ret += item.second.GetFrozenCommission();
		}		
		return ret;
	}
	
	std::string PositionProfitMgr::ToString() const{
		std::stringstream result;
		result << "$AccountInfo =>" << std::endl;
		result << CommonUtils::ConvertAccountInfoToString(m_accountInfo) << std::endl << std::endl;

		result << "$PositionField => {" << std::endl;
		for (auto posfield : m_posFieldMap){
			result << "InstrumentID : " << posfield.first << std::endl;
			result << posfield.second.ToString();
			result << std::endl;
		}
	        result << "}" << std::endl;	
		return result.str();
	}

	std::string PositionProfitMgr::PositionOfInstruments() const{
		std::stringstream result;
		for (auto item : m_posFieldMap){
			result << item.first << " Long:" << item.second.GetLongPos() << " Short:" << item.second.GetShortPos() << "\n";
		}
		return result.str();
	}
}
