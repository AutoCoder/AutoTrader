#ifndef POSITION_PROFIT_MGR_H
#define POSITION_PROFIT_MGR_H

#include <string.h>
#include <vector>
#include <map>
#include "stdafx.h"
#include "ThostFtdcUserApiStruct.h"
#include "TickWrapper.h"

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

	//Todo: Thread Synchronization
	struct CThostFtdcInvestorPositionFieldWrapper{
		explicit CThostFtdcInvestorPositionFieldWrapper();
		//the below 4 APIs will be called by tradeApi thread to update position 
		CThostFtdcInvestorPositionFieldWrapper& operator +=(const CThostFtdcInvestorPositionField& other);
		CThostFtdcInvestorPositionFieldWrapper& operator +=(const CThostFtdcTradeField& trade);
		void OnOrder(const CThostFtdcOrderField& order, OrderCallBackType type);
		void OnTick(const TickWrapper& newTick);

		// the below APIs will be called by mdSpi thread to generate order
		inline double  GetLongMargin() const;
		inline double  GetShortMargin() const;
		inline double  GetLongFrozenMargin() const;
		inline double  GetShortFrozenMargin() const;
		inline double  GetBiggerMargin() const;
		inline double  GetBiggerFrozenMargin() const;
		inline double  GetBothMargin() const;
		inline double  GetBothFrozenMargin() const;

		double 	GetCommission() const;
		double 	GetFrozenCommission() const;
		double  GetPositionProfit() const;

		int 	GetLongPos() const { return m_LongPos.Position; }
		int 	GetShortPos() const { return m_ShortPos.Position; }
		int 	GetYdLongPos() const { return m_LongPos.YdPosition; }
		int 	GetYdShortPos() const { return m_ShortPos.YdPosition; }
		std::string ToString();
	private:
		bool IsLongPosEmpty() const;
		bool IsShortPosEmpty() const;
		std::string InstrumentId() const; 
		

	private:
		TickWrapper                     m_LastTick;
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
		void UpdateLastTick(const TickWrapper& newTick);
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
		double GetPositionProfit() const;

		void   SetAccountInfoInitialized(bool init = true) { m_acccountInfoInitialized = init; };
		
		std::string ToString() const ;
		std::string PositionOfInstruments() const;
		const std::vector<CThostFtdcOrderField>& GetAllOrders() const { return m_orderFieldVec; };

	private:
		typedef std::map<std::string, CThostFtdcInvestorPositionFieldWrapper> PositionInfoMap; // key => InstrumentID

		bool                                            m_acccountInfoInitialized;
		CThostFtdcTradingAccountField					m_accountInfo;
		std::vector<CThostFtdcOrderField>				m_orderFieldVec;
		std::vector<CThostFtdcTradeField>				m_tradeFieldVec;
		PositionInfoMap									m_posFieldMap;
	};

}



#endif