#include "stdafx.h"
#include "crossplatform.h"
#include "tradespi.h"
#include "OrderQueue.h"
#include "Order.h"
#include "ThostFtdcTraderApi.h"
#include "traderspi.h"
#include "config.h"
#include <condition_variable>
#include "spdlog/spdlog.h"
#include "CommonUtils.h"
#include "IAccount.h"
extern int requestId;

extern std::condition_variable cv_trade;

std::vector<CThostFtdcOrderField*> orderList;
std::vector<CThostFtdcTradeField*> tradeList;

//char MapDirection(char src, bool toOrig);
//char MapOffset(char src, bool toOrig);

CtpTradeSpi::CtpTradeSpi(CThostFtdcTraderApi* p, const char * brokerID, const char* userID, const char* password, const char* prodname)
	: pUserApi(p)
	, pAccountMgr(nullptr)
	, m_frontID(-1)
	, m_sessionID(-1)
	, m_isFrontConnected(false)
	, m_islogin(false)
	, m_isConfirmSettlementInfo(false)
	, m_isAccountFreshed(false)
	
{
	strcpy_s(m_brokerID, brokerID);
	strcpy_s(m_userID, userID);
	strcpy_s(m_password, password);
	strcpy_s(m_productName, prodname);
}

CtpTradeSpi::~CtpTradeSpi()
{
}

void CtpTradeSpi::OnFrontConnected()
{
	spdlog::get("console")->info() << "[Trade Thread] Response | connected...";
	//SetEvent(g_tradehEvent);
	m_isFrontConnected = true;
	cv_trade.notify_all();
}

void CtpTradeSpi::OnFrontDisconnected(int nReason)
{
	spdlog::get("console")->info() << "[Trade Thread] Response | Disconnected..."
		<< " reason=" << nReason;

	m_isFrontConnected = false;
	m_islogin = false;
}

void CtpTradeSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.UserID, m_userID);
	strcpy_s(req.Password, m_password);
	strcpy_s(req.UserProductInfo, m_productName);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | send logging ..." << ((ret == 0) ? "success" : "fail");
}

void CtpTradeSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {

		m_frontID = pRspUserLogin->FrontID;
		m_sessionID = pRspUserLogin->SessionID;
		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		sprintf_s(m_orderRef, "%d", ++nextOrderRef);
		spdlog::get("console")->info() << "[Trade Thread] Response | login successfully...CurrentDate:"
			<< pRspUserLogin->TradingDay;
		m_islogin = true;
		cv_trade.notify_all();

	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | sending settlementInfo confirmation..." << ((ret == 0) ? "success" : "fail");

}

