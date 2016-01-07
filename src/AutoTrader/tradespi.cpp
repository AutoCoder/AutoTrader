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
	SYNC_PRINT << "[Trade] �ظ� | �ɹ�����...";
	m_stateChangeHandler.OnFrontConnected();
}

void CtpTradeSpi::OnFrontDisconnected(int nReason){
	SYNC_PRINT << "[Trade] �ظ� | �Ͽ�����..." << " reason=" << nReason;
}

void CtpTradeSpi::ReqUserLogin(){
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.UserID, m_userID);
	STRCPY(req.Password, m_password);
	STRCPY(req.UserProductInfo, m_productName);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);
	SYNC_PRINT << "[Trade] ���� | ��½ ..." << ((ret == 0) ? "�ɹ�" : "ʧ��");
}

void CtpTradeSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {

		m_frontID = pRspUserLogin->FrontID;
		m_sessionID = pRspUserLogin->SessionID;

		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		SPRINTF(m_orderRef, "%d", ++nextOrderRef);

		SYNC_PRINT << "[Trade] �ظ� | ��½�ɹ�...��ǰ����:" << pRspUserLogin->TradingDay << "| FrontID: " << m_frontID << "| SessionID: " << m_sessionID;

		m_stateChangeHandler.OnLogined();
	}
}

void CtpTradeSpi::ReqSettlementInfoConfirm(){
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++requestId);
	SYNC_PRINT << "[Trade] ���� | ���ͽ���ȷ�ϵ�..." << ((ret == 0) ? "�ɹ�" : "ʧ��");
}

void CtpTradeSpi::OnRspSettlementInfoConfirm( CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
		SYNC_PRINT << "[Trade] �ظ� | ����ȷ�ϳɹ�..." 
			<< pSettlementInfoConfirm->InvestorID
			<< "...<" << pSettlementInfoConfirm->ConfirmDate
			<< " " << pSettlementInfoConfirm->ConfirmTime << ">...ȷ��";
	}

	m_stateChangeHandler.OnConfirmedSettlementInfo();
}

void CtpTradeSpi::ReqQryOrder(){
	CThostFtdcQryOrderField req;
	memset(&req, 0, sizeof(req));

	STRCPY(req.InvestorID, m_userID);//Ͷ���ߴ���,Ҳ��userId
	int ret = pUserApi->ReqQryOrder(&req, ++requestId);

	SYNC_PRINT << "[Trade] ���� | ���Ͳ�ѯ����..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << " ret:" << ret; //retֵΪ-3��ʾÿ�뷢�����������������
}

///�����ѯ������Ӧ
void CtpTradeSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	SYNC_PRINT << "[Trade] �ظ� | ��ѯ����";

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
				SYNC_PRINT << "[Trade] ���к�Լ����������" << AP::GetManager().todayOrderCount();
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
			SYNC_PRINT << "[Trade] ��ѯ����������û�б���";
			m_stateChangeHandler.OnRspQryOrder();
		}

	}
}

