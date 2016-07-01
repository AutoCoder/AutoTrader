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

			m_TradeUserSpiPtr->NotifyQueryFinished();
		}

	private:
		CtpTradeSpi* m_TradeUserSpiPtr;
	};

public:
	CtpTradeSpi(CThostFtdcTraderApi* pUserApi,
		const char * brokerID, const char* userID, const char* password, const char* prodName);
        virtual	~CtpTradeSpi();

	void WaitQueryResponsed(){
		std::unique_lock<std::mutex> lk(m_mtx1);
		m_con1.wait(lk, [this]() -> bool { return !m_hanging.load(); });
	}

	void WaitQueryFinshed(){
		std::unique_lock<std::mutex> lk(m_mtx2);
		m_con2.wait(lk, [this]() -> bool { return m_isfinished.load(); });
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

public:
	///请求查询合约保证金率
	void ReqQryInstrumentMarginRate(const std::string& instrumentId);

	///请求查询合约手续费率
	void ReqQryInstrumentCommissionRate(const std::string& instrumentId);

	///请求查询期权交易成本
	void ReqQryOptionInstrTradeCost(const std::string& instrumentId);

	///请求查询期权合约手续费
	void ReqQryOptionInstrCommRate(const std::string& instrumentId);

	void NotifyQueryResponse(){
		m_hanging.store(false);
		m_con1.notify_all();
	}
private:
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

	void NotifyQueryFinished(){
		m_isfinished.store(true);
		m_con2.notify_all();
	}

private:
	TThostFtdcBrokerIDType								m_brokerID;
	TThostFtdcUserIDType								m_userID;
	TThostFtdcPasswordType								m_password;
	TThostFtdcProductInfoType							m_productName;
	TThostFtdcFrontIDType								m_frontID;
	TThostFtdcSessionIDType								m_sessionID;
	char												m_orderRef[13];

private:
	std::mutex											m_mtx1;
	std::mutex											m_mtx2;
	std::condition_variable								m_con1;
	std::condition_variable								m_con2;
	std::atomic<bool>									m_hanging;
	std::atomic<bool>									m_isfinished;
	
	int              									m_requestId;
	TradeThreadStateChangedHandler						m_stateChangeHandler;
	CThostFtdcTraderApi*								pUserApi;
};

#endif 
