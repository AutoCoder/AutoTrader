#include "stdafx.h"
#include "AccountManger.h"
#include "OrderQueue.h"
#include "Order.h"
#include "ThostFtdcTraderApi.h"
#include "traderspi.h"
#include "windows.h"
#include "config.h"


extern threadsafe_queue<Order> order_queue;
extern HANDLE g_tradehEvent;
extern int requestId;

std::vector<CThostFtdcOrderField*> orderList;
std::vector<CThostFtdcTradeField*> tradeList;

char MapDirection(char src, bool toOrig);
char MapOffset(char src, bool toOrig);

AccountMangerSpi::AccountMangerSpi(CThostFtdcTraderApi* p, const char * brokerID, const char* userID, const char* password)
	: pUserApi(p)
	, m_frontID(-1)
	, m_sessionID(-1)
{
	strcpy_s(m_brokerID, brokerID);
	strcpy_s(m_userID, userID);
	strcpy_s(m_password, password);
}

AccountMangerSpi::~AccountMangerSpi()
{
}

void AccountMangerSpi::ExcuteOrderQueue()
{
	std::cout << "Start to trade" << std::endl;
	std::cout << "> start to loop order queue" << std::endl;
	while (!order_queue.empty()){
		Order ord;
		if (order_queue.try_pop(ord)){ // if pop success
			std::cout << "Excute Order regarding instrumentID:" << ord.GetInstrumentId() << std::endl;
			//Todo: according ord to insert order
		}

		//query accout to refresh the cashed the investor position
		// todo : sleep 500ms
		Sleep(500);
	}

	std::cout << "> end to loop order queue" << std::endl;
}

void AccountMangerSpi::OnFrontConnected()
{
	std::cout << __FUNCTION__ << std::endl;
	SetEvent(g_tradehEvent);
}

void AccountMangerSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.UserID, m_userID);
	strcpy_s(req.Password, m_password);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);
	std::cout << " Request | send logging ..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void AccountMangerSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {

		m_frontID = pRspUserLogin->FrontID;
		m_sessionID = pRspUserLogin->SessionID;
		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		sprintf_s(m_orderRef, "%d", ++nextOrderRef);
		std::cout << " Response | login successfully...CurrentDate:"
			<< pRspUserLogin->TradingDay << std::endl;
	}
	if (bIsLast) SetEvent(g_tradehEvent);
}

void AccountMangerSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++requestId);
	std::cout << " Request | sending settlementInfo confirmation..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void AccountMangerSpi::OnRspSettlementInfoConfirm(
	CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
		std::cout << " Response | settlementInfo..." << pSettlementInfoConfirm->InvestorID
			<< "...<" << pSettlementInfoConfirm->ConfirmDate
			<< " " << pSettlementInfoConfirm->ConfirmTime << ">...Confirm" << std::endl;
	}
	if (bIsLast) SetEvent(g_tradehEvent);
}

void AccountMangerSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInstrument(&req, ++requestId);
	std::cout << " Request | send Instrument Query..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void AccountMangerSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInstrument){
		std::cout << " Response | Instrument:" << pInstrument->InstrumentID
			<< " DeliveryMonth:" << pInstrument->DeliveryMonth
			<< " LongMarginRatio:" << pInstrument->LongMarginRatio
			<< " ShortMarginRatio:" << pInstrument->ShortMarginRatio << std::endl;
	}
	if (bIsLast) SetEvent(g_tradehEvent);
}

void AccountMangerSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	int ret = pUserApi->ReqQryTradingAccount(&req, ++requestId);
	std::cout << " Request | send trading account query..." << ((ret == 0) ? "success" : "fail") << std::endl;

}

void AccountMangerSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pTradingAccount){
		memcpy(&m_accountInfo, pTradingAccount, sizeof(CThostFtdcTradingAccountField));
		std::cout << " Response | Balance:" << pTradingAccount->Balance
			<< " Available:" << pTradingAccount->Available
			<< " CurrMargin:" << pTradingAccount->CurrMargin
			<< " CloseProfit:" << pTradingAccount->CloseProfit
			<< " PositionProfit:" << pTradingAccount->PositionProfit
			<< " Commission:" << pTradingAccount->Commission
			<< " FrozenMargin:" << pTradingAccount->FrozenMargin
			<< std::endl;
	}

	if (bIsLast) SetEvent(g_tradehEvent);
}