void CtpTradeSpi::ReqQryTrade(){
	CThostFtdcQryTradeField req;
	memset(&req, 0, sizeof(req));

	STRCPY(req.InvestorID, m_userID);//Ͷ���ߴ���,Ҳ��userId
	int ret = pUserApi->ReqQryTrade(&req, ++requestId);

	SYNC_PRINT << "[Trade] ���� | ���ͳɽ���ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << " ret:" << ret;//retֵΪ-3��ʾÿ�뷢�����������������
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
				SYNC_PRINT << "�ɽ�������" << AP::GetManager().todayTradeCount();
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

	STRCPY(req.InvestorID, m_userID);//Ͷ���ߴ���,Ҳ��userId

	//strcpy(req.InstrumentID, "IF1402");
	int ret = pUserApi->ReqQryInvestorPositionDetail(&req, ++requestId);

	SYNC_PRINT << "[Trade] ���� | ����Ͷ���ֲ߳���ϸ��ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << " ret:" << ret;//retֵΪ-3��ʾÿ�뷢�����������������
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

			STRCPY(trade.InvestorID, pInvestorPositionDetail->InvestorID);///Ͷ���ߴ���
			STRCPY(trade.InstrumentID, pInvestorPositionDetail->InstrumentID);///��Լ����
			STRCPY(trade.ExchangeID, pInvestorPositionDetail->ExchangeID);///����������
			trade.Direction = pInvestorPositionDetail->Direction;///��������
			trade.Price = pInvestorPositionDetail->OpenPrice;///�۸�
			trade.Volume = pInvestorPositionDetail->Volume;///����
			STRCPY(trade.TradeDate, pInvestorPositionDetail->OpenDate);///�ɽ�ʱ��


			if (pInvestorPositionDetail->Volume > 0)//ɸѡδƽ�ֵ�
			{
				if (trade.Direction == '0')
					AP::GetManager().pushYesterdayUnClosedTrade(trade, AP::Long);
				else if (trade.Direction == '1')
					AP::GetManager().pushYesterdayUnClosedTrade(trade, AP::Short);
			}


			//������к�Լ�ĳֲ���ϸ��Ҫ����߽�����һ���Ĳ�ѯReqQryTradingAccount();
			if (bIsLast)
			{
				m_firstquery_Detail = false;

				SYNC_PRINT << "[Trade]�˻����к�Լδƽ�ֵ�����(��������,һ�ʿɶ�Ӧ����):�൥" << AP::GetManager().yesterdayUnClosedTradeCount(AP::Long) << " �յ�" << AP::GetManager().yesterdayUnClosedTradeCount(AP::Short);
				SYNC_PRINT << "--------�ȶ���-------";

				SYNC_PRINT << AP::GetManager().yesterdayUnClosedTradeToString(AP::Long);
				SYNC_PRINT << AP::GetManager().yesterdayUnClosedTradeToString(AP::Short);

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
			SYNC_PRINT << "[Trade] ��ѯͶ���ֲ߳���ϸ������û��Ͷ���ֲ߳���ϸ";
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
			SYNC_PRINT << "[Trade] ���� | �����˻���ѯ...�ɹ�";
			break;
		}
		SYNC_PRINT << "[Trade] ���� | �����˻���ѯ...ʧ��, ret:" << ret;
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

			SYNC_PRINT << "[Trade] �ʽ��ѯ��������ѯͶ���ֲ߳�:";

			m_stateChangeHandler.OnRspQryTradingAccount();
		}
	}
	else{
		if (m_firstquery_TradingAccount){
			m_firstquery_TradingAccount = false;


			SYNC_PRINT << " [Trade] �ʽ��ѯ����,��ѯͶ���ֲ߳�:";
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
	SYNC_PRINT << "[Trade] ���� | ���ͳֲֲ�ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��");
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


//�ֲֲ�ѯ�ص�����,�Ѿ�ƽ�ֵĵ��ӣ��ֲ���Ϊ0�˻��᷵��
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
			SYNC_PRINT << "[Trade] ��ѯ�ֲֳ�����û�гֲ�";
			m_stateChangeHandler.OnRspQryInvestorPosition();
		}
	}
}

void CtpTradeSpi::ReqQryInstrument_all(){
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	int ret = pUserApi->ReqQryInstrument(&req, ++requestId);
	SYNC_PRINT << "[Trade] ���� | ���ͺ�Լ��ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << " ret:" << ret;
}

void CtpTradeSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInstrument(&req, ++requestId);
	SYNC_PRINT << "[Trade] ���� | ���ͺ�Լ��ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��");
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
				SYNC_PRINT << "[Trade] ���гֲֺ�Լ��" << AP::GetManager().getInstrumentList();

				// Ϊʲô��tradespi�̳߳�ʼ��ȫ������Ժ������MD?
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
		SYNC_PRINT << "[Trade] �ظ� | ��������ɹ�...��������:" << pInputOrder->OrderRef;
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
	SYNC_PRINT << "[Trade] ���� | ��������..." << ((ret == 0) ? "�ɹ�" : "ʧ��");
}

void CtpTradeSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField *pInputOrderAction,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction){
		SYNC_PRINT << "[Trade] �ظ� | �����ɹ�..."
			<< "����������:" << pInputOrderAction->ExchangeID
			<< "�������:" << pInputOrderAction->OrderSysID;
	}
	//if (bIsLast) SetEvent(g_tradehEvent);
}

void CtpTradeSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	SYNC_PRINT << "[Trade] �����ر�:ǰ�ñ��FrontID:" << pOrder->FrontID << " �Ự���SessionID:" << pOrder->SessionID << " OrderRef:" << pOrder->OrderRef;

	AP::GetManager().pushImmediateOrder(*pOrder);

	SYNC_PRINT << "[Trade] �ظ� | ����...ID:" << pOrder->BrokerOrderSeq << ";����״̬:" << CommonUtils::InterpretOrderSubmitStatusCode(pOrder->OrderSubmitStatus);
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
	SYNC_PRINT << "[Trade] �ظ� | �����ɽ���" << pTrade->TradeDate << pTrade->TradeTime << "...TradeID:" << pTrade->TradeID;

	////fresh accout
	//SYNC_PRINT << "[Trade] Order executed. begin to refresh Account info...";

	//if (pAccountMgr->isReady()) // this check means that the history trade before account query will not record
	//	Position::GetManager().PushTradeItem(*pTrade);
	//ReqQryTradingAccount(); //depreted: calulate locally instead

	//sync the order execute process. Notify that the order trade process is finished.
	//if (g_OrderRunMtx.try_lock())
	//	g_OrderRunMtx.unlock();
	//g_OrderRunMtx.unlock();
	
	AP::GetManager().pushTodayNewTrade(*pTrade);//����������˻��Ͳ�λ��״̬��ʹ�ʽ�״̬��������
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
		SYNC_PRINT << "[Trade] �ظ� | " << pRspInfo->ErrorMsg;
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
		SYNC_PRINT << "[Debug] ���붩��:" << CommonUtils::StringFromStruct(ordstruct);
		int ret = pUserApi->ReqOrderInsert(&ordstruct, ++requestId);
		SYNC_PRINT << "[Trade] ���� | ���붩��..." << ((ret == 0) ? "�ɹ�" : "ʧ��");
		//if (ret)
		//	g_OrderRunMtx.unlock();
	}
	else{
		SYNC_PRINT << "[Trade] Invalid OrderField construct";
	}
}

