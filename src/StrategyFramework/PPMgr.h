#ifndef POSITION_PROFIT_MGR_H
#define POSITION_PROFIT_MGR_H

#include <string.h>
#include <vector>
#include <map>
#include "stdafx.h"
#include "ThostFtdcUserApiStruct.h"


namespace PP {

	struct IsSameTrade{

		IsSameTrade(const CThostFtdcTradeField& input)
		{
			memcpy(&m_data, &input, sizeof(input));
		}

		bool operator() (const CThostFtdcTradeField& input){
			return strcmp(m_data.TradeID, input.TradeID) == 0;
		}

	private:
		CThostFtdcTradeField m_data;
	};

	template <typename T>
	struct IsSameOrder{

		IsSameOrder(const T& input)
		{
			memcpy(&m_data, &input, sizeof(T));
		}

		bool operator() (const T& input){
			return m_data.BrokerOrderSeq == input.BrokerOrderSeq;
		}

	private:
		T m_data; // CThostFtdcOrderField or CThostFtdcTradeField
	};

	enum class OrderCallBackType{
		Unknown,
		InsertOrder,
		FinishOrder,
		CancellOrder,
		Other,
	};

	struct CThostFtdcInvestorPositionFieldWrapper{
		explicit CThostFtdcInvestorPositionFieldWrapper();
		CThostFtdcInvestorPositionFieldWrapper& operator +=(const CThostFtdcInvestorPositionField& other);
		CThostFtdcInvestorPositionFieldWrapper& operator +=(const CThostFtdcTradeField& trade);
		void OnOrder(const CThostFtdcOrderField& order, OrderCallBackType type);

		double 	GetMargin() const;
		double 	GetFrozenMargin() const;
		double 	GetCommission() const;
		double 	GetFrozenCommission() const;

		int 	GetLongPos() const { return m_LongPos.Position; }
		int 	GetShortPos() const { return m_ShortPos.Position; }
		std::string ToString();
	private:
		bool IsLongPosEmpty();
		bool IsShortPosEmpty();
		

	private:
		CThostFtdcInvestorPositionField m_LongPos;
		CThostFtdcInvestorPositionField m_ShortPos;
	};

	class STRATEGY_API PositionProfitMgr
	{
	public:
		PositionProfitMgr();
		~PositionProfitMgr();

		bool PushOrder(const CThostFtdcOrderField& orderField);
		void PushTrade(const CThostFtdcTradeField& tradeField);

		void SetAccountInfo(const CThostFtdcTradingAccountField& info);
		void PushInvestorPosition(const CThostFtdcInvestorPositionField& posInfo);
		void PushInvestorPositionDetail(const CThostFtdcInvestorPositionDetailField& posDetail);

		/*
		*  ///Âò
		*	#define THOST_FTDC_D_Buy '0'
		*	///Âô
		*	#define THOST_FTDC_D_Sell '1'
		*/
		size_t GetUnclosedPosition(const std::string& instrumentId, TThostFtdcDirectionType type) const ;
		size_t GetYDUnclosedPosition(const std::string& instrumentId, TThostFtdcDirectionType type) const;
		double GetAvailableMoney() const ;
		double GetBalanceMoney() const;
		double GetFrozenMargin() const;
		double GetUsedMargin() const;
		double GetCommission() const;
		double GetFrozenCommission() const;

		void   SetAccountInfoInitialized(bool init = true) { m_acccountInfoInitialized = init; };

		std::string ToString() const ;
		std::string PositionOfInstruments() const;
		const std::vector<CThostFtdcOrderField>& GetAllOrders() const { return m_orderFieldVec; };
	private:
		CThostFtdcInvestorPositionField ToPositionInfo(const CThostFtdcTradeField& newestTrade);
		

	private:
		typedef std::map<std::string, CThostFtdcInvestorPositionFieldWrapper> PositionInfoMap; // key => InstrumentID

		bool                                            m_acccountInfoInitialized;
		CThostFtdcTradingAccountField					m_accountInfo;
		double                                          m_fronzonMargin;      //total
		double                                          m_fronzonCommission;  //total 
		std::vector<CThostFtdcOrderField>				m_orderFieldVec;
		std::vector<CThostFtdcTradeField>				m_tradeFieldVec;
		PositionInfoMap									m_posFieldMap;
	};

}



#endif