#include "stdafx.h"
#include "tradespi.h"
#include "ThreadSafeQueue.h"
#include "Order.h"
#include "ThostFtdcTraderApi.h"
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
	STRCPY(m_brokerID, brokerID);
	STRCPY(m_userID, userID);
	STRCPY(m_password, password);
	STRCPY(m_productName, prodname);
}

CtpTradeSpi::~CtpTradeSpi(){}

void CtpTradeSpi::OnFrontConnected(){
	SYNC_PRINT << "[Trade] 回复 | 成功连接...";
	m_stateChangeHandler.OnFrontConnected();
}

void CtpTradeSpi::OnFrontDisconnected(int nReason){
	SYNC_PRINT << "[Trade] 回复 | 断开连接..." << " reason=" << nReason;
}

void CtpTradeSpi::ReqUserLogin(){
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.UserID, m_userID);
	STRCPY(req.Password, m_password);
	STRCPY(req.UserProductInfo, m_productName);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);
	SYNC_PRINT << "[Trade] 请求 | 登陆 ..." << ((ret == 0) ? "成功" : "失败");
}

void CtpTradeSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {

		m_frontID = pRspUserLogin->FrontID;
		m_sessionID = pRspUserLogin->SessionID;

		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		SPRINTF(m_orderRef, "%d", ++nextOrderRef);

		SYNC_PRINT << "[Trade] 回复 | 登陆成功...当前日期:" << pRspUserLogin->TradingDay << "| FrontID: " << m_frontID << "| SessionID: " << m_sessionID;

		m_stateChangeHandler.OnLogined();
	}
}

void CtpTradeSpi::ReqSettlementInfoConfirm(){
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++requestId);
	SYNC_PRINT << "[Trade] 请求 | 发送结算确认单..." << ((ret == 0) ? "成功" : "失败");
}

void CtpTradeSpi::OnRspSettlementInfoConfirm( CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
		SYNC_PRINT << "[Trade] 回复 | 结算确认成功..." 
			<< pSettlementInfoConfirm->InvestorID
			<< "...<" << pSettlementInfoConfirm->ConfirmDate
			<< " " << pSettlementInfoConfirm->ConfirmTime << ">...确认";
	}

	m_stateChangeHandler.OnConfirmedSettlementInfo();
}

void CtpTradeSpi::ReqQryOrder(){
	CThostFtdcQryOrderField req;
	memset(&req, 0, sizeof(req));

	STRCPY(req.InvestorID, m_userID);//投资者代码,也是userId
	int ret = pUserApi->ReqQryOrder(&req, ++requestId);

	SYNC_PRINT << "[Trade] 请求 | 发送查询报单..." << ((ret == 0) ? "成功" : "失败") << " ret:" << ret; //ret值为-3表示每秒发送请求数超过许可数
}

///请求查询报单响应
void CtpTradeSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	SYNC_PRINT << "[Trade] 回复 | 查询报单";

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
				SYNC_PRINT << "[Trade] 所有合约报单次数：" << AP::GetManager().todayOrderCount();
				SYNC_PRINT << AP::GetManager().todayOrderToString();
			}
			m_stateChangeHandler.OnRspQryOrder();
		}
	}
	else
	{
		if (m_firstquery_order == true)
		{
			m_firstquery_order = false;
			SYNC_PRINT << "[Trade] 查询报单出错，或没有报单";
			m_stateChangeHandler.OnRspQryOrder();
		}

	}
}

