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
		return m_LongPos.PosiDirection == THOST_FTDC_PD_Net;
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
			if (IsLongPosEmpty()){
				memcpy(&m_LongPos, &(other), sizeof(CThostFtdcInvestorPositionField));
			}
			else{
				m_LongPos.LongFrozen += other.LongFrozen;
				m_LongPos.LongFrozenAmount += other.LongFrozenAmount;
				m_LongPos.PositionCost = (m_LongPos.PositionCost * m_LongPos.OpenVolume + other.PositionCost * other.OpenVolume) / (m_LongPos.OpenVolume + other.OpenVolume);
				m_LongPos.OpenVolume += other.OpenVolume;
				m_LongPos.CloseVolume += other.CloseVolume;
				m_LongPos.OpenAmount += other.OpenAmount;
				m_LongPos.CloseAmount += other.CloseAmount;
				m_LongPos.UseMargin += other.UseMargin;
				m_LongPos.Commission += other.Commission;
			}
		}
		else if (other.PosiDirection == THOST_FTDC_PD_Short)
		{
			if (IsShortPosEmpty()){
				memcpy(&m_ShortPos, &(other), sizeof(CThostFtdcInvestorPositionField));
			}
			else{
				m_ShortPos.LongFrozen += other.LongFrozen;
				m_ShortPos.LongFrozenAmount += other.LongFrozenAmount;
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

	PositionProfitMgr::PositionProfitMgr()
		:m_acccountInfoInitialized(false)
	{
		memset(&m_accountInfo, 0, sizeof(CThostFtdcTradingAccountField));
	}


	PositionProfitMgr::~PositionProfitMgr()
	{
	}


	bool PositionProfitMgr::PushOrder(const CThostFtdcOrderField& orderField){
		bool IsInsertOrder = false;
		bool IsfinishOrder = orderField.OrderStatus == THOST_FTDC_OST_AllTraded;
		bool IsCancelled = orderField.OrderStatus == THOST_FTDC_OST_Canceled;
		auto iter = std::find_if(m_orderFieldVec.begin(), m_orderFieldVec.end(), IsSameOrder<CThostFtdcOrderField>(orderField));
		if (iter == m_orderFieldVec.end()){
			m_orderFieldVec.push_back(orderField);
			//that mean OrderStatus == THOST_FTDC_OST_NoTradeQueueing
			IsInsertOrder = true;

		}
		else{
			//if a order is finished, it will be earsed in the list
			if (IsfinishOrder || IsCancelled){
				m_orderFieldVec.erase(iter);
			}
		}

		bool PositionStatusChanged = IsInsertOrder || IsfinishOrder || IsCancelled;


		if (PositionStatusChanged){
			double commratio = 1.0;
			if (orderField.CombOffsetFlag[0] == THOST_FTDC_OF_Open)
				commratio = InstrumentManager.Get(orderField.InstrumentID).ComRateField.OpenRatioByVolume;
			else if (THOST_FTDC_OF_Close == orderField.CombOffsetFlag[0] && THOST_FTDC_OF_CloseYesterday == orderField.CombOffsetFlag[0])
				commratio = InstrumentManager.Get(orderField.InstrumentID).ComRateField.CloseRatioByVolume;
			else if (orderField.CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday)
				commratio = InstrumentManager.Get(orderField.InstrumentID).ComRateField.CloseTodayRatioByVolume;
			else {
				assert(false);
			}
			double commission = commratio * orderField.VolumeTotalOriginal;

			double margin = 0.0;
			if (orderField.Direction == THOST_FTDC_D_Buy){
				margin = InstrumentManager.Get(orderField.InstrumentID).MgrRateField.LongMarginRatioByVolume * orderField.VolumeTotalOriginal;
			}
			else if (orderField.Direction == THOST_FTDC_D_Sell){
				margin = InstrumentManager.Get(orderField.InstrumentID).MgrRateField.ShortMarginRatioByVolume * orderField.VolumeTotalOriginal;
			}

			if (IsInsertOrder)
			{
				m_accountInfo.FrozenCommission += commission;
				m_accountInfo.FrozenMargin += margin;
				m_accountInfo.Available -= commission;
				m_accountInfo.Available -= margin;
			}
			else if (IsfinishOrder){
				m_accountInfo.FrozenCommission -= commission;
				m_accountInfo.FrozenMargin -= margin;
				m_accountInfo.Commission += commission;
				m_accountInfo.CurrMargin += margin;
			}
			else if (IsCancelled){
				m_accountInfo.FrozenCommission -= commission;
				m_accountInfo.FrozenMargin -= margin;
				m_accountInfo.Available += commission;
				m_accountInfo.Available += margin;
			}
		}
		
		return IsInsertOrder;
	}


	void PositionProfitMgr::PushTrade(const CThostFtdcTradeField& tradeField){

		if (m_acccountInfoInitialized){
			CThostFtdcInvestorPositionField newPos = ToPositionInfo(tradeField);
			PushInvestorPosition(newPos);
		}
	}

	CThostFtdcInvestorPositionField PositionProfitMgr::ToPositionInfo(const CThostFtdcTradeField& tradeField)
	{
		CThostFtdcInvestorPositionField newPosInfo;
		memset(&newPosInfo, 0, sizeof(CThostFtdcInvestorPositionField));

		memcpy(newPosInfo.BrokerID, tradeField.BrokerID, sizeof(newPosInfo.BrokerID));
		memcpy(newPosInfo.InvestorID, tradeField.InvestorID, sizeof(newPosInfo.InvestorID));
		memcpy(newPosInfo.InstrumentID, tradeField.InstrumentID, sizeof(newPosInfo.InstrumentID));
		newPosInfo.HedgeFlag = tradeField.HedgeFlag;
		newPosInfo.TodayPosition = tradeField.Volume;

		/* PosiDirection
		///净
		#define THOST_FTDC_PD_Net '1'
		///多头
		#define THOST_FTDC_PD_Long '2'
		///空头
		#define THOST_FTDC_PD_Short '3'
		*/

		/*  Direction
		///买
		#define THOST_FTDC_D_Buy '0'
		///卖
		#define THOST_FTDC_D_Sell '1'
		*/
		newPosInfo.PosiDirection = tradeField.Direction + 2;
		newPosInfo.YdPosition = 0;
		newPosInfo.Position = newPosInfo.TodayPosition + newPosInfo.YdPosition;
		newPosInfo.PositionCost = tradeField.Price;

		//double exchangeMoney = tradeField.Price * tradeField.Volume;

		if (tradeField.Direction == THOST_FTDC_D_Buy){
			newPosInfo.UseMargin = InstrumentManager.Get(tradeField.InstrumentID).MgrRateField.LongMarginRatioByVolume * tradeField.Volume;
		}
		else if (tradeField.Direction == THOST_FTDC_D_Sell){
			newPosInfo.UseMargin = InstrumentManager.Get(tradeField.InstrumentID).MgrRateField.ShortMarginRatioByVolume * tradeField.Volume;
		}

		/*
				///开仓
		#define THOST_FTDC_OF_Open '0'
				///平仓
		#define THOST_FTDC_OF_Close '1'
				///强平
		#define THOST_FTDC_OF_ForceClose '2'
				///平今
		#define THOST_FTDC_OF_CloseToday '3'
				///平昨
		#define THOST_FTDC_OF_CloseYesterday '4'
				///强减
		#define THOST_FTDC_OF_ForceOff '5'
				///本地强平
		#define THOST_FTDC_OF_LocalForceClose '6'
		*/
		if (tradeField.OffsetFlag == THOST_FTDC_OF_Open){
			newPosInfo.Commission = InstrumentManager.Get(tradeField.InstrumentID).ComRateField.OpenRatioByVolume * tradeField.Volume;
		}
		else if (tradeField.OffsetFlag == THOST_FTDC_OF_Close)
		{
			newPosInfo.Commission = InstrumentManager.Get(tradeField.InstrumentID).ComRateField.CloseRatioByVolume * tradeField.Volume;
		}
		else if (tradeField.OffsetFlag == THOST_FTDC_OF_CloseToday)
		{
			newPosInfo.Commission = InstrumentManager.Get(tradeField.InstrumentID).ComRateField.CloseTodayRatioByVolume * tradeField.Volume;
		}
		else{
			assert(false);
		}

		return newPosInfo;
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
				return m_posFieldMap.at(instrumentId).GetLongPos().Position;
			}
			else if (type == THOST_FTDC_D_Sell){
				return m_posFieldMap.at(instrumentId).GetShortPos().Position;
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
				return m_posFieldMap.at(instrumentId).GetLongPos().YdPosition;
			}
			else if (type == THOST_FTDC_D_Sell){
				return m_posFieldMap.at(instrumentId).GetShortPos().YdPosition;
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
		return m_accountInfo.Available;
	}

	double PositionProfitMgr::GetBalanceMoney() const {
		return m_accountInfo.Balance;
	}

	double PositionProfitMgr::GetFrozenCommission() const{
		return m_accountInfo.FrozenCommission;
	}

	double PositionProfitMgr::GetUsedMargin() const{
		return m_accountInfo.CurrMargin;
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
		
		return result.str();
	}

	std::string PositionProfitMgr::PositionOfInstruments() const{
		std::stringstream result;
		for (auto item : m_posFieldMap){
			result << item.first << " Long:" << item.second.GetLongPos().Position << " Short:" << item.second.GetShortPos().Position << "\n";
		}
		return result.str();
	}
}
