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
#include "IPositionControl.h"
#include "AP_Mgr.h"

extern int requestId;

extern std::condition_variable cv_trade;

std::vector<CThostFtdcOrderField*> orderList;
std::vector<CThostFtdcTradeField*> tradeList;

CtpTradeSpi::CtpTradeSpi(CThostFtdcTraderApi* p, const char * brokerID, const char* userID, const char* password, const char* prodname)
	: pUserApi(p)
	, m_frontID(-1)
	, m_sessionID(-1)
	, m_stateChangeHandler(this)
	, m_firstquery_order(true)
	, m_firstquery_trade(true)
	, m_firstquery_Detail(true)
	, m_firstquery_TradingAccount(true)
	, m_firstquery_Position(true)
	, m_firstquery_Instrument(true)
{
	strcpy_s(m_brokerID, brokerID);
	strcpy_s(m_userID, userID);
	strcpy_s(m_password, password);
	strcpy_s(m_productName, prodname);
}

CtpTradeSpi::~CtpTradeSpi(){}

void CtpTradeSpi::OnFrontConnected(){
	SYNC_PRINT << "[Trade Thread] Response | connected...";
	m_stateChangeHandler.OnFrontConnected();
}

void CtpTradeSpi::OnFrontDisconnected(int nReason){
	SYNC_PRINT << "[Trade Thread] Response | Disconnected..." << " reason=" << nReason;
}

void CtpTradeSpi::ReqUserLogin(){
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.UserID, m_userID);
	strcpy_s(req.Password, m_password);
	strcpy_s(req.UserProductInfo, m_productName);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);
	SYNC_PRINT << "[Trade Thread] Request | send logging ..." << ((ret == 0) ? "success" : "fail");
}

void CtpTradeSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {

		m_frontID = pRspUserLogin->FrontID;
		m_sessionID = pRspUserLogin->SessionID;

		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		sprintf_s(m_orderRef, "%d", ++nextOrderRef);

		SYNC_PRINT << "[Trade Thread] Response | login successfully...CurrentDate:" << pRspUserLogin->TradingDay << "| FrontID: " << m_frontID << "| SessionID: " << m_sessionID;

		m_stateChangeHandler.OnLogined();
	}
}

void CtpTradeSpi::ReqSettlementInfoConfirm(){
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++requestId);
	SYNC_PRINT << "[Trade Thread] Request | sending settlementInfo confirmation..." << ((ret == 0) ? "success" : "fail");
}

void CtpTradeSpi::OnRspSettlementInfoConfirm( CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
		SYNC_PRINT << "[Trade Thread] Response | settlementInfo..." 
			<< pSettlementInfoConfirm->InvestorID
			<< "...<" << pSettlementInfoConfirm->ConfirmDate
			<< " " << pSettlementInfoConfirm->ConfirmTime << ">...Confirm";
	}

	m_stateChangeHandler.OnConfirmedSettlementInfo();
}

void CtpTradeSpi::ReqQryOrder(){
	CThostFtdcQryOrderField req;
	memset(&req, 0, sizeof(req));

	strcpy_s(req.InvestorID, m_userID);//投资者代码,也是userId
	int ret = pUserApi->ReqQryOrder(&req, ++requestId);

	SYNC_PRINT << " 请求 | 发送查询报单..." << ((ret == 0) ? "成功" : "失败") << " ret:" << ret; //ret值为-3表示每秒发送请求数超过许可数
}

///请求查询报单响应
void CtpTradeSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	SYNC_PRINT << "[Trade Thread] Response | Query order : " << pOrder->OrderRef;

	if (!IsErrorRspInfo(pRspInfo) && pOrder)
	{
		if (m_firstquery_order == true)
		{
			std::shared_ptr<CThostFtdcOrderField> order(new CThostFtdcOrderField());
			CThostFtdcOrderField copy_order;
			memcpy(&copy_order, pOrder, sizeof(CThostFtdcOrderField));
			AP::GetManager().pushTodayOrder(copy_order);

			if (bIsLast)
			{
				m_firstquery_order = false;
				SYNC_PRINT << "所有合约报单次数：" << AP::GetManager().TodayOrderCount();
				SYNC_PRINT << AP::GetManager().TodayOrderToString();
			}
			m_stateChangeHandler.OnRspQryOrder();
		}
	}
	else
	{
		if (m_firstquery_order == true)
		{
			m_firstquery_order = false;
			SYNC_PRINT << "查询报单出错，或没有报单";
			m_stateChangeHandler.OnRspQryOrder();
		}

	}
}