void CtpTradeSpi::OnRspSettlementInfoConfirm(
	CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
		spdlog::get("console")->info() << "[Trade Thread] Response | settlementInfo..." << pSettlementInfoConfirm->InvestorID
			<< "...<" << pSettlementInfoConfirm->ConfirmDate
			<< " " << pSettlementInfoConfirm->ConfirmTime << ">...Confirm";

		m_isConfirmSettlementInfo = true;
		cv_trade.notify_all();
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInstrument(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | send Instrument Query..." << ((ret == 0) ? "success" : "fail");
}

void CtpTradeSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInstrument){
		spdlog::get("console")->info() << "[Trade Thread] Response | Instrument:" << pInstrument->InstrumentID
			<< " DeliveryMonth:" << pInstrument->DeliveryMonth
			<< " LongMarginRatio:" << pInstrument->LongMarginRatio
			<< " ShortMarginRatio:" << pInstrument->ShortMarginRatio;
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	int ret = pUserApi->ReqQryTradingAccount(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | send trading account query..." << ((ret == 0) ? "success" : "fail");

}

void CtpTradeSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pTradingAccount){
		if (pAccountMgr)
			pAccountMgr->update(*pTradingAccount);
		memcpy(&m_accountInfo, pTradingAccount, sizeof(CThostFtdcTradingAccountField));
		spdlog::get("console")->info() << "[Trade Thread] Response | Balance:" << pTradingAccount->Balance
			<< " Available:" << pTradingAccount->Available
			<< " CurrMargin:" << pTradingAccount->CurrMargin
			<< " CloseProfit:" << pTradingAccount->CloseProfit
			<< " PositionProfit:" << pTradingAccount->PositionProfit
			<< " Commission:" << pTradingAccount->Commission
			<< " FrozenMargin:" << pTradingAccount->FrozenMargin;
		m_isAccountFreshed = true;
	}

	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	strcpy_s(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInvestorPosition(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | send InvestorPosition query..." << ((ret == 0) ? "success" : "fail");
}

void CtpTradeSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField *pInvestorPosition,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition){
		spdlog::get("console")->info() << "[Trade Thread] Response| Instrument:" << pInvestorPosition->InstrumentID
			<< " PosiDirection:" << pInvestorPosition->PosiDirection
			<< " Position:" << pInvestorPosition->Position
			<< " Yesterday Position:" << pInvestorPosition->YdPosition
			<< " Today Position:" << pInvestorPosition->TodayPosition
			<< " Position Profit:" << pInvestorPosition->PositionProfit
			<< " UseMargin:" << pInvestorPosition->UseMargin;
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrder){
		spdlog::get("console")->info() << "[Back Order]" << CommonUtils::StringFromStruct(*pInputOrder);
		spdlog::get("console")->info() << "[Trade Thread] Response | Insert order success...Order Reference:" << pInputOrder->OrderRef;
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::ReqOrderAction(TThostFtdcSequenceNoType orderSeq)
{
	bool found = false; unsigned int i = 0;
	for (i = 0; i<orderList.size(); i++){
		if (orderList[i]->BrokerOrderSeq == orderSeq){ found = true; break; }
	}
	if (!found){ spdlog::get("console")->info() << "[Trade Thread] Request | Insert order doesn't exist."; return; }

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	//strcpy_s(req.OrderRef, pOrderRef); 
	//req.FrontID = frontId;           
	//req.SessionID = sessionId;       
	strcpy_s(req.ExchangeID, orderList[i]->ExchangeID);
	strcpy_s(req.OrderSysID, orderList[i]->OrderSysID);
	req.ActionFlag = THOST_FTDC_AF_Delete;

	int ret = pUserApi->ReqOrderAction(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | backout order..." << ((ret == 0) ? "success" : "fail");
}

void CtpTradeSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField *pInputOrderAction,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction){
		spdlog::get("console")->info() << "[Trade Thread] Response | backout order success..."
			<< "Exchange ID:" << pInputOrderAction->ExchangeID
			<< " Order System ID:" << pInputOrderAction->OrderSysID;
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	CThostFtdcOrderField* order = new CThostFtdcOrderField();
	memcpy(order, pOrder, sizeof(CThostFtdcOrderField));
	bool founded = false;    unsigned int i = 0;
	for (i = 0; i<orderList.size(); i++){
		if (orderList[i]->BrokerOrderSeq == order->BrokerOrderSeq) {
			founded = true;    break;
		}
	}
	if (founded) orderList[i] = order;
	else  orderList.push_back(order);
	
	spdlog::get("console")->info() << "[Trade Thread] Response | order submitted...ID:" << order->BrokerOrderSeq << ";Order submit Status:" << CommonUtils::InterpretOrderSubmitStatusCode(order->OrderSubmitStatus);
	spdlog::get("console")->info() << "[Trade Thread] Response | order submitted...ID:" << order->BrokerOrderSeq << ";Order Status:" << CommonUtils::InterpretOrderStatusCode(order->OrderStatus);
	spdlog::get("console")->info() << "[Trade Thread] Response | order submitted...ID:" << order->BrokerOrderSeq << ";StatusMsg:" << order->StatusMsg;
	//SetEvent(g_tradehEvent);
}

void CtpTradeSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	CThostFtdcTradeField* trade = new CThostFtdcTradeField();
	memcpy(trade, pTrade, sizeof(CThostFtdcTradeField));
	bool founded = false;     unsigned int i = 0;
	for (i = 0; i<tradeList.size(); i++){
		if (tradeList[i]->TradeID == trade->TradeID) {
			founded = true;   break;
		}
	}
	if (founded) tradeList[i] = trade;
	else  tradeList.push_back(trade);
	spdlog::get("console")->info() << "[Trade Thread] Response | order traded...TradeID:" << trade->TradeID;
	//SetEvent(g_tradehEvent);
}