void CtpTradeSpi::ReqQryTrade(){
	CThostFtdcQryTradeField req;
	memset(&req, 0, sizeof(req));

	STRCPY(req.InvestorID, m_userID);//投资者代码,也是userId
	int ret = pUserApi->ReqQryTrade(&req, ++requestId);

	SYNC_PRINT << "[Trade] 请求 | 发送成交查询..." << ((ret == 0) ? "成功" : "失败") << " ret:" << ret;//ret值为-3表示每秒发送请求数超过许可数
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
				SYNC_PRINT << "成交次数：" << AP::GetManager().todayTradeCount();
				SYNC_PRINT << AP::GetManager().todayTradeToString();
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

	STRCPY(req.InvestorID, m_userID);//投资者代码,也是userId

	//strcpy(req.InstrumentID, "IF1402");
	int ret = pUserApi->ReqQryInvestorPositionDetail(&req, ++requestId);

	SYNC_PRINT << "[Trade] 请求 | 发送投资者持仓明细查询..." << ((ret == 0) ? "成功" : "失败") << " ret:" << ret;//ret值为-3表示每秒发送请求数超过许可数
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

			STRCPY(trade.InvestorID, pInvestorPositionDetail->InvestorID);///投资者代码
			STRCPY(trade.InstrumentID, pInvestorPositionDetail->InstrumentID);///合约代码
			STRCPY(trade.ExchangeID, pInvestorPositionDetail->ExchangeID);///交易所代码
			trade.Direction = pInvestorPositionDetail->Direction;///买卖方向
			trade.Price = pInvestorPositionDetail->OpenPrice;///价格
			trade.Volume = pInvestorPositionDetail->Volume;///数量
			STRCPY(trade.TradeDate, pInvestorPositionDetail->OpenDate);///成交时期


			if (pInvestorPositionDetail->Volume > 0)//筛选未平仓的
			{
				if (trade.Direction == '0')
					AP::GetManager().pushYesterdayUnClosedTrade(trade, AP::Long);
				else if (trade.Direction == '1')
					AP::GetManager().pushYesterdayUnClosedTrade(trade, AP::Short);
			}


			//输出所有合约的持仓明细，要在这边进行下一步的查询ReqQryTradingAccount();
			if (bIsLast)
			{
				m_firstquery_Detail = false;

				SYNC_PRINT << "[Trade]账户所有合约未平仓单笔数(不是手数,一笔可对应多手):多单" << AP::GetManager().yesterdayUnClosedTradeCount(AP::Long) << " 空单" << AP::GetManager().yesterdayUnClosedTradeCount(AP::Short);
				SYNC_PRINT << "--------先多后空-------";

				SYNC_PRINT << AP::GetManager().yesterdayUnClosedTradeToString(AP::Long);
				SYNC_PRINT << AP::GetManager().yesterdayUnClosedTradeToString(AP::Short);

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
			SYNC_PRINT << "[Trade] 查询投资者持仓明细出错，或没有投资者持仓明细";
			m_stateChangeHandler.OnRspQryInvestorPositionDetail();
		}
	}
}

void CtpTradeSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	int ret = -1;
	while (true){
		ret = pUserApi->ReqQryTradingAccount(&req, ++requestId);
		if (ret == 0){
			SYNC_PRINT << "[Trade] 请求 | 交易账户查询...成功";
			break;
		}
		SYNC_PRINT << "[Trade] 请求 | 交易账户查询...失败, ret:" << ret;
	}
}

void CtpTradeSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pTradingAccount){
		AP::GetManager().setAccountStatus(*pTradingAccount);

		memcpy(&m_accountInfo, pTradingAccount, sizeof(CThostFtdcTradingAccountField));
		SYNC_PRINT << "[Trade] Response | Balance:" << pTradingAccount->Balance
			<< " Available:" << pTradingAccount->Available
			<< " CurrMargin:" << pTradingAccount->CurrMargin
			<< " CloseProfit:" << pTradingAccount->CloseProfit
			<< " PositionProfit:" << pTradingAccount->PositionProfit
			<< " Commission:" << pTradingAccount->Commission
			<< " FrozenMargin:" << pTradingAccount->FrozenMargin;
		

		if (m_firstquery_TradingAccount){
			m_firstquery_TradingAccount = false;

			SYNC_PRINT << "[Trade] 资金查询正常，查询投资者持仓:";

			m_stateChangeHandler.OnRspQryTradingAccount();
		}
	}
	else{
		if (m_firstquery_TradingAccount){
			m_firstquery_TradingAccount = false;


			SYNC_PRINT << " [Trade] 资金查询出错,查询投资者持仓:";
			m_stateChangeHandler.OnRspQryTradingAccount();
		}

	}
}

void CtpTradeSpi::ReqQryInvestorPosition_all()
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	//strcpy(req.BrokerID, appId);
	//strcpy(req.InvestorID, userId);
	//strcpy(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInvestorPosition(&req, ++requestId);
	SYNC_PRINT << "[Trade] 请求 | 发送持仓查询..." << ((ret == 0) ? "成功" : "失败");
}