void CtpTradeSpi::ForceClose(){
	TThostFtdcInstrumentIDType    instId;//��Լ
	TThostFtdcDirectionType       dir;//����,'0'��'1'��
	TThostFtdcCombOffsetFlagType  kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
	TThostFtdcPriceType           price;//�۸�0���м�,��������֧��
	TThostFtdcVolumeType          vol;//����

	for (auto item : AP::GetManager().getAllPositionMap()){
		//ƽ��
		if (item.second.Holding_long > 0)
		{
			STRCPY(instId, item.second.InstId.c_str());
			dir = THOST_FTDC_D_Sell;// #define THOST_FTDC_D_Buy '0' ||| #define THOST_FTDC_D_Sell '1'
			price = item.second.LastPrice - 5 * AP::GetManager().getInstrumentField(instId).PriceTick;

			//������
			if (strcmp(AP::GetManager().getInstrumentField(instId).ExchangeID, "SHFE") == 0)
			{
				if (item.second.YdPosition_long == 0)//û�����
				{
					SYNC_PRINT << "[Trade] �൥������ ȫ��ƽ��";;

					STRCPY(kpp, "3");//ƽ��
					vol = item.second.Holding_long;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				else if (item.second.TodayPosition_long == 0)//û�н��
				{
					SYNC_PRINT << "[Trade] �൥������ ȫ��ƽ��";

					STRCPY(kpp, "1");//ƽ��
					vol = item.second.Holding_long;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				//ͬʱ������ֺͽ��
				else if (item.second.YdPosition_long > 0 && item.second.TodayPosition_long > 0)
				{
					SYNC_PRINT << "[Trade] �൥������ͬʱ ƽ��ƽ��";

					STRCPY(kpp, "3");//ƽ��
					vol = item.second.TodayPosition_long;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

					STRCPY(kpp, "1");//ƽ��
					vol = item.second.YdPosition_long;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}

			}
			//��������
			else
			{
				SYNC_PRINT << "[Trade] ���������൥ ƽ��[��֧��ƽ��]";

				STRCPY(kpp, "1");
				vol = item.second.Holding_long;
				ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

			}
		}

		//ƽ��
		if (item.second.Holding_short > 0)
		{
			STRCPY(instId, item.second.InstId.c_str());//��strcpy(instId, iter->first.c_str());
			dir = '0';
			price = item.second.LastPrice + 5 * AP::GetManager().getInstrumentField(instId).PriceTick;

			//������
			if (strcmp(AP::GetManager().getInstrumentField(instId).ExchangeID, "SHFE") == 0)
			{
				if (item.second.YdPosition_short == 0)//û�����
				{
					SYNC_PRINT << "[Trade] �յ������� ȫ��ƽ��";

					STRCPY(kpp, "3");//ƽ��
					vol = item.second.Holding_short;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				else if (item.second.TodayPosition_short == 0)//û�н��
				{
					SYNC_PRINT << "[Trade] �յ������� ȫ��ƽ��";

					STRCPY(kpp, "1");//ƽ��
					vol = item.second.Holding_short;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
				//ͬʱ������ֺͽ��
				else if (item.second.YdPosition_short > 0 && item.second.TodayPosition_short > 0)
				{
					SYNC_PRINT << "[Trade] �յ������� ͬʱƽ��ƽ��";

					STRCPY(kpp, "3");//ƽ��
					vol = item.second.TodayPosition_short;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

					STRCPY(kpp, "1");//ƽ��
					vol = item.second.YdPosition_short;
					ReqOrderInsert(Order(item.second.InstId, price, vol, dir, kpp));

				}
			}
			//��������
			else
			{
				SYNC_PRINT << "[Trade] ���������յ� ƽ��[��֧��ƽ��]";

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
			// ����aliveDuration(6��)δ�ɽ�������
			if (CommonUtils::TimeToSenconds(item.InsertTime) + aliveDuration < CommonUtils::TimeToSenconds(MDtime.c_str()) )
			{
				SYNC_PRINT << "[Trade] ����...����ID:" << item.BrokerOrderSeq;
				ReqOrderAction(item);
			}

		}
	}
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