void CtpTradeSpi::OnHeartBeatWarning(int nTimeLapse)
{
	spdlog::get("console")->info() << "[Trade Thread] Response | heartbeat is out of time limit..."
		<< " TimerLapse = " << nTimeLapse ;
}

void CtpTradeSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}

bool CtpTradeSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret){
		spdlog::get("console")->info() << "[Trade Thread] Response | " << pRspInfo->ErrorMsg;
	}
	return ret;
}

void CtpTradeSpi::ReqOrderInsert(Order ord){
	spdlog::get("console")->info() << ("Execute Order (") << ord.GetInstrumentId() << ", " \
		<< ord.GetRefExchangePrice() << ", " \
		<< (ord.GetExchangeDirection() == ExchangeDirection::Buy ? "Buy)" : "Sell)");

	ord.SetIdentityInfo(m_brokerID, m_userID, m_userID, m_orderRef);
	int nextOrderRef = atoi(m_orderRef);
	sprintf_s(m_orderRef, "%d", ++nextOrderRef);

	CThostFtdcInputOrderField ordstruct;
	bool success = ord.GetOrderOriginStruct(ordstruct);
	if (success){
		spdlog::get("console")->info() << "[Debug] Send Order:" << CommonUtils::StringFromStruct(ordstruct);
		int ret = pUserApi->ReqOrderInsert(&ordstruct, ++requestId);
		spdlog::get("console")->info() << "[Trade Thread] Request | insert order..." << ((ret == 0) ? "success" : "fail");
	}
	else{
		spdlog::get("console")->info() << "[Trade Thread] Invalid OrderField construct";
	}



	if (pAccountMgr)
		pAccountMgr->setUpdated(false);

	//fresh accout
	spdlog::get("console")->info() << "[Trade Thread] Order executed. begin to refresh Account info...";
	ReqQryTradingAccount();
}
//
/////TFtdcTimeConditionType是一个有效期类型类型
///////////////////////////////////////////////////////////////////////////
/////立即完成，否则撤销
//#define THOST_FTDC_TC_IOC '1'
/////本节有效
//#define THOST_FTDC_TC_GFS '2'
/////当日有效
//#define THOST_FTDC_TC_GFD '3'
/////指定日期前有效
//#define THOST_FTDC_TC_GTD '4'
/////撤销前有效
//#define THOST_FTDC_TC_GTC '5'
/////集合竞价有效
//#define THOST_FTDC_TC_GFA '6'
//
/////TFtdcVolumeConditionType是一个成交量类型类型
///////////////////////////////////////////////////////////////////////////
/////任何数量
//#define THOST_FTDC_VC_AV '1'
/////最小数量
//#define THOST_FTDC_VC_MV '2'
/////全部数量
//#define THOST_FTDC_VC_CV '3'

//立即全部成交否则自动撤销指令(FOK指令)，指在限定价位下达指令，如果该指令下所有申报手数未能全部成交，该指令下所有申报手数自动被系统撤销。
//
//立即成交剩余指令自动撤销指令(FAK指令)

//
//#define THOST_FTDC_D_Buy '0'
//	///卖
//#define THOST_FTDC_D_Sell '1
//
///////////////////////////////////////////////////////////////////////////
/////TFtdcOffsetFlagType是一个开平标志类型
///////////////////////////////////////////////////////////////////////////
/////开仓
//#define THOST_FTDC_OF_Open '0'
/////平仓
//#define THOST_FTDC_OF_Close '1'
/////强平
//#define THOST_FTDC_OF_ForceClose '2'
/////平今
//#define THOST_FTDC_OF_CloseToday '3'
/////平昨
//#define THOST_FTDC_OF_CloseYesterday '4'
/////强减
//#define THOST_FTDC_OF_ForceOff '5'
/////本地强平
//#define THOST_FTDC_OF_LocalForceClose '6'
//
//typedef char TThostFtdcOffsetFlagType;