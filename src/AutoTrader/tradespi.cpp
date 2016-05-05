#include "stdafx.h"
#include "tradespi.h"
#include "ThreadSafeQueue.h"
#include "Order.h"
#include "ThostFtdcTraderApi.h"
#include "Config.h"
#include <condition_variable>
#include "spdlog/spdlog.h"
#include "CommonUtils.h"
#include "AP_Mgr.h"
#include "InstrumentInfoMgr.h"

std::vector<CThostFtdcOrderField*> orderList;
std::vector<CThostFtdcTradeField*> tradeList;

CtpTradeSpi::CtpTradeSpi(CThostFtdcTraderApi* p, const char * brokerID, const char* userID, const char* password, const char* prodname, AP::AccountDetailMgr& admgr, 
	RtnOrderCallback onRtnOrderCallback, RtnTradeCallback onRtnTradeCallback, CancelOrderCallback onRtnCancellOrderCallback)
	: pUserApi(p)
	, m_frontID(-1)
	, m_sessionID(-1)
	, m_stateChangeHandler(this)
	, m_account_detail_mgr(admgr)
	, m_OnRtnOrder_callback(onRtnOrderCallback)
	, m_OnRtnTrade_callback(onRtnTradeCallback)
	, m_OnCancelOrder_callback(onRtnCancellOrderCallback)
	, m_requestId(0)
{
	STRCPY(m_brokerID, brokerID);
	STRCPY(m_userID, userID);
	STRCPY(m_password, password);
	STRCPY(m_productName, prodname);
}

CtpTradeSpi::~CtpTradeSpi(){}

void CtpTradeSpi::ReqOrderInsert(Order ord){
	SYNC_PRINT << "[Trade] Request |" << "Insert Order (" << ord.GetInstrumentId() << ", " \
		<< ord.GetRefExchangePrice() << ", " \
		<< (ord.GetExchangeDirection() == THOST_FTDC_D_Buy ? "Buy)" : "Sell)");

	ord.SetIdentityInfo(m_brokerID, m_userID, m_userID, m_orderRef);
	int nextOrderRef = atoi(m_orderRef);
	SPRINTF(m_orderRef, "%d", ++nextOrderRef);

	CThostFtdcInputOrderField ordstruct;
	bool success = ord.GetOrderOriginStruct(ordstruct);
	if (success){
		//SYNC_PRINT << "[Debug] 插入订单:" << CommonUtils::StringFromStruct(ordstruct);
		SYNC_PRINT << "[Trade] Request | Inserting Order:\n" << CommonUtils::StringFromStruct(ordstruct);
		int ret = pUserApi->ReqOrderInsert(&ordstruct, ++m_requestId);
		SYNC_PRINT << "[Trade] Request | Insert Order..." << ((ret == 0) ? "Success" : "Fail");

	}
	else{
		SYNC_PRINT << "[Trade] Exception : Invalid OrderField construct";
	}
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

	int ret = pUserApi->ReqOrderAction(&req, ++m_requestId);
	//SYNC_PRINT << "[Trade] 请求 | 撤销报单..." << ((ret == 0) ? "成功" : "失败");
	SYNC_DEBUG_LOG << "[Trade] Request | Cancell Order...OrderSysID:" << order.OrderSysID << ((ret == 0) ? "Success" : "Fail");
}