//[doesn't work, may delete later]
//void CtpTradeSpi::ReqQryInvestorPosition()//(TThostFtdcInstrumentIDType instId)
//{
//	CThostFtdcQryInvestorPositionField req;
//	//CThostFtdcQryInvestorPositionDetailField req;
//	memset(&req, 0, sizeof(req));
//	STRCPY(req.BrokerID, m_brokerID);
//	STRCPY(req.InvestorID, m_userID);
//	STRCPY(req.InstrumentID, "");// pAccountMgr->InstrumentID());
//
//	int ret = -1;
//	while (true){
//		ret = pUserApi->ReqQryInvestorPosition(&req, ++requestId);
//		//ret = pUserApi->ReqQryInvestorPositionDetail(&req, ++requestId);
//		if (ret == 0){
//			SYNC_PRINT << "[Trade] Request | send InvestorPosition query...success";
//			break;
//		}
//		else{
//			//SYNC_PRINT << "[Trade] Request | send InvestorPosition query... fail";
//			if (AP::GetManager().isReady())
//				break;
//			sleep(2000);
//		}
//	}
//}


//持仓查询回调函数,已经平仓的单子，持仓量为0了还会返回
void CtpTradeSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField *pInvestorPosition, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition){

		SYNC_PRINT << "[Trade] Response| Instrument:" << pInvestorPosition->InstrumentID
			<< " PosiDirection:" << pInvestorPosition->PosiDirection
			<< " Position:" << pInvestorPosition->Position
			<< " Yesterday Position:" << pInvestorPosition->YdPosition
			<< " Today Position:" << pInvestorPosition->TodayPosition
			<< " Position Profit:" << pInvestorPosition->PositionProfit
			<< " UseMargin:" << pInvestorPosition->UseMargin;

		if (m_firstquery_Position == true){
			AP::GetManager().pushTradeMessage(*pInvestorPosition);

			if (bIsLast)
			{
				m_firstquery_Position = false;
				m_stateChangeHandler.OnRspQryInvestorPosition();
			}
		}
	}
	else
	{
		if (m_firstquery_Position == true)
		{
			m_firstquery_Position = false;
			SYNC_PRINT << "[Trade] 查询持仓出错，或没有持仓";
			m_stateChangeHandler.OnRspQryInvestorPosition();
		}
	}
}

void CtpTradeSpi::ReqQryInstrument_all(){
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	int ret = pUserApi->ReqQryInstrument(&req, ++requestId);
	SYNC_PRINT << "[Trade] 请求 | 发送合约查询..." << ((ret == 0) ? "成功" : "失败") << " ret:" << ret;
}

void CtpTradeSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInstrument(&req, ++requestId);
	SYNC_PRINT << "[Trade] 请求 | 发送合约查询..." << ((ret == 0) ? "成功" : "失败");
}

void CtpTradeSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInstrument)
	{
		if (m_firstquery_Instrument == true)
		{
			AP::GetManager().pushInstrumentStruct(*pInstrument);

			if (bIsLast)
			{
				m_firstquery_Instrument = false;
				SYNC_PRINT << "[Trade] 所有持仓合约：" << AP::GetManager().getInstrumentList();

				// 为什么在tradespi线程初始化全部完成以后才启动MD?
				m_stateChangeHandler.OnRspQryInstrument();
			}
		}
	}

}

void CtpTradeSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrder){
		SYNC_PRINT << "[Back Order]" << CommonUtils::StringFromStruct(*pInputOrder);
		SYNC_PRINT << "[Trade] 回复 | 订单插入成功...订单引用:" << pInputOrder->OrderRef;
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::ReqOrderAction(const CThostFtdcOrderField& order)//TThostFtdcSequenceNoType orderSeq, TThostFtdcExchangeIDType exchangeId, TThostFtdcOrderSysIDType orderSysId)
{
	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);

	STRCPY(req.ExchangeID, order.ExchangeID);
	STRCPY(req.OrderSysID, order.OrderSysID);
	req.ActionFlag = THOST_FTDC_AF_Delete;

	int ret = pUserApi->ReqOrderAction(&req, ++requestId);
	SYNC_PRINT << "[Trade] 请求 | 撤销报单..." << ((ret == 0) ? "成功" : "失败");
}

void CtpTradeSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField *pInputOrderAction,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction){
		SYNC_PRINT << "[Trade] 回复 | 撤单成功..."
			<< "交易所代码:" << pInputOrderAction->ExchangeID
			<< "报单编号:" << pInputOrderAction->OrderSysID;
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	SYNC_PRINT << "[Trade] 报单回报:前置编号FrontID:" << pOrder->FrontID << " 会话编号SessionID:" << pOrder->SessionID << " OrderRef:" << pOrder->OrderRef;

	AP::GetManager().pushImmediateOrder(*pOrder);

	SYNC_PRINT << "[Trade] 回复 | 订单...ID:" << pOrder->BrokerOrderSeq << ";订单状态:" << CommonUtils::InterpretOrderSubmitStatusCode(pOrder->OrderSubmitStatus);
}

void CtpTradeSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	//CThostFtdcTradeField* trade = new CThostFtdcTradeField();
	//memcpy(trade, pTrade, sizeof(CThostFtdcTradeField));
	//bool founded = false;     unsigned int i = 0;
	//for (i = 0; i<tradeList.size(); i++){
	//	if (tradeList[i]->TradeID == trade->TradeID) {
	//		founded = true;   break;
	//	}
	//}
	//if (founded) tradeList[i] = trade;
	//else  tradeList.push_back(trade);
	SYNC_PRINT << "[Trade] 回复 | 订单成交于" << pTrade->TradeDate << pTrade->TradeTime << "...TradeID:" << pTrade->TradeID;

	////fresh accout
	//SYNC_PRINT << "[Trade] Order executed. begin to refresh Account info...";

	//if (pAccountMgr->isReady()) // this check means that the history trade before account query will not record
	//	Position::GetManager().PushTradeItem(*pTrade);
	//ReqQryTradingAccount(); //depreted: calulate locally instead

	//sync the order execute process. Notify that the order trade process is finished.
	//if (g_OrderRunMtx.try_lock())
	//	g_OrderRunMtx.unlock();
	//g_OrderRunMtx.unlock();
	
	AP::GetManager().pushTodayNewTrade(*pTrade);//会更新整个账户和仓位的状态，使资金状态保持最新
}


void CtpTradeSpi::OnHeartBeatWarning(int nTimeLapse)
{
	SYNC_PRINT << "[Trade] Response | heartbeat is out of time limit..."
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
		SYNC_PRINT << "[Trade] 回复 | " << pRspInfo->ErrorMsg;
	}
	return ret;
}

void CtpTradeSpi::ReqOrderInsert(Order ord){
	SYNC_PRINT << ("Execute Order (") << ord.GetInstrumentId() << ", " \
		<< ord.GetRefExchangePrice() << ", " \
		<< (ord.GetExchangeDirection() == THOST_FTDC_D_Buy ? "Buy)" : "Sell)");

	ord.SetIdentityInfo(m_brokerID, m_userID, m_userID, m_orderRef);
	int nextOrderRef = atoi(m_orderRef);
	SPRINTF(m_orderRef, "%d", ++nextOrderRef);

	CThostFtdcInputOrderField ordstruct;
	bool success = ord.GetOrderOriginStruct(ordstruct);
	if (success){
		SYNC_PRINT << "[Debug] 插入订单:" << CommonUtils::StringFromStruct(ordstruct);
		int ret = pUserApi->ReqOrderInsert(&ordstruct, ++requestId);
		SYNC_PRINT << "[Trade] 请求 | 插入订单..." << ((ret == 0) ? "成功" : "失败");
		//if (ret)
		//	g_OrderRunMtx.unlock();
	}
	else{
		SYNC_PRINT << "[Trade] Invalid OrderField construct";
	}
}

