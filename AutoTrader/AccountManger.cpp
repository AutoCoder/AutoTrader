#include "stdafx.h"
#include "AccountManger.h"
#include "OrderQueue.h"
#include "Order.h"
#include "ThostFtdcTraderApi.h"
#include "traderspi.h"
//#include "windows.h"
#include "config.h"
#include <condition_variable>
#include "spdlog/spdlog.h"

extern int requestId;

extern std::condition_variable cv_trade;

std::vector<CThostFtdcOrderField*> orderList;
std::vector<CThostFtdcTradeField*> tradeList;

//char MapDirection(char src, bool toOrig);
//char MapOffset(char src, bool toOrig);

AccountMangerSpi::AccountMangerSpi(CThostFtdcTraderApi* p, const char * brokerID, const char* userID, const char* password, const char* prodname)
	: pUserApi(p)
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

AccountMangerSpi::~AccountMangerSpi()
{
}

void AccountMangerSpi::OnFrontConnected()
{
	std::cout << __FUNCTION__ << std::endl;
	//SetEvent(g_tradehEvent);
	m_isFrontConnected = true;
	cv_trade.notify_all();
}

void AccountMangerSpi::OnFrontDisconnected(int nReason)
{
	spdlog::get("console")->info() << "[Trade Thread] Response | Disconnected..."
		<< " reason=" << nReason;

	m_isFrontConnected = false;
	m_islogin = false;
}

void AccountMangerSpi::ReqUserLogin()
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

void AccountMangerSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
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

void AccountMangerSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | sending settlementInfo confirmation..." << ((ret == 0) ? "success" : "fail");

}

void AccountMangerSpi::OnRspSettlementInfoConfirm(
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

void AccountMangerSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInstrument(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | send Instrument Query..." << ((ret == 0) ? "success" : "fail");
}

void AccountMangerSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
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

void AccountMangerSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	int ret = pUserApi->ReqQryTradingAccount(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | send trading account query..." << ((ret == 0) ? "success" : "fail");

}

void AccountMangerSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pTradingAccount){
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

void AccountMangerSpi::ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	strcpy_s(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInvestorPosition(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | send InvestorPosition query..." << ((ret == 0) ? "success" : "fail");
}

void AccountMangerSpi::OnRspQryInvestorPosition(
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

void AccountMangerSpi::ReqOrderInsert(TThostFtdcInstrumentIDType instId,
	TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
	TThostFtdcPriceType price, TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	strcpy_s(req.InstrumentID, instId);
	strcpy_s(req.OrderRef, m_orderRef);
	int nextOrderRef = atoi(m_orderRef);
	sprintf_s(m_orderRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
	req.Direction = dir; 
	req.CombOffsetFlag[0] = kpp[0];
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	req.LimitPrice = price;	
	req.VolumeTotalOriginal = vol;	
	//FAK 立即成交剩余指令自动撤销指令 (THOST_FTDC_TC_IOC + THOST_FTDC_VC_AV)
	req.TimeCondition = THOST_FTDC_TC_IOC;
	req.VolumeCondition = THOST_FTDC_VC_AV; 
	req.MinVolume = 1;	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  

	//TThostFtdcPriceType	StopPrice;  
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	
	req.IsAutoSuspend = 0;  
	req.UserForceClose = 0; 

	int ret = pUserApi->ReqOrderInsert(&req, ++requestId);
	spdlog::get("console")->info() << "[Trade Thread] Request | insert order..." << ((ret == 0) ? "success" : "fail");
}

void AccountMangerSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrder){
		spdlog::get("console")->info() << "[Trade Thread] Response | Insert order success...Order Reference:" << pInputOrder->OrderRef;
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void AccountMangerSpi::ReqOrderAction(TThostFtdcSequenceNoType orderSeq)
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

void AccountMangerSpi::OnRspOrderAction(
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

void AccountMangerSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
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
	spdlog::get("console")->info() << "[Trade Thread] Response | order submitted...ID:" << order->BrokerOrderSeq;
	//SetEvent(g_tradehEvent);
}

void AccountMangerSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
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


void AccountMangerSpi::OnHeartBeatWarning(int nTimeLapse)
{
	spdlog::get("console")->info() << "[Trade Thread] Response | heartbeat is out of time limit..."
		<< " TimerLapse = " << nTimeLapse ;
}

void AccountMangerSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}

bool AccountMangerSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret){
		spdlog::get("console")->info() << " [Trade Thread] Response | " << pRspInfo->ErrorMsg;
	}
	return ret;
}
//
//char MapDirection(char src, bool toOrig = true){
//	if (toOrig){
//		if ('b' == src || 'B' == src){ src = '0'; }
//		else if ('s' == src || 'S' == src){ src = '1'; }
//	}
//	else{
//		if ('0' == src){ src = 'B'; }
//		else if ('1' == src){ src = 'S'; }
//	}
//	return src;
//}
//char MapOffset(char src, bool toOrig = true){
//	if (toOrig){
//		if ('o' == src || 'O' == src){ src = '0'; }
//		else if ('c' == src || 'C' == src){ src = '1'; }
//		else if ('j' == src || 'J' == src){ src = '3'; }
//	}
//	else{
//		if ('0' == src){ src = 'O'; }
//		else if ('1' == src){ src = 'C'; }
//		else if ('3' == src){ src = 'J'; }
//	}
//	return src;
//}

bool AccountMangerSpi::ExecuteOrder(const Order& ord){
	spdlog::get("console")->info() << (!m_isAccountFreshed ? "Account is not fresh..." : "Account is freshed");
	// if account is not refreshed, wait to refresh
	while (!m_isAccountFreshed){
		Sleep(50);
		//spdlog::get("console")->info() << "Account is not fresh, sleep(100)...";
	}
	spdlog::get("console")->info() << ("begin to execute order");
	double purchaseMoney = m_accountInfo.Available - (m_accountInfo.Balance * 0.8);
	//Get price == ord.GetInstrumentId()
	if (purchaseMoney > 0){
		int vol = purchaseMoney / ord.GetRefExchangePrice();

		TThostFtdcCombOffsetFlagType kpp;
		
		kpp[0] = (vol == 0) ? THOST_FTDC_OF_CloseToday : THOST_FTDC_OF_Open;
		std::string inst = ord.GetInstrumentId();

		char direction = (ord.GetExchangeDirection() == ExchangeDirection::Buy) ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
		ReqOrderInsert(const_cast<char*>(inst.c_str()), direction, kpp, ord.GetRefExchangePrice(), vol);
	}
	m_isAccountFreshed = false;

	//fresh accout
	spdlog::get("console")->info() << "[Trade Thread] Order executed. begin to refresh Account info...";
	ReqQryTradingAccount();

	return true;
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