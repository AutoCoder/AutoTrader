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

	strcpy_s(req.InvestorID, m_userID);//Ͷ���ߴ���,Ҳ��userId
	int ret = pUserApi->ReqQryOrder(&req, ++requestId);

	SYNC_PRINT << " ���� | ���Ͳ�ѯ����..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << " ret:" << ret; //retֵΪ-3��ʾÿ�뷢�����������������
}

///�����ѯ������Ӧ
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
				SYNC_PRINT << "���к�Լ����������" << AP::GetManager().TodayOrderCount();
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
			SYNC_PRINT << "��ѯ����������û�б���";
			m_stateChangeHandler.OnRspQryOrder();
		}

	}
}

void CtpTradeSpi::ReqQryTrade(){
	CThostFtdcQryTradeField req;
	memset(&req, 0, sizeof(req));

	strcpy_s(req.InvestorID, m_userID);//Ͷ���ߴ���,Ҳ��userId

	int ret = pUserApi->ReqQryTrade(&req, ++requestId);

	SYNC_PRINT << " ���� | ���ͳɽ���ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << " ret:" << ret;//retֵΪ-3��ʾÿ�뷢�����������������
}

//�����ѯ�ɽ���Ӧ,Ҫ���ֳ�������ʱ��һ�β�ѯ��֮��Ĳ�ѯ
//ֻ�ܲ�ѯ����ģ���ֲ��ܣ����Ի�Ҫ��ѯ�ֲ���ϸOnRspQryInvestorPositionDetail()
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
				SYNC_PRINT << "�ɽ�������" << AP::GetManager().TodayTradeCount();
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

	strcpy_s(req.InvestorID, m_userID);//Ͷ���ߴ���,Ҳ��userId

	//strcpy(req.InstrumentID, "IF1402");

	int ret = pUserApi->ReqQryInvestorPositionDetail(&req, ++requestId);

	SYNC_PRINT << " ���� | ����Ͷ���ֲ߳���ϸ��ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << " ret:" << ret;//retֵΪ-3��ʾÿ�뷢�����������������
}


void CtpTradeSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPositionDetail)
	{
		//��ʱ���Ⱥ����򷵻أ�����Լ����

		//���к�Լ
		if (m_firstquery_Detail == true)
		{
			//�������к�Լ����������ƽ�ֵģ�ֻ����δƽ�ֵ�
			//����������ǰ�ĳֲּ�¼���浽δƽ������tradeList_notClosed_account_long��tradeList_notClosed_account_short
			//Ϊʲô���ڲ�ѯ�ɽ��ص�����OnRspQryTrade()������,��Ϊֻ�ܲ�ѯ�����
			//ʹ�ýṹ����CThostFtdcTradeField����Ϊ����ʱ���ֶΣ���CThostFtdcInvestorPositionDetailFieldû��ʱ���ֶ�

			CThostFtdcTradeField trade;

			strcpy_s(trade.InvestorID, pInvestorPositionDetail->InvestorID);///Ͷ���ߴ���
			strcpy_s(trade.InstrumentID, pInvestorPositionDetail->InstrumentID);///��Լ����
			strcpy_s(trade.ExchangeID, pInvestorPositionDetail->ExchangeID);///����������
			trade.Direction = pInvestorPositionDetail->Direction;///��������
			trade.Price = pInvestorPositionDetail->OpenPrice;///�۸�
			trade.Volume = pInvestorPositionDetail->Volume;///����
			strcpy_s(trade.TradeDate, pInvestorPositionDetail->OpenDate);///�ɽ�ʱ��


			if (pInvestorPositionDetail->Volume > 0)//ɸѡδƽ�ֵ�
			{
				if (trade.Direction == '0')
					AP::GetManager().pushYesterdayUnClosedTrade(trade, AP::Buy);
				else if (trade.Direction == '1')
					AP::GetManager().pushYesterdayUnClosedTrade(trade, AP::Sell);
			}


			//������к�Լ�ĳֲ���ϸ��Ҫ����߽�����һ���Ĳ�ѯReqQryTradingAccount();
			if (bIsLast)
			{
				m_firstquery_Detail = false;

				SYNC_PRINT << "�˻����к�Լδƽ�ֵ�����(��������,һ�ʿɶ�Ӧ����):�൥" << AP::GetManager().yesterdayUnClosedTradeCount(AP::Buy) << " �յ�" << AP::GetManager().yesterdayUnClosedTradeCount(AP::Sell);
				SYNC_PRINT << "--------�ȶ���-------";

				SYNC_PRINT << AP::GetManager().YesterdayUnClosedTradeToString(AP::Buy);
				SYNC_PRINT << AP::GetManager().YesterdayUnClosedTradeToString(AP::Sell);

				SYNC_PRINT << "--------����-------";

				SYNC_PRINT << "��ѯͶ���ֲ߳���ϸ����";
				m_stateChangeHandler.OnRspQryInvestorPositionDetail();

			}
		}
	}
	else
	{
		if (m_firstquery_Detail == true)
		{
			m_firstquery_Detail = false;
			SYNC_PRINT << "��ѯͶ���ֲ߳���ϸ������û��Ͷ���ֲ߳���ϸ";
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
/////TFtdcTimeConditionType��һ����Ч����������
///////////////////////////////////////////////////////////////////////////
/////������ɣ�������
//#define THOST_FTDC_TC_IOC '1'
/////������Ч
//#define THOST_FTDC_TC_GFS '2'
/////������Ч
//#define THOST_FTDC_TC_GFD '3'
/////ָ������ǰ��Ч
//#define THOST_FTDC_TC_GTD '4'
/////����ǰ��Ч
//#define THOST_FTDC_TC_GTC '5'
/////���Ͼ�����Ч
//#define THOST_FTDC_TC_GFA '6'
//
/////TFtdcVolumeConditionType��һ���ɽ�����������
///////////////////////////////////////////////////////////////////////////
/////�κ�����
//#define THOST_FTDC_VC_AV '1'
/////��С����
//#define THOST_FTDC_VC_MV '2'
/////ȫ������
//#define THOST_FTDC_VC_CV '3'

//����ȫ���ɽ������Զ�����ָ��(FOKָ��)��ָ���޶���λ�´�ָ������ָ���������걨����δ��ȫ���ɽ�����ָ���������걨�����Զ���ϵͳ������
//
//�����ɽ�ʣ��ָ���Զ�����ָ��(FAKָ��)

//
//#define THOST_FTDC_D_Buy '0'
//	///��
//#define THOST_FTDC_D_Sell '1
//
///////////////////////////////////////////////////////////////////////////
/////TFtdcOffsetFlagType��һ����ƽ��־����
///////////////////////////////////////////////////////////////////////////
/////����
//#define THOST_FTDC_OF_Open '0'
/////ƽ��
//#define THOST_FTDC_OF_Close '1'
/////ǿƽ
//#define THOST_FTDC_OF_ForceClose '2'
/////ƽ��
//#define THOST_FTDC_OF_CloseToday '3'
/////ƽ��
//#define THOST_FTDC_OF_CloseYesterday '4'
/////ǿ��
//#define THOST_FTDC_OF_ForceOff '5'
/////����ǿƽ
//#define THOST_FTDC_OF_LocalForceClose '6'
//
//typedef char TThostFtdcOffsetFlagType;