void CtpTradeSpi::CancelOrder(long long MDtime, int aliveDuration, const std::string& instrumentId){
	for (auto item : m_account_detail_mgr.getAllOrders())
	{
		if (std::string(item.InstrumentID) != instrumentId)
			continue;

		if (item.OrderStatus == THOST_FTDC_OST_PartTradedQueueing || item.OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
		{
			// 超过aliveDuration(6秒)未成交，撤单
			if ((CommonUtils::TimeToSenconds(item.InsertTime) + aliveDuration) * 2 < MDtime) // unit = 0.5s
			{
				//SYNC_PRINT << "[Trade] 撤单...报单ID:" << item.BrokerOrderSeq;
				SYNC_DEBUG_LOG << "[Trade] Request | Cancel Order...OrderID:" << item.BrokerOrderSeq;
				ReqOrderAction(item);
			}

		}
	}
}

void CtpTradeSpi::ReqAllRateParameters(const std::vector<std::string>& instruments){
	for (auto instru : instruments){
		sleep(1000);
		m_querying.store(true);
		ReqQryInstrumentMarginRate(instru.c_str());
		WaitQueryEnd();

		sleep(1000);
		m_querying.store(true);
		ReqQryInstrumentCommissionRate(instru.c_str());
		WaitQueryEnd();
	}
}

void CtpTradeSpi::ForceClose(){
	TThostFtdcInstrumentIDType    instId;//合约
	TThostFtdcDirectionType       dir;//方向,'0'买，'1'卖
	TThostFtdcCombOffsetFlagType  kpp;//开平，"0"开，"1"平,"3"平今
	TThostFtdcPriceType           price;//价格，0是市价,上期所不支持
	TThostFtdcVolumeType          vol;//数量

	for (auto item : m_account_detail_mgr.getAllPositionMap()){
		//平多
		if (item.second.Holding_long > 0)
		{
			STRCPY(instId, item.second.InstId.c_str());
			dir = THOST_FTDC_D_Sell;// #define THOST_FTDC_D_Buy '0' ||| #define THOST_FTDC_D_Sell '1'
			price = item.second.LastPrice - 5 * m_account_detail_mgr.getInstrumentField(instId).PriceTick;

			//上期所
			if (strcmp(m_account_detail_mgr.getInstrumentField(instId).ExchangeID, "SHFE") == 0)
			{
				if (item.second.YdPosition_long == 0)//没有昨仓
				{
					//SYNC_PRINT << "[Trade] 多单上期所 全部平今";;
					SYNC_PRINT << "[Trade] Order(Direction:Long) from ShQiSuo, PingJin totally";

					STRCPY(kpp, "3");//平今
					vol = item.second.Holding_long;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				else if (item.second.TodayPosition_long == 0)//没有今仓
				{
					//SYNC_PRINT << "[Trade] 多单上期所 全部平昨";
					SYNC_PRINT << "[Trade] Order(Direction:Long) from ShQiSuo, PingCang totally";

					STRCPY(kpp, "1");//平仓
					vol = item.second.Holding_long;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				//同时持有昨仓和今仓
				else if (item.second.YdPosition_long > 0 && item.second.TodayPosition_long > 0)
				{
					//SYNC_PRINT << "[Trade] 多单上期所同时 平今平昨";
					SYNC_PRINT << "[Trade] Order(Direction:Long) from ShQiSuo, PingJin & PingCang totally";

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
				//SYNC_PRINT << "[Trade] 非上期所多单 平仓[不支持平今]";
				SYNC_PRINT << "[Trade] Order(Direction:Long) from Non-ShQiSuo, PingCang Totally[PingJin is unSupported]";

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
			price = item.second.LastPrice + 5 * m_account_detail_mgr.getInstrumentField(instId).PriceTick;

			//上期所
			if (strcmp(m_account_detail_mgr.getInstrumentField(instId).ExchangeID, "SHFE") == 0)
			{
				if (item.second.YdPosition_short == 0)//没有昨仓
				{
					//SYNC_PRINT << "[Trade] 空单上期所 全部平今";
					SYNC_PRINT << "[Trade] Order(Direction:Short) from ShQiSuo, PingJin totally";

					STRCPY(kpp, "3");//平今
					vol = item.second.Holding_short;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				else if (item.second.TodayPosition_short == 0)//没有今仓
				{
					//SYNC_PRINT << "[Trade] 空单上期所 全部平昨";
					SYNC_PRINT << "[Trade] Order(Direction:Short) from ShQiSuo, PingCang totally";

					STRCPY(kpp, "1");//平仓
					vol = item.second.Holding_short;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				//同时持有昨仓和今仓
				else if (item.second.YdPosition_short > 0 && item.second.TodayPosition_short > 0)
				{
					//SYNC_PRINT << "[Trade] 空单上期所 同时平今平昨";
					SYNC_PRINT << "[Trade] Order(Direction:Short) from ShQiSuo, PingJin & PingCang totally";

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
				//SYNC_PRINT << "[Trade] 非上期所空单 平仓[不支持平今]";
				SYNC_PRINT << "[Trade] Order(Direction:Short) from Non-ShQiSuo, PingCang Totally[PingJin is unSupported]";

				STRCPY(kpp, "1");
				vol = item.second.Holding_short;
				ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

			}
		}
	}
}


void CtpTradeSpi::OnFrontConnected(){
	SYNC_PRINT << "[Trade] Response | connected...";
	m_stateChangeHandler.OnFrontConnected();
}

void CtpTradeSpi::OnFrontDisconnected(int nReason){
	SYNC_PRINT << "[Trade] Response | disconnected..." << " Reason=" << nReason;
}

void CtpTradeSpi::ReqUserLogin(){
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.UserID, m_userID);
	STRCPY(req.Password, m_password);
	STRCPY(req.UserProductInfo, m_productName);
	int ret = pUserApi->ReqUserLogin(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Login ..." << ((ret == 0) ? "Success" : "Fail");
}

void CtpTradeSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {

		m_frontID = pRspUserLogin->FrontID;
		m_sessionID = pRspUserLogin->SessionID;

		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		SPRINTF(m_orderRef, "%d", ++nextOrderRef);

		SYNC_PRINT << "[Trade] Response | login success...Current Date:" << pRspUserLogin->TradingDay << "| FrontID: " << m_frontID << "| SessionID: " << m_sessionID;

		m_stateChangeHandler.OnLogined();
	}
}

void CtpTradeSpi::ReqSettlementInfoConfirm(){
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Send Settlement Confirm..." << ((ret == 0) ? "Success" : "Fail");
}

void CtpTradeSpi::OnRspSettlementInfoConfirm( CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
		SYNC_PRINT << "[Trade] Response | Settlement Confirm Success..." 
			<< pSettlementInfoConfirm->InvestorID
			<< "...<" << pSettlementInfoConfirm->ConfirmDate
			<< " " << pSettlementInfoConfirm->ConfirmTime << ">...Confirm";
	}

	m_stateChangeHandler.OnConfirmedSettlementInfo();
}

void CtpTradeSpi::ReqQryOrder(){
	CThostFtdcQryOrderField req;
	memset(&req, 0, sizeof(req));

	STRCPY(req.InvestorID, m_userID);//投资者代码,也是userId
	int ret = pUserApi->ReqQryOrder(&req, ++m_requestId);

	SYNC_PRINT << "[Trade] Request | Query Order..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret; //ret值为 0, 代表成功。 -1 , 表示网络连接失败; -2, 表示未处理请求超过许可数; -3表示每秒发送请求数超过许可数
}

///请求查询报单响应
void CtpTradeSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	SYNC_PRINT << "[Trade] Response | Query Order";

	if (!IsErrorRspInfo(pRspInfo) && pOrder)
	{
		std::shared_ptr<CThostFtdcOrderField> order(new CThostFtdcOrderField());
		CThostFtdcOrderField copy_order;
		memcpy(&copy_order, pOrder, sizeof(CThostFtdcOrderField));
		m_account_detail_mgr.pushTodayOrder(copy_order);
	}
	else
	{
		SYNC_PRINT << "[Trade] Query Order fail, or no orders exist";
	}

	if (bIsLast)
	{
		SYNC_PRINT << "[Trade] Inserted order count of all instruments today:" << m_account_detail_mgr.todayOrderCount();
		SYNC_PRINT << m_account_detail_mgr.todayOrderToString();
		m_stateChangeHandler.OnLastRspQryOrder();
	}
}

void CtpTradeSpi::ReqQryTrade(){
	CThostFtdcQryTradeField req;
	memset(&req, 0, sizeof(req));

	STRCPY(req.InvestorID, m_userID);//投资者代码,也是userId
	int ret = pUserApi->ReqQryTrade(&req, ++m_requestId);

	SYNC_PRINT << "[Trade] Request | Query Trade..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;//ret值为-3表示每秒发送请求数超过许可数
}

//请求查询成交响应,要区分程序启动时第一次查询跟之后的查询
//只能查询当天的，昨仓不能，所以还要查询持仓明细OnRspQryInvestorPositionDetail()
void CtpTradeSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	SYNC_PRINT << "[Trade] Response | Query Trade";

	if (!IsErrorRspInfo(pRspInfo) && pTrade)
	{
		CThostFtdcTradeField trade;
		memcpy(&trade, pTrade, sizeof(CThostFtdcTradeField));
		m_account_detail_mgr.pushTodayTrade(trade);
	}
	else{
		SYNC_PRINT << "[Trade] Query Trade fails!";
	}

	if (bIsLast)
	{
		SYNC_PRINT << "[Trade] Trade count Today：" << m_account_detail_mgr.todayTradeCount();
		SYNC_PRINT << m_account_detail_mgr.todayTradeToString();
		m_stateChangeHandler.OnLastRspQryTrade();
	}

}

void CtpTradeSpi::ReqQryInvestorPositionDetail(){
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));

	STRCPY(req.InvestorID, m_userID);//投资者代码,也是userId

	//strcpy(req.InstrumentID, "IF1402");
	int ret = pUserApi->ReqQryInvestorPositionDetail(&req, ++m_requestId);

	SYNC_PRINT << "[Trade] Request | Query Invester position details..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;//ret值为-3表示每秒发送请求数超过许可数
}


void CtpTradeSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	SYNC_PRINT << "[Trade] Response | Query Invester Position Details";

	if (!IsErrorRspInfo(pRspInfo) && pInvestorPositionDetail)
	{
		//按时间先后排序返回，按合约返回

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
				m_account_detail_mgr.pushYesterdayUnClosedTrade(trade, AP::Long);
			else if (trade.Direction == '1')
				m_account_detail_mgr.pushYesterdayUnClosedTrade(trade, AP::Short);
		}
	}
	else
	{
		//SYNC_PRINT << "[Trade] 查询投资者持仓明细出错，或没有投资者持仓明细";
		SYNC_PRINT << "[Trade] Query Invester Position Details unsuccessfully, or no Position Details exists";
	}

	//输出所有合约的持仓明细，要在这边进行下一步的查询ReqQryTradingAccount();
	if (bIsLast)
	{
		//账户所有合约未平仓单笔数(不是手数,一笔可对应多手):多单
		SYNC_PRINT << "[Trade] UnClosed Trade Count (All Instruments) [Long:" << m_account_detail_mgr.yesterdayUnClosedTradeCount(AP::Long) << ", Short:" << m_account_detail_mgr.yesterdayUnClosedTradeCount(AP::Short) << "]";
		//SYNC_PRINT << "--------先多后空-------";
		SYNC_PRINT << "--------Long First, Short after-------";

		SYNC_PRINT << m_account_detail_mgr.yesterdayUnClosedTradeToString(AP::Long);
		SYNC_PRINT << m_account_detail_mgr.yesterdayUnClosedTradeToString(AP::Short);

		//SYNC_PRINT << "--------结束-------";
		SYNC_PRINT << "--------Finished-------";

		//SYNC_PRINT << "查询投资者持仓明细正常";
		SYNC_PRINT << "Query Invester Position Details Successfully";
		m_stateChangeHandler.OnLastRspQryInvestorPositionDetail();
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
		ret = pUserApi->ReqQryTradingAccount(&req, ++m_requestId);
		if (ret == 0){
			SYNC_PRINT << "[Trade] Request | Query Trading Account...Sccuess";
			break;
		}
		SYNC_PRINT << "[Trade] Request | Query Trading Account...Fail, ret:" << ret;
	}
}

void CtpTradeSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pTradingAccount){
		m_account_detail_mgr.setAccountStatus(*pTradingAccount);

		memcpy(&m_accountInfo, pTradingAccount, sizeof(CThostFtdcTradingAccountField));
		SYNC_PRINT << "[Trade] Response | Balance:" << pTradingAccount->Balance
			<< " Available:" << pTradingAccount->Available
			<< " CurrMargin:" << pTradingAccount->CurrMargin
			<< " CloseProfit:" << pTradingAccount->CloseProfit
			<< " PositionProfit:" << pTradingAccount->PositionProfit
			<< " Commission:" << pTradingAccount->Commission
			<< " FrozenMargin:" << pTradingAccount->FrozenMargin;


		//SYNC_PRINT << "[Trade] 资金查询正常，查询投资者持仓:";
		SYNC_PRINT << "[Trade] Query Trading Account Successfully，next step-> Query Investor Position:";
	}
	else{
		SYNC_PRINT << " [Trade] Query Trading Account Unsuccessfully, next step-> Query Investor Position:";
	}

	if (bIsLast){
		m_stateChangeHandler.OnLastRspQryTradingAccount();
	}
}

void CtpTradeSpi::ReqQryInvestorPosition_all()
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	//strcpy(req.BrokerID, appId);
	//strcpy(req.InvestorID, userId);
	//strcpy(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInvestorPosition(&req, ++m_requestId);
	//SYNC_PRINT << "[Trade] 请求 | 发送持仓查询..." << ((ret == 0) ? "成功" : "失败");
	SYNC_PRINT << "[Trade] Request | Query Investor Positon..." << ((ret == 0) ? "Sccuess" : "Fail");
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
//			if (m_account_detail_mgr().isReady())
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

		SYNC_PRINT << "[Trade] Response | Instrument:" << pInvestorPosition->InstrumentID
			<< " PosiDirection:" << pInvestorPosition->PosiDirection
			<< " Position:" << pInvestorPosition->Position
			<< " Yesterday Position:" << pInvestorPosition->YdPosition
			<< " Today Position:" << pInvestorPosition->TodayPosition
			<< " Position Profit:" << pInvestorPosition->PositionProfit
			<< " UseMargin:" << pInvestorPosition->UseMargin;

		m_account_detail_mgr.pushTradeMessage(*pInvestorPosition);
	}
	else
	{
		//SYNC_PRINT << "[Trade] 查询持仓出错，或没有持仓";
		SYNC_PRINT << "[Trade] Query Investor Positon Unsuccessfully, or no positon exists";
	}

	if (bIsLast)
	{
		m_stateChangeHandler.OnLastRspQryInvestorPosition();
	}
}

