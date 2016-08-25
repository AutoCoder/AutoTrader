#include <assert.h>
#include <algorithm>
#include <sstream>
#include <string.h>
#include "PPMgr.h"
#include "InstrumentInfoMgr.h"
#include "CommonUtils.h"

namespace PP {
	struct TwoSideMargin{
		TwoSideMargin()
			: Long(0.0)
			, Short(0.0)
		{}

		double Long;
		double Short;

		double GetBiggerOne() const { return Long > Short ? Long : Short; }
	};


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
				m_LongPos.LongFrozen += other.LongFrozen;
				m_LongPos.FrozenMargin += other.FrozenMargin;
				m_LongPos.FrozenCommission += other.FrozenCommission;
				m_LongPos.LongFrozenAmount += other.LongFrozenAmount;
				//m_LongPos.PositionCost = (m_LongPos.PositionCost * m_LongPos.OpenVolume + other.PositionCost * other.OpenVolume) / (m_LongPos.OpenVolume + other.OpenVolume);
				m_LongPos.PositionCost += other.PositionCost; // PositionCost = Price * Volume * VolumeMultiple (please refer Note.3)
				m_LongPos.OpenVolume += other.OpenVolume;
				m_LongPos.CloseVolume += other.CloseVolume;
				m_LongPos.OpenAmount += other.OpenAmount;
				m_LongPos.CloseAmount += other.CloseAmount;
				m_LongPos.UseMargin += other.UseMargin;
				m_LongPos.Commission += other.Commission;
				m_LongPos.PosiDirection = other.PosiDirection;
			}

			//##According debuging, I found YdPosition may not correct when tradespi callback
			m_LongPos.YdPosition = m_LongPos.Position - m_LongPos.TodayPosition;
		}
		else if (other.PosiDirection == THOST_FTDC_PD_Short)
		{
			if (IsShortPosEmpty()){
				memcpy(&m_ShortPos, &(other), sizeof(CThostFtdcInvestorPositionField));
			}
			else{
				m_ShortPos.Position += other.Position;
				m_ShortPos.TodayPosition += other.TodayPosition;
				m_ShortPos.ShortFrozen += other.ShortFrozen;
				m_ShortPos.FrozenMargin += other.FrozenMargin;
				m_ShortPos.FrozenCommission += other.FrozenCommission;
				m_ShortPos.ShortFrozenAmount += other.ShortFrozenAmount;
				m_ShortPos.PositionCost += other.PositionCost; // PositionCost = Price * Volume * VolumeMultiple (please refer Note.3)
				m_ShortPos.OpenVolume += other.OpenVolume;
				m_ShortPos.CloseVolume += other.CloseVolume;
				m_ShortPos.OpenAmount += other.OpenAmount;
				m_ShortPos.CloseAmount += other.CloseAmount;
				m_ShortPos.UseMargin += other.UseMargin;
				m_ShortPos.Commission += other.Commission;
				m_ShortPos.PosiDirection = other.PosiDirection;
			}

			//##According debuging, I found YdPosition may not correct when tradepi callback
			m_ShortPos.YdPosition = m_ShortPos.Position - m_ShortPos.TodayPosition;
		}
		else{
			//do nothing
		}

		return *this;
	}

	CThostFtdcInvestorPositionFieldWrapper& CThostFtdcInvestorPositionFieldWrapper::operator +=(const CThostFtdcTradeField& trade){
		int volumeMultiple = InstrumentManager.VolumeMultiple(trade.InstrumentID);

		auto initPosFieldFunc = [&](const CThostFtdcTradeField& tradeField, CThostFtdcInvestorPositionField& posField) -> void {
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
			posField.OpenAmount = tradeField.Price * tradeField.Volume * volumeMultiple; //开仓金额
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
			posField.PositionCost = tradeField.Price * volumeMultiple;
			posField.PreMargin = 0;
			posField.UseMargin += InstrumentManager.GetMargin(tradeField.InstrumentID, tradeField.Volume, tradeField.Price, tradeField.Direction);
			posField.Commission += InstrumentManager.GetCommission(tradeField.InstrumentID, tradeField.Volume, tradeField.Price, trade.OffsetFlag);
			
			strcpy(posField.TradingDay, tradeField.TradeDate);
		};

		auto appendPosFunc = [&](const CThostFtdcTradeField& tradeField, CThostFtdcInvestorPositionField& posField) -> void {
			double amount = posField.PositionCost;
			double delta_amount = tradeField.Volume * tradeField.Price * volumeMultiple;
			posField.PreMargin = posField.UseMargin;

			//update Position
			if (tradeField.OffsetFlag == THOST_FTDC_OF_Open )
			{
				posField.TodayPosition += tradeField.Volume;
				posField.Position = posField.TodayPosition + posField.YdPosition;
				posField.OpenVolume += tradeField.Volume; //更新开仓量
				posField.OpenAmount += delta_amount;
				posField.PositionCost += delta_amount;
			}
			else if (THOST_FTDC_OF_Close == tradeField.OffsetFlag || THOST_FTDC_OF_ForceClose == tradeField.OffsetFlag || 
				THOST_FTDC_OF_CloseToday == tradeField.OffsetFlag || THOST_FTDC_OF_CloseYesterday == tradeField.OffsetFlag){
				posField.TodayPosition -= tradeField.Volume;
				if (posField.TodayPosition < tradeField.Volume){
					posField.TodayPosition = 0;
					posField.YdPosition -= (tradeField.Volume - posField.TodayPosition);
				}
				else{
					posField.TodayPosition -= tradeField.Volume;
				}

				posField.Position = posField.TodayPosition + posField.YdPosition;
				posField.CloseVolume += tradeField.Volume; //更新平仓量
				posField.CloseAmount += delta_amount;
				posField.PositionCost -= delta_amount;
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
			double margin = InstrumentManager.GetMargin(orderField.InstrumentID, orderField.VolumeTotalOriginal, orderField.LimitPrice, orderField.Direction);

			switch(ordCBType){
				case OrderCallBackType::InsertOrder:
				{
					if (orderField.Direction == THOST_FTDC_D_Buy){
						m_LongPos.PosiDirection = THOST_FTDC_PD_Long;
						m_LongPos.LongFrozen += orderField.VolumeTotalOriginal;
						m_LongPos.FrozenMargin += margin;
						m_LongPos.FrozenCommission += commission;
					}
					else{ // THOST_FTDC_D_Sell
						m_ShortPos.PosiDirection = THOST_FTDC_PD_Short;
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

	double  CThostFtdcInvestorPositionFieldWrapper::GetLongMargin() const { 
		return m_LongPos.UseMargin; 
	}

	double  CThostFtdcInvestorPositionFieldWrapper::GetShortMargin() const { 
		return m_ShortPos.UseMargin; 
	}

	double  CThostFtdcInvestorPositionFieldWrapper::GetLongFrozenMargin() const{
		return m_LongPos.FrozenMargin;
	}

	double  CThostFtdcInvestorPositionFieldWrapper::GetShortFrozenMargin() const{
		return m_ShortPos.FrozenMargin;
	}

	double CThostFtdcInvestorPositionFieldWrapper::GetBiggerMargin() const{
		return std::max(m_LongPos.UseMargin, m_ShortPos.UseMargin);
	}

	double CThostFtdcInvestorPositionFieldWrapper::GetBiggerFrozenMargin() const{
		return std::max(m_LongPos.FrozenMargin, m_ShortPos.FrozenMargin);
	}

	double  CThostFtdcInvestorPositionFieldWrapper::GetBothMargin() const{
		return m_LongPos.UseMargin + m_ShortPos.UseMargin;
	}

	double  CThostFtdcInvestorPositionFieldWrapper::GetBothFrozenMargin() const{
		return m_LongPos.FrozenMargin + m_ShortPos.FrozenMargin;
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
		double aMoney = m_accountInfo.Available;
		return aMoney - GetUsedMargin() - GetFrozenMargin() - GetCommission() - GetFrozenCommission();
	}

	double PositionProfitMgr::GetBalanceMoney() const {
		double bMoney = m_accountInfo.Balance;
		return bMoney - GetCommission();
	}

	double PositionProfitMgr::GetFrozenMargin() const{
		double ret = 0.0;
		std::map<std::string, TwoSideMargin> pdMarginDict;
		for (auto item : m_posFieldMap){
			switch(InstrumentManager.GetExchangeID(item.first))
			{
				case THOST_FTDC_EIDT_SHFE:
				case THOST_FTDC_EIDT_CFFEX:
				{
					//one-way bigger frozenmargin
					const std::string& prodID = CommonUtils::InstrumentIDToProductID(item.first);
					pdMarginDict[prodID].Long += item.second.GetLongFrozenMargin();
					pdMarginDict[prodID].Short += item.second.GetShortFrozenMargin();
				}
				break;
				case THOST_FTDC_EIDT_CZCE: 
					ret += item.second.GetBiggerFrozenMargin();
					break;
				case THOST_FTDC_EIDT_DCE: //大连商品交易所 two-way margin
					ret += item.second.GetBothFrozenMargin();
					break;
				case THOST_FTDC_EIDT_INE:
				default:
					assert(false);
					break;
			}
		}	

		for (auto item : pdMarginDict){
			ret += item.second.GetBiggerOne();
		}			
		return ret;
	}

	double PositionProfitMgr::GetUsedMargin() const{
		double ret = 0.0;
		std::map<std::string, TwoSideMargin> pdMarginDict;
		for (auto item : m_posFieldMap){
			switch(InstrumentManager.GetExchangeID(item.first))
			{
				case THOST_FTDC_EIDT_SHFE:
				case THOST_FTDC_EIDT_CFFEX:
				{
					//one-way bigger margin
					const std::string& prodID = CommonUtils::InstrumentIDToProductID(item.first);
					pdMarginDict[prodID].Long += item.second.GetLongMargin();
					pdMarginDict[prodID].Short += item.second.GetShortMargin();
				}
				break;
				case THOST_FTDC_EIDT_CZCE: 
					ret += item.second.GetBiggerMargin();
					break;
				case THOST_FTDC_EIDT_DCE: //大连商品交易所 two-way margin
					ret += item.second.GetBothMargin();
					break;
				case THOST_FTDC_EIDT_INE:
				default:
					assert(false);
					break;
			}
		}		

		for (auto item : pdMarginDict){
			ret += item.second.GetBiggerOne();
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
		result.precision(10);
		result << "$AccountInfo => {" << std::endl;
		result << "Balance:" << GetBalanceMoney() << "," << std::endl;
		result << "Available:" << GetAvailableMoney() << "," << std::endl;
		result << "Margin:" << GetUsedMargin() << "," << std::endl;
		result << "FrozenMargin" << GetFrozenMargin() << "," << std::endl;
		result << "Commission" << GetCommission() << "," << std::endl;
		result << "FrozenCommission" << GetFrozenCommission() << "," << std::endl;

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