void CtpTradeSpi::ReqQryTrade(){
	CThostFtdcQryTradeField req;
	memset(&req, 0, sizeof(req));

	strcpy_s(req.InvestorID, m_userID);//投资者代码,也是userId

	int ret = pUserApi->ReqQryTrade(&req, ++requestId);

	SYNC_PRINT << " 请求 | 发送成交查询..." << ((ret == 0) ? "成功" : "失败") << " ret:" << ret;//ret值为-3表示每秒发送请求数超过许可数
}

//请求查询成交响应,要区分程序启动时第一次查询跟之后的查询
//只能查询当天的，昨仓不能，所以还要查询持仓明细OnRspQryInvestorPositionDetail()
void CtpTradeSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pTrade)
	{
		if (m_firstquery_trade == true)
		{
			CThostFtdcTradeField trade;
			memcpy(&trade, pTrade, sizeof(CThostFtdcTradeField));
			AP::GetManager().pushTodayTrade(trade);

			if (bIsLast)
			{
				m_firstquery_trade = false;
				SYNC_PRINT << "成交次数：" << AP::GetManager().TodayTradeCount();
				SYNC_PRINT << AP::GetManager().TodayTradeToString();
				m_stateChangeHandler.OnRspQryTrade();
			}
		}
	}
	else{
		if (m_firstquery_trade == true){
			m_firstquery_trade = false;
			m_stateChangeHandler.OnRspQryTrade();
		}
	}
}

void CtpTradeSpi::ReqQryInvestorPositionDetail(){
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));

	strcpy_s(req.InvestorID, m_userID);//投资者代码,也是userId

	//strcpy(req.InstrumentID, "IF1402");

	int ret = pUserApi->ReqQryInvestorPositionDetail(&req, ++requestId);

	SYNC_PRINT << " 请求 | 发送投资者持仓明细查询..." << ((ret == 0) ? "成功" : "失败") << " ret:" << ret;//ret值为-3表示每秒发送请求数超过许可数
}


void CtpTradeSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPositionDetail)
	{
		//按时间先后排序返回，按合约返回

		//所有合约
		if (m_firstquery_Detail == true)
		{
			//对于所有合约，不保存已平仓的，只保存未平仓的
			//将程序启动前的持仓记录保存到未平仓容器tradeList_notClosed_account_long和tradeList_notClosed_account_short
			//为什么不在查询成交回调函数OnRspQryTrade()来处理,因为只能查询当天的
			//使用结构体用CThostFtdcTradeField，因为含有时间字段，而CThostFtdcInvestorPositionDetailField没有时间字段

			CThostFtdcTradeField trade;

			strcpy_s(trade.InvestorID, pInvestorPositionDetail->InvestorID);///投资者代码
			strcpy_s(trade.InstrumentID, pInvestorPositionDetail->InstrumentID);///合约代码
			strcpy_s(trade.ExchangeID, pInvestorPositionDetail->ExchangeID);///交易所代码
			trade.Direction = pInvestorPositionDetail->Direction;///买卖方向
			trade.Price = pInvestorPositionDetail->OpenPrice;///价格
			trade.Volume = pInvestorPositionDetail->Volume;///数量
			strcpy_s(trade.TradeDate, pInvestorPositionDetail->OpenDate);///成交时期


			if (pInvestorPositionDetail->Volume > 0)//筛选未平仓的
			{
				if (trade.Direction == '0')
					AP::GetManager().pushYesterdayUnClosedTrade(trade, AP::Buy);
				else if (trade.Direction == '1')
					AP::GetManager().pushYesterdayUnClosedTrade(trade, AP::Sell);
			}


			//输出所有合约的持仓明细，要在这边进行下一步的查询ReqQryTradingAccount();
			if (bIsLast)
			{
				m_firstquery_Detail = false;

				SYNC_PRINT << "账户所有合约未平仓单笔数(不是手数,一笔可对应多手):多单" << AP::GetManager().yesterdayUnClosedTradeCount(AP::Buy) << " 空单" << AP::GetManager().yesterdayUnClosedTradeCount(AP::Sell);
				SYNC_PRINT << "--------先多后空-------";

				SYNC_PRINT << AP::GetManager().YesterdayUnClosedTradeToString(AP::Buy);
				SYNC_PRINT << AP::GetManager().YesterdayUnClosedTradeToString(AP::Sell);

				SYNC_PRINT << "--------结束-------";

				SYNC_PRINT << "查询投资者持仓明细正常";
				m_stateChangeHandler.OnRspQryInvestorPositionDetail();

			}
		}
	}
	else
	{
		if (m_firstquery_Detail == true)
		{
			m_firstquery_Detail = false;
			SYNC_PRINT << "查询投资者持仓明细出错，或没有投资者持仓明细";
			m_stateChangeHandler.OnRspQryInvestorPositionDetail();
		}
	}
}

void CtpTradeSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInstrument(&req, ++requestId);
	SYNC_PRINT << "[Trade Thread] Request | send Instrument Query..." << ((ret == 0) ? "success" : "fail");
}

void CtpTradeSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInstrument){
		SYNC_PRINT << "[Trade Thread] Response | Instrument:" << pInstrument->InstrumentID
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
	int ret = -1;
	while (true){
		ret = pUserApi->ReqQryTradingAccount(&req, ++requestId);
		if (ret == 0){
			SYNC_PRINT << "[Trade Thread] Request | send trading account query...success";
			break;
		}
		else{
			//SYNC_PRINT << "[Trade Thread] Request | send trading account query... fail";
			if (AP::GetManager().isReady())
				break;
			sleep(2000);
		}
	}
}

void CtpTradeSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pTradingAccount){
		AP::GetManager().update(*pTradingAccount);

		memcpy(&m_accountInfo, pTradingAccount, sizeof(CThostFtdcTradingAccountField));
		SYNC_PRINT << "[Trade Thread] Response | Balance:" << pTradingAccount->Balance
			<< " Available:" << pTradingAccount->Available
			<< " CurrMargin:" << pTradingAccount->CurrMargin
			<< " CloseProfit:" << pTradingAccount->CloseProfit
			<< " PositionProfit:" << pTradingAccount->PositionProfit
			<< " Commission:" << pTradingAccount->Commission
			<< " FrozenMargin:" << pTradingAccount->FrozenMargin;
		//m_isAccountFreshed = true;
	}

	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::ReqQryInvestorPosition()//(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	//CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, m_brokerID);
	strcpy_s(req.InvestorID, m_userID);
	strcpy_s(req.InstrumentID, "");// pAccountMgr->InstrumentID());

	int ret = -1;
	while (true){
		ret = pUserApi->ReqQryInvestorPosition(&req, ++requestId);
		//ret = pUserApi->ReqQryInvestorPositionDetail(&req, ++requestId);
		if (ret == 0){
			SYNC_PRINT << "[Trade Thread] Request | send InvestorPosition query...success";
			break;
		}
		else{
			//SYNC_PRINT << "[Trade Thread] Request | send InvestorPosition query... fail";
			if (AP::GetManager().isReady())
				break;
			sleep(2000);
		}
	}
}

void CtpTradeSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField *pInvestorPosition, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition){
		//depreted
		//if (pAccountMgr)
		//	pAccountMgr->update(*pInvestorPosition);
		SYNC_PRINT << "[Trade Thread] Response| Instrument:" << pInvestorPosition->InstrumentID
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
		SYNC_PRINT << "[Back Order]" << CommonUtils::StringFromStruct(*pInputOrder);
		SYNC_PRINT << "[Trade Thread] Response | Insert order success...Order Reference:" << pInputOrder->OrderRef;
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::ReqOrderAction(TThostFtdcSequenceNoType orderSeq)
{
	bool found = false; unsigned int i = 0;
	for (i = 0; i<orderList.size(); i++){
		if (orderList[i]->BrokerOrderSeq == orderSeq){ found = true; break; }
	}
	if (!found){ SYNC_PRINT << "[Trade Thread] Request | Insert order doesn't exist."; return; }

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
	SYNC_PRINT << "[Trade Thread] Request | backout order..." << ((ret == 0) ? "success" : "fail");
}

void CtpTradeSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField *pInputOrderAction,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction){
		SYNC_PRINT << "[Trade Thread] Response | backout order success..."
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
	
	//SYNC_PRINT << "[Trade Thread] Response | order submitted...ID:" << order->BrokerOrderSeq << ";Order submit Status:" << CommonUtils::InterpretOrderSubmitStatusCode(order->OrderSubmitStatus);
	//SYNC_PRINT << "[Trade Thread] Response | order submitted...ID:" << order->BrokerOrderSeq << ";Order Status:" << CommonUtils::InterpretOrderStatusCode(order->OrderStatus);
	SYNC_PRINT << "[Trade Thread] Response | order submitted...ID:" << order->BrokerOrderSeq << ";StatusMsg:" << order->StatusMsg;
	//SetEvent(g_tradehEvent);
}
extern std::mutex g_OrderRunMtx;

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
	SYNC_PRINT << "[Trade Thread] Response | order traded at " << trade->TradeDate << trade->TradeTime << "...TradeID:" << trade->TradeID;

	////fresh accout
	//SYNC_PRINT << "[Trade Thread] Order executed. begin to refresh Account info...";

	//if (pAccountMgr->isReady()) // this check means that the history trade before account query will not record
	//	Position::GetManager().PushTradeItem(*pTrade);
	//ReqQryTradingAccount(); //depreted: calulate locally instead

	//sync the order execute process. Notify that the order trade process is finished.
	//if (g_OrderRunMtx.try_lock())
	//	g_OrderRunMtx.unlock();
	//g_OrderRunMtx.unlock();
}


void CtpTradeSpi::OnHeartBeatWarning(int nTimeLapse)
{
	SYNC_PRINT << "[Trade Thread] Response | heartbeat is out of time limit..."
		<< " TimerLapse = " << nTimeLapse ;
}

void CtpTradeSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}

bool CtpTradeSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret && !std::string(pRspInfo->ErrorMsg).empty()){
		SYNC_PRINT << "[Trade Thread] Response | " << pRspInfo->ErrorMsg;
	}
	return ret;
}

void CtpTradeSpi::ReqOrderInsert(Order ord){
	SYNC_PRINT << ("Execute Order (") << ord.GetInstrumentId() << ", " \
		<< ord.GetRefExchangePrice() << ", " \
		<< (ord.GetExchangeDirection() == ExchangeDirection::Buy ? "Buy)" : "Sell)");

	ord.SetIdentityInfo(m_brokerID, m_userID, m_userID, m_orderRef);
	int nextOrderRef = atoi(m_orderRef);
	sprintf_s(m_orderRef, "%d", ++nextOrderRef);

	CThostFtdcInputOrderField ordstruct;
	bool success = ord.GetOrderOriginStruct(ordstruct);
	if (success){
		SYNC_PRINT << "[Debug] Send Order:" << CommonUtils::StringFromStruct(ordstruct);
		int ret = pUserApi->ReqOrderInsert(&ordstruct, ++requestId);
		SYNC_PRINT << "[Trade Thread] Request | insert order..." << ((ret == 0) ? "success" : "fail");
		//if (ret)
		//	g_OrderRunMtx.unlock();
	}
	else{
		SYNC_PRINT << "[Trade Thread] Invalid OrderField construct";
	}


	//depreted : don't Real-time update account by API 
	//if (pAccountMgr)
	//	pAccountMgr->setUpdated(false);

	////block the order executing thread 
	//m_isExecutingOrder.set_value(false);
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