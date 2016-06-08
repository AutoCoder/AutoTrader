#include <assert.h>
#include "PPMgr.h"
#include "InstrumentInfoMgr.h"

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

	CThostFtdcInvestorPositionFieldWrapper& CThostFtdcInvestorPositionFieldWrapper::operator +=(const CThostFtdcInvestorPositionField& other){
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
		}
	}

	PositionProfitMgr::PositionProfitMgr()
		:m_acccountInfoInitialized(false)
	{
		memset(&m_accountInfo, 0, sizeof(CThostFtdcTradingAccountField));
	}


	PositionProfitMgr::~PositionProfitMgr()
	{
	}


	void PositionProfitMgr::PushOrder(const CThostFtdcOrderField& orderField){
		bool ForzonUpdateNeeded = false;
		bool IsfinishedorCancelled = (orderField.OrderStatus == THOST_FTDC_OST_AllTraded || orderField.OrderStatus == THOST_FTDC_OST_Canceled);
		auto iter = std::find(m_orderFieldVec.begin(), m_orderFieldVec.end(), IsSameOrder<CThostFtdcOrderField>(orderField));
		if (iter == m_orderFieldVec.end()){
			m_orderFieldVec.push_back(orderField);
			//that mean OrderStatus == THOST_FTDC_OST_NoTradeQueueing
			ForzonUpdateNeeded = true;

		}
		else{
			//if a order is finished, it will be earsed in the list
			if (IsfinishedorCancelled){
				m_orderFieldVec.erase(iter);
				ForzonUpdateNeeded = true;
			}
			else{
				ForzonUpdateNeeded = false;
			}
		}

		if (ForzonUpdateNeeded){
			double commratio = 1.0;
			if (orderField.CombOffsetFlag[0] == THOST_FTDC_OF_Open)
				commratio = InstrumentManager.Get(orderField.InstrumentID).ComRateField.OpenRatioByVolume;
			else if (orderField.CombOffsetFlag[0] == THOST_FTDC_OF_Close)
				commratio = InstrumentManager.Get(orderField.InstrumentID).ComRateField.CloseRatioByVolume;
			else if (orderField.CombOffsetFlag[0] == THOST_FTDC_OF_Close)
				commratio = InstrumentManager.Get(orderField.InstrumentID).ComRateField.CloseTodayRatioByVolume;
			else {
				assert(false);
			}
			double commission = commratio * orderField.VolumeTotalOriginal;
			m_accountInfo.FrozenCommission += commission * (IsfinishedorCancelled ? -1 : 1);
			m_accountInfo.Available += commission * (IsfinishedorCancelled ? 1 : -1);

			double margin = 0.0;
			if (orderField.Direction == THOST_FTDC_D_Buy){
				margin = InstrumentManager.Get(orderField.InstrumentID).MgrRateField.LongMarginRatioByVolume * orderField.VolumeTotalOriginal;
				m_accountInfo.FrozenMargin += (margin * (IsfinishedorCancelled ? -1 : 1));
			}
			else if (orderField.Direction == THOST_FTDC_D_Sell){
				margin = InstrumentManager.Get(orderField.InstrumentID).MgrRateField.ShortMarginRatioByVolume * orderField.VolumeTotalOriginal;
				m_accountInfo.FrozenMargin += (margin * (IsfinishedorCancelled ? -1 : 1));
			}
			m_accountInfo.Available += (margin * (IsfinishedorCancelled ? 1 : -1));
		}
	}


	void PositionProfitMgr::PushTrade(const CThostFtdcTradeField& tradeField){

		if (m_acccountInfoInitialized){
			CThostFtdcInvestorPositionField newPos = ToPositionInfo(tradeField);
			PushInvestorPosition(newPos);

			//update m_accountInfo
			for (auto instr : m_posFieldMap){

			}
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
		///��
		#define THOST_FTDC_PD_Net '1'
		///��ͷ
		#define THOST_FTDC_PD_Long '2'
		///��ͷ
		#define THOST_FTDC_PD_Short '3'
		*/

		/*  Direction
		///��
		#define THOST_FTDC_D_Buy '0'
		///��
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
				///����
		#define THOST_FTDC_OF_Open '0'
				///ƽ��
		#define THOST_FTDC_OF_Close '1'
				///ǿƽ
		#define THOST_FTDC_OF_ForceClose '2'
				///ƽ��
		#define THOST_FTDC_OF_CloseToday '3'
				///ƽ��
		#define THOST_FTDC_OF_CloseYesterday '4'
				///ǿ��
		#define THOST_FTDC_OF_ForceOff '5'
				///����ǿƽ
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
	}

	void PositionProfitMgr::SetAccountInfo(const CThostFtdcTradingAccountField& info){
		memcpy(&m_accountInfo, &info, sizeof(CThostFtdcTradingAccountField));
	}

	void PositionProfitMgr::PushInvestorPosition(const CThostFtdcInvestorPositionField& posInfo){
		m_posFieldMap[posInfo.InstrumentID] += posInfo;
	}

	void PositionProfitMgr::PushInvestorPositionDetail(const CThostFtdcInvestorPositionDetailField& posDetail){

	}


	size_t PositionProfitMgr::GetUnclosedPosition(const std::string& instrumentId, TThostFtdcDirectionType type){
		if (m_posFieldMap.find(instrumentId) != m_posFieldMap.end()){
			if (type == THOST_FTDC_D_Buy){
				return m_posFieldMap[instrumentId].GetLongPos().Position;
			}
			else if (type == THOST_FTDC_D_Sell){
				return m_posFieldMap[instrumentId].GetShortPos().Position;
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

	size_t PositionProfitMgr::GetYDUnclosedPosition(const std::string& instrumentId, TThostFtdcDirectionType type){
		if (m_posFieldMap.find(instrumentId) != m_posFieldMap.end()){
			if (type == THOST_FTDC_D_Buy){
				return m_posFieldMap[instrumentId].GetLongPos().YdPosition;
			}
			else if (type == THOST_FTDC_D_Sell){
				return m_posFieldMap[instrumentId].GetShortPos().YdPosition;
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

	double PositionProfitMgr::GetAvailableMoney(){
		return m_accountInfo.Available;
	}

	double PositionProfitMgr::GetFrozenCommission(){
		return m_accountInfo.FrozenCommission;
	}

	double PositionProfitMgr::GetUsedMargin(){
		return m_accountInfo.CurrMargin;
	}
}