void CtpTradeSpi::ReqQryInstrument_all(){
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	int ret = pUserApi->ReqQryInstrument(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query Instrument(All)..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;
}

void CtpTradeSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInstrument(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query Instrument(" << instId << ")..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;;
}

void CtpTradeSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInstrument)
	{
		if (((pInstrument->ProductClass == THOST_FTDC_PC_Futures) || (pInstrument->ProductClass == THOST_FTDC_PC_Options) ||
			(pInstrument->ProductClass == THOST_FTDC_PC_SpotOption) /*||(pInstrument->ProductClass ==THOST_FTDC_PC_Combination)*/) &&
			(pInstrument->IsTrading))
		{
			Instrument::Information info(*pInstrument);

			std::string instru_key(pInstrument->InstrumentID);
			InstrumentManager.Add(instru_key, info);
		}
	}
	else
	{
		if (pRspInfo)
		{
			SYNC_PRINT << "[Trade] Failed to query instrument info, error:" << pRspInfo->ErrorMsg;
		}
	}

	if (bIsLast)
	{
		//SYNC_PRINT << "[Trade] All Instruments in Position:" << //todo: return all position's instruments;
		SYNC_PRINT << "[Trade] All Instruments Queried:" << InstrumentManager.AllInstruments();
	}
}

void CtpTradeSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrder){
		//SYNC_PRINT << "[Trade] 回复 | 订单插入成功...订单引用:" << pInputOrder->OrderRef;
		SYNC_PRINT << "[Trade] Response | Insert Order Success...OrderRef:" << pInputOrder->OrderRef; 
		SYNC_PRINT << "[Trade] Inserting Order : " << CommonUtils::StringFromStruct(*pInputOrder);
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField *pInputOrderAction,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction){
		//SYNC_PRINT << "[Trade] 回复 | 撤单成功..."
		//	<< "交易所代码:" << pInputOrderAction->ExchangeID
		//	<< "报单编号:" << pInputOrderAction->OrderSysID;
		SYNC_DEBUG_LOG << "[Trade] Response | Cancell Order Success..."
			<< "ExchangeID:" << pInputOrderAction->ExchangeID
			<< "OrderSysID:" << pInputOrderAction->OrderSysID;
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
	m_OnCancelOrder_callback(pInputOrderAction, pRspInfo);
}

void CtpTradeSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	//SYNC_PRINT << "[Trade] 报单回报:前置编号FrontID:" << pOrder->FrontID << " 会话编号SessionID:" << pOrder->SessionID << " OrderRef:" << pOrder->OrderRef;
	SYNC_PRINT << "[Trade] Response | Insert_Order Received...";

	//if it fails to push, whichi mean it is duplicated order
	bool success = m_account_detail_mgr.pushImmediateOrder(*pOrder);
	if (success){
		SYNC_PRINT << "[Trade] FrontID:" << pOrder->FrontID << ", SessionID: " << pOrder->SessionID << ", OrderRef:" << pOrder->OrderRef;
		SYNC_PRINT << "[Trade] Order ID:" << pOrder->BrokerOrderSeq << ", OrderSubmitStatus:" << CommonUtils::InterpretOrderSubmitStatusCode(pOrder->OrderSubmitStatus);
		m_OnRtnOrder_callback(pOrder);
	}
}

void CtpTradeSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	SYNC_PRINT << "[Trade] Response | Order trade on:" << pTrade->TradeDate << "|" << pTrade->TradeTime << ", TradeID:" << pTrade->TradeID;
	
	m_account_detail_mgr.pushTodayNewTrade(*pTrade);//会更新整个账户和仓位的状态，使资金状态保持最新

	m_OnRtnTrade_callback(pTrade);
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
		SYNC_PRINT << "[Trade] Response | " << pRspInfo->ErrorMsg;
	}
	return ret;
}

///请求查询合约保证金率
void CtpTradeSpi::ReqQryInstrumentMarginRate(const char* instId)
{
	assert(instId);
	CThostFtdcQryInstrumentMarginRateField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);

	STRCPY(req.InstrumentID, instId);
	req.HedgeFlag = THOST_FTDC_HF_Speculation;

	int ret = pUserApi->ReqQryInstrumentMarginRate(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query MarginRate of Instrument(" << instId << ")..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;
}

//响应查询合约保证金率
void CtpTradeSpi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInstrumentMarginRate)
	{
		InstrumentManager.SetMarginRate(pInstrumentMarginRate->InstrumentID, *pInstrumentMarginRate);
	}
	else
	{
		SYNC_PRINT << "[Trade] Reponse | failed to obtain the margin rate field for " << pInstrumentMarginRate->InstrumentID;
	}

	m_stateChangeHandler.NotifyQueryEnd();
}

///请求查询合约手续费率
void CtpTradeSpi::ReqQryInstrumentCommissionRate(const char* instId)
{
	assert(instId);
	CThostFtdcQryInstrumentCommissionRateField req;

	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);

	STRCPY(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInstrumentCommissionRate(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query CommissionRate of Instrument(" << instId << ")..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;
}

///响应查询合约手续费率
void CtpTradeSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInstrumentCommissionRate)
	{
		InstrumentManager.SetCommissionRate(pInstrumentCommissionRate->InstrumentID, *pInstrumentCommissionRate);
	}
	else
	{
		SYNC_PRINT << "[Trade] Reponse | failed to obtain the commission rate field for " << pInstrumentCommissionRate->InstrumentID;
	}

	m_stateChangeHandler.NotifyQueryEnd();
}

///请求查询期权交易成本
void CtpTradeSpi::ReqQryOptionInstrTradeCost(const char* instId)
{
	assert(instId);
	CThostFtdcQryOptionInstrTradeCostField req;

	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	STRCPY(req.InstrumentID, instId);

	req.HedgeFlag = THOST_FTDC_HF_Speculation;
	int ret = pUserApi->ReqQryOptionInstrTradeCost(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query OptionInstrTradeCost of Instrument(" << instId << ")..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;
}

///响应查询期权交易成本
void CtpTradeSpi::OnRspQryOptionInstrTradeCost(CThostFtdcOptionInstrTradeCostField *pOptionInstrTradeCost, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pOptionInstrTradeCost)
	{
		//todo: store the Option InstrTradeCost
	}
	else
	{
		
	}
	if (bIsLast){
	}
}

///请求查询期权合约手续费
void CtpTradeSpi::ReqQryOptionInstrCommRate(const char* instId)
{
	assert(instId);
	CThostFtdcQryOptionInstrCommRateField req;

	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	STRCPY(req.InstrumentID, instId);

	int ret = pUserApi->ReqQryOptionInstrCommRate(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query OptionInstrCommRate of Instrument(" << instId << ")..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;
}

///响应查询期权合约手续费
void CtpTradeSpi::OnRspQryOptionInstrCommRate(CThostFtdcOptionInstrCommRateField *pOptionInstrCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pOptionInstrCommRate)
	{
		//todo: store the Option OptionInstrCommRate
	}
	else
	{
		
	}

	if (bIsLast){

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