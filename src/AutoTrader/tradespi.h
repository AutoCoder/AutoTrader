#ifndef TRADE_SPI_H_
#define TRADE_SPI_H_

#include "ThostFtdcTraderApi.h"
//#include <future>
#include <mutex>
#include <condition_variable>
#include "crossplatform.h"
#include <functional>

class IPositionControl;
class Order;
namespace AP{
	class AccountDetailMgr;
};

extern std::condition_variable cv_md;

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
		void OnRspQryOrder(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryTrade();
		}

		//step 5 : get the trade list
		//请求查询成交响应,要区分程序启动时第一次查询跟之后的查询
		//并且只能查询当天的成交，昨仓不能，所以还要查询持仓明细OnRspQryInvestorPositionDetail()
		void OnRspQryTrade(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInvestorPositionDetail();
		}

		//step 6: query account
		void OnRspQryInvestorPositionDetail(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryTradingAccount();
		}

		//step 7:
		void OnRspQryTradingAccount(){
			//查询所有合约的持仓
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInvestorPosition_all();
		}

		//step 8
		void OnRspQryInvestorPosition(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInstrument_all();
		}

		//step 9
		void OnRspQryInstrument(){
			//invoke md thread
			//cv_md.notify_all();
			//send Account init finish action to fifo
		}

	private:
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
	~CtpTradeSpi();

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
	//excute order queue

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

	void ForceClose();

	//理解：在触发了新的交易之前，把已提交未成交的报单给撤销
	void CancelOrder(long long MDtime, int aliveDuration = 6, const std::string& instrumentId = "");

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

	bool m_firstquery_order;//是否首次查询报单
	bool m_firstquery_trade;//是否首次查询成交
	bool m_firstquery_Detail;//是否首次查询持仓明细
	bool m_firstquery_TradingAccount;//是否首次查询资金账号
	bool m_firstquery_Position;//是否首次查询投资者持仓
	bool m_firstquery_Instrument;//是否首次查询合约

private:
	InitedAccountCallback								m_initFinish_callback;
	RtnOrderCallback									m_OnRtnOrder_callback;
	RtnTradeCallback									m_OnRtnTrade_callback;
	CancelOrderCallback									m_OnCancelOrder_callback;
	AP::AccountDetailMgr&								m_account_detail_mgr;
	CThostFtdcTraderApi*								pUserApi;
	TradeThreadStateChangedHandler						m_stateChangeHandler;
};

#endif 