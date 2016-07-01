#ifndef TRADE_SPI_H_
#define TRADE_SPI_H_

#include <mutex>
#include <atomic>
#include <condition_variable>
#include "crossplatform.h"
#include "ThostFtdcTraderApi.h"
#include "InstrumentInfoMgr.h"

class CtpTradeSpi : public CThostFtdcTraderSpi
{

	class TradeThreadStateChangedHandler{
		//Sleep(1000) is for avoiding query frequency limit, otherwise the queryRequest will return -3
	public:
		TradeThreadStateChangedHandler(CtpTradeSpi* pTradeUserSpi)
			:m_TradeUserSpiPtr(pTradeUserSpi)
		{
		}

		//step 1
		void OnFrontConnected(){
			m_TradeUserSpiPtr->ReqUserLogin();
		}

		//step 2
		void OnLogined(){
			m_TradeUserSpiPtr->ReqSettlementInfoConfirm();
		}

		//step 3 
		void OnConfirmedSettlementInfo(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInstrument_all();
		}

		//step 4
		void OnLastRspQryInstrument(){
			for(auto iter = InstrumentManager.begin(); iter != InstrumentManager.end(); ++iter){
				sleep(1000);
				m_TradeUserSpiPtr->ReqQryInstrumentMarginRate(iter->first);
				m_TradeUserSpiPtr->WaitQueryEnd();
			}

			for(auto iter = InstrumentManager.begin(); iter != InstrumentManager.end(); ++iter){
				sleep(1000);
				m_TradeUserSpiPtr->ReqQryInstrumentCommissionRate(iter->first);
				m_TradeUserSpiPtr->WaitQueryEnd();
			}

			SYNC_PRINT << "[Trade] Finish all margin & commission querying.";
		}

	private:
		CtpTradeSpi* m_TradeUserSpiPtr;
	};

public:
	CtpTradeSpi(CThostFtdcTraderApi* pUserApi,
		const char * brokerID, const char* userID, const char* password, const char* prodName);
        virtual	~CtpTradeSpi();

	void WaitQueryEnd(){
		std::unique_lock<std::mutex> lk(m_mtx);
		m_con.wait(lk, [this]() -> bool { return !m_querying.load(); });
	}

private:

	virtual void OnFrontConnected();

	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//响应查询合约保证金率
	void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///响应查询合约手续费率
	void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///响应查询期权交易成本
	void OnRspQryOptionInstrTradeCost(CThostFtdcOptionInstrTradeCostField *pOptionInstrTradeCost, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///响应查询期权合约手续费
	void OnRspQryOptionInstrCommRate(CThostFtdcOptionInstrCommRateField *pOptionInstrCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

private:
	void ReqUserLogin();

	void ReqSettlementInfoConfirm();

	void ReqQryInstrument_all();

	///请求查询合约保证金率
	void ReqQryInstrumentMarginRate(const std::string& instrumentId);

	///请求查询合约手续费率
	void ReqQryInstrumentCommissionRate(const std::string& instrumentId);

	///请求查询期权交易成本
	void ReqQryOptionInstrTradeCost(const std::string& instrumentId);

	///请求查询期权合约手续费
	void ReqQryOptionInstrCommRate(const std::string& instrumentId);

	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

	void NotifyQueryEnd(){
		m_querying.store(false);
		m_con.notify_all();
	}

private:
	TThostFtdcBrokerIDType								m_brokerID;
	TThostFtdcUserIDType								m_userID;
	TThostFtdcPasswordType								m_password;
	TThostFtdcProductInfoType							m_productName;
	TThostFtdcFrontIDType								m_frontID;
	TThostFtdcSessionIDType								m_sessionID;
	char												m_orderRef[13];
	CThostFtdcTradingAccountField						m_accountInfo;

private:
	std::mutex											m_mtx;
	std::condition_variable								m_con;
	std::atomic<bool>									m_querying;
	
	int              									m_requestId;
	CThostFtdcTraderApi*								pUserApi;
	TradeThreadStateChangedHandler						m_stateChangeHandler;
};

#endif 