void CtpTradeSpi::ForceClose(){
	TThostFtdcInstrumentIDType    instId;//合约
	TThostFtdcDirectionType       dir;//方向,'0'买，'1'卖
	TThostFtdcCombOffsetFlagType  kpp;//开平，"0"开，"1"平,"3"平今
	TThostFtdcPriceType           price;//价格，0是市价,上期所不支持
	TThostFtdcVolumeType          vol;//数量

	for (auto item : AP::GetManager().getAllPositionMap()){
		//平多
		if (item.second.Holding_long > 0)
		{
			STRCPY(instId, item.second.InstId.c_str());
			dir = THOST_FTDC_D_Sell;// #define THOST_FTDC_D_Buy '0' ||| #define THOST_FTDC_D_Sell '1'
			price = item.second.LastPrice - 5 * AP::GetManager().getInstrumentField(instId).PriceTick;

			//上期所
			if (strcmp(AP::GetManager().getInstrumentField(instId).ExchangeID, "SHFE") == 0)
			{
				if (item.second.YdPosition_long == 0)//没有昨仓
				{
					SYNC_PRINT << "[Trade] 多单上期所 全部平今";;

					STRCPY(kpp, "3");//平今
					vol = item.second.Holding_long;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				else if (item.second.TodayPosition_long == 0)//没有今仓
				{
					SYNC_PRINT << "[Trade] 多单上期所 全部平昨";

					STRCPY(kpp, "1");//平仓
					vol = item.second.Holding_long;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				//同时持有昨仓和今仓
				else if (item.second.YdPosition_long > 0 && item.second.TodayPosition_long > 0)
				{
					SYNC_PRINT << "[Trade] 多单上期所同时 平今平昨";

					STRCPY(kpp, "3");//平今
					vol = item.second.TodayPosition_long;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

					STRCPY(kpp, "1");//平仓
					vol = item.second.YdPosition_long;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}

			}
			//非上期所
			else
			{
				SYNC_PRINT << "[Trade] 非上期所多单 平仓[不支持平今]";

				STRCPY(kpp, "1");
				vol = item.second.Holding_long;
				ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

			}
		}

		//平空
		if (item.second.Holding_short > 0)
		{
			STRCPY(instId, item.second.InstId.c_str());//或strcpy(instId, iter->first.c_str());
			dir = '0';
			price = item.second.LastPrice + 5 * AP::GetManager().getInstrumentField(instId).PriceTick;

			//上期所
			if (strcmp(AP::GetManager().getInstrumentField(instId).ExchangeID, "SHFE") == 0)
			{
				if (item.second.YdPosition_short == 0)//没有昨仓
				{
					SYNC_PRINT << "[Trade] 空单上期所 全部平今";

					STRCPY(kpp, "3");//平今
					vol = item.second.Holding_short;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				else if (item.second.TodayPosition_short == 0)//没有今仓
				{
					SYNC_PRINT << "[Trade] 空单上期所 全部平昨";

					STRCPY(kpp, "1");//平仓
					vol = item.second.Holding_short;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				//同时持有昨仓和今仓
				else if (item.second.YdPosition_short > 0 && item.second.TodayPosition_short > 0)
				{
					SYNC_PRINT << "[Trade] 空单上期所 同时平今平昨";

					STRCPY(kpp, "3");//平今
					vol = item.second.TodayPosition_short;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

					STRCPY(kpp, "1");//平仓
					vol = item.second.YdPosition_short;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
			}
			//非上期所
			else
			{
				SYNC_PRINT << "[Trade] 非上期所空单 平仓[不支持平今]";

				STRCPY(kpp, "1");
				vol = item.second.Holding_short;
				ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

			}
		}
	}
}

void CtpTradeSpi::CancelOrder(const std::string& MDtime, int aliveDuration, const std::string& instrumentId){
	for (auto item : AP::GetManager().getAllOrders())
	{
		if (std::string(item.InstrumentID) != instrumentId)
			continue;

		if (item.OrderStatus == THOST_FTDC_OST_PartTradedQueueing || item.OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
		{
			// 超过aliveDuration(6秒)未成交，撤单
			if (CommonUtils::TimeToSenconds(item.InsertTime) + aliveDuration < CommonUtils::TimeToSenconds(MDtime.c_str()) )
			{
				SYNC_PRINT << "[Trade] 撤单...报单ID:" << item.BrokerOrderSeq;
				ReqOrderAction(item);
			}

		}
	}
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