void AccountMangerSpi::ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	strcpy_s(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInvestorPosition(&req, ++requestId);
	std::cout << " Request | send InvestorPosition query..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void AccountMangerSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField *pInvestorPosition,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition){
		std::cout << " Response| Instrument:" << pInvestorPosition->InstrumentID
			<< " PosiDirection:" << MapDirection(pInvestorPosition->PosiDirection - 2, false)
			<< " Position:" << pInvestorPosition->Position
			<< " Yesterday Position:" << pInvestorPosition->YdPosition
			<< " Today Position:" << pInvestorPosition->TodayPosition
			<< " Position Profit:" << pInvestorPosition->PositionProfit
			<< " UseMargin:" << pInvestorPosition->UseMargin << std::endl;
	}
	if (bIsLast) SetEvent(g_tradehEvent);
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

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	req.Direction = MapDirection(dir, true); 
	req.CombOffsetFlag[0] = MapOffset(kpp[0], true); 
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	req.LimitPrice = price;	
	req.VolumeTotalOriginal = vol;	
	req.TimeCondition = THOST_FTDC_TC_GFD;  
	req.VolumeCondition = THOST_FTDC_VC_AV; 
	req.MinVolume = 1;	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  

	//TThostFtdcPriceType	StopPrice;  
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	
	req.IsAutoSuspend = 0;  
	req.UserForceClose = 0; 

	int ret = pUserApi->ReqOrderInsert(&req, ++requestId);
	std::cout << " Request | insert order..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void AccountMangerSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrder){
		std::cout << "Response | Insert order success...Order Reference:" << pInputOrder->OrderRef << std::endl;
	}
	if (bIsLast) SetEvent(g_tradehEvent);
}

void AccountMangerSpi::ReqOrderAction(TThostFtdcSequenceNoType orderSeq)
{
	bool found = false; unsigned int i = 0;
	for (i = 0; i<orderList.size(); i++){
		if (orderList[i]->BrokerOrderSeq == orderSeq){ found = true; break; }
	}
	if (!found){ std::cout << " Request | Insert order doesn't exist." << std::endl; return; }

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
	std::cout << " Request | backout order..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void AccountMangerSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField *pInputOrderAction,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction){
		std::cout << " Response | backout order success..."
			<< "Exchange ID:" << pInputOrderAction->ExchangeID
			<< " Order System ID:" << pInputOrderAction->OrderSysID << std::endl;
	}
	if (bIsLast) SetEvent(g_tradehEvent);
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
	std::cout << " Response | order submitted...ID:" << order->BrokerOrderSeq << std::endl;
	SetEvent(g_tradehEvent);
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
	std::cout << " Response | order traded...TradeID:" << trade->TradeID << std::endl;
	SetEvent(g_tradehEvent);
}

void AccountMangerSpi::OnFrontDisconnected(int nReason)
{
	std::cout << " Response | Disconnected..."
		<< " reason=" << nReason << std::endl;
}

void AccountMangerSpi::OnHeartBeatWarning(int nTimeLapse)
{
	std::cout << "Response | heartbeat is out of time limit..."
		<< " TimerLapse = " << nTimeLapse << std::endl;
}

void AccountMangerSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}

bool AccountMangerSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret){
		std::cout << " Response | " << pRspInfo->ErrorMsg << std::endl;
	}
	return ret;
}

char MapDirection(char src, bool toOrig = true){
	if (toOrig){
		if ('b' == src || 'B' == src){ src = '0'; }
		else if ('s' == src || 'S' == src){ src = '1'; }
	}
	else{
		if ('0' == src){ src = 'B'; }
		else if ('1' == src){ src = 'S'; }
	}
	return src;
}
char MapOffset(char src, bool toOrig = true){
	if (toOrig){
		if ('o' == src || 'O' == src){ src = '0'; }
		else if ('c' == src || 'C' == src){ src = '1'; }
		else if ('j' == src || 'J' == src){ src = '3'; }
	}
	else{
		if ('0' == src){ src = 'O'; }
		else if ('1' == src){ src = 'C'; }
		else if ('3' == src){ src = 'J'; }
	}
	return src;
}