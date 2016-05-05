#ifndef TRADE_SPI_H_
#define TRADE_SPI_H_

#include "ThostFtdcTraderApi.h"
#include <mutex>
#include <condition_variable>
#include "crossplatform.h"
#include <functional>
#include "AccountMgr.h"

class IPositionControl;
class Order;
namespace AP{
	class AccountDetailMgr;
};

typedef std::function<void()>															InitedAccountCallback;
typedef std::function<void(CThostFtdcOrderField*)>										RtnOrderCallback;
typedef std::function<void(CThostFtdcTradeField*)>										RtnTradeCallback;
typedef std::function<void(CThostFtdcInputOrderActionField*, CThostFtdcRspInfoField*)>	CancelOrderCallback;

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
			m_TradeUserSpiPtr->ReqQryOrder();
		}

		//step 4 : get the order list 
		void OnLastRspQryOrder(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryTrade();
		}

		//step 5 : get the trade list
		//请求查询成交响应,要区分程序启动时第一次查询跟之后的查询
		//并且只能查询当天的成交，昨仓不能，所以还要查询持仓明细OnRspQryInvestorPositionDetail()
		void OnLastRspQryTrade(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInvestorPositionDetail();
		}

		//step 6: query account
		void OnLastRspQryInvestorPositionDetail(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryTradingAccount();
		}

		//step 7:
		void OnLastRspQryTradingAccount(bool successful = true){
			//查询所有合约的持仓
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInvestorPosition_all();
		}

		//step 8
		void OnLastRspQryInvestorPosition(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInstrument_all();
		}

		//step 9
		void OnLastRspQryInstrument(bool successful = true){
			//Todo: loop all related Instrument, query their marginrate CommissionRate
			for (auto instru : Account::Manager::Instance().Instruments()){
				sleep(1000);
				m_querying.store(true);
				m_TradeUserSpiPtr->ReqQryInstrumentMarginRate(instru.c_str());
				WaitQueryEnd();

				sleep(1000);
				m_querying.store(true);
				m_TradeUserSpiPtr->ReqQryInstrumentCommissionRate(instru.c_str());
				WaitQueryEnd();
			}

			m_TradeUserSpiPtr->InitializationFinished();
		}

	public:
		void WaitQueryEnd(){
			std::unique_lock<std::mutex> lk(m_mtx);
			m_con.wait(lk, [this]() -> bool { return !m_querying.load(); });
		}
		
		void NotifyQueryEnd(){
			m_querying.store(false);
			m_con.notify_all();
		}

	private:
		std::condition_variable m_con;
		std::atomic<bool>       m_querying;
		std::mutex              m_mtx;
		CtpTradeSpi* m_TradeUserSpiPtr;
	};

public:
	CtpTradeSpi(CThostFtdcTraderApi* pUserApi,
		const char * brokerID, const char* userID, const char* password, const char* prodName, 
		AP::AccountDetailMgr& admgr, 
		InitedAccountCallback initFinishCallback,
		RtnOrderCallback onRtnOrderCallback,
		RtnTradeCallback onRtnTradeCallback,
		CancelOrderCallback OnRtnCancellOrderCallback);
        virtual	~CtpTradeSpi();

	virtual void OnFrontConnected();

	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询报单响应
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询成交响应
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者持仓明细响应
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnFrontDisconnected(int nReason);

	virtual void OnHeartBeatWarning(int nTimeLapse);

	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

	//响应查询合约保证金率
	void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///响应查询合约手续费率
	void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///响应查询期权交易成本
	void OnRspQryOptionInstrTradeCost(CThostFtdcOptionInstrTradeCostField *pOptionInstrTradeCost, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///响应查询期权合约手续费
	void OnRspQryOptionInstrCommRate(CThostFtdcOptionInstrCommRateField *pOptionInstrCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

public:
	void ReqUserLogin();

	void ReqSettlementInfoConfirm();

	void ReqQryOrder();

	void ReqQryTrade();

	void ReqQryInvestorPositionDetail();

	void ReqQryInstrument(TThostFtdcInstrumentIDType instId);

	void ReqQryTradingAccount();

	void ReqQryInvestorPosition_all();

	void ReqQryInvestorPosition();

	void ReqQryInstrument_all();

	void ReqOrderInsert(Order ord);

	void ReqOrderAction(const CThostFtdcOrderField& order);// TThostFtdcSequenceNoType orderSeq, TThostFtdcExchangeIDType exchangeId, TThostFtdcOrderSysIDType orderSysId);

	///请求查询合约保证金率
	void ReqQryInstrumentMarginRate(const char* instId);

	///请求查询合约手续费率
	void ReqQryInstrumentCommissionRate(const char* instId);

	///请求查询期权交易成本
	void ReqQryOptionInstrTradeCost(const char* instId);

	///请求查询期权合约手续费
	void ReqQryOptionInstrCommRate(const char* instId);

	void ForceClose();

	//理解：在触发了新的交易之前，把已提交未成交的报单给撤销
	void CancelOrder(long long MDtime, int aliveDuration = 6, const std::string& instrumentId = "");

	void InitializationFinished();

	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

private:
	TThostFtdcBrokerIDType m_brokerID;
	TThostFtdcUserIDType m_userID;
	TThostFtdcPasswordType m_password;
	TThostFtdcProductInfoType m_productName;
	TThostFtdcFrontIDType m_frontID;
	TThostFtdcSessionIDType m_sessionID;
	char m_orderRef[13];
	CThostFtdcTradingAccountField m_accountInfo;

	//bool m_firstquery_order;//是否首次查询报单
	//bool m_firstquery_trade;//是否首次查询成交
	//bool m_firstquery_Detail;//是否首次查询持仓明细
	//bool m_firstquery_TradingAccount;//是否首次查询资金账号
	//bool m_firstquery_Position;//是否首次查询投资者持仓
	//bool m_firstquery_Instrument;//是否首次查询合约

private:
	InitedAccountCallback								m_initFinish_callback;
	RtnOrderCallback									m_OnRtnOrder_callback;
	RtnTradeCallback									m_OnRtnTrade_callback;
	CancelOrderCallback									m_OnCancelOrder_callback;
	AP::AccountDetailMgr&								m_account_detail_mgr;
	CThostFtdcTraderApi*								pUserApi;
	TradeThreadStateChangedHandler						m_stateChangeHandler;
	int              									m_requestId;
};

#endif 
