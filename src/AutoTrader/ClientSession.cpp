#include "stdafx.h"
#include <functional>

#include "ThostFtdcTraderApi.h"
#include "ClientSession.h"
#include "ClientSessionMgr.h"
#include "AccountMgr.h"

#include "TriggerFactory.h"
#include "RealTimeDataProcessor.h"
#include "RealTimeDataProcessorPool.h"
#include "Order.h"
#include "tradespi.h"
#include "OrderTrigger.h"
#include "AP_Mgr.h"
#include "Config.h"
#include "TickWrapper.h"
#include "Transmission.h"
#include "Utils.h"
#include "CommonUtils.h"

ClientSession::ClientSession(const std::string& userId, const std::shared_ptr<Transmission::socket_session>& s, CThostFtdcTraderApi* api)
: m_userId(userId)
, m_session(s)
, m_detailMgr(std::make_unique<AP::AccountDetailMgr>())
{
	assert(api);
	Account::Meta meta = Account::Manager::Instance().GetMeta(m_userId);
	InitedAccountCallback accountInitFinished_Callback = std::bind(&ClientSession::OnAccountInitFinished, this);
	RtnOrderCallback onRtnOrder_Callback = std::bind(&ClientSession::OnRtnOrder, this, std::placeholders::_1);
	RtnTradeCallback OnRtnTrade_Callback = std::bind(&ClientSession::OnRtnTrade, this, std::placeholders::_1);
	CancelOrderCallback OnCancelOrder_Callback = std::bind(&ClientSession::OnCancelOrder, this, std::placeholders::_1, std::placeholders::_2);

	m_trade_spi = std::make_unique<CtpTradeSpi>(api, meta.m_BrokerId.c_str(), meta.m_UserId.c_str(), meta.m_Password.c_str(), \
		Config::Instance()->ProductName().c_str(), *(m_detailMgr.get()), \
		accountInitFinished_Callback, onRtnOrder_Callback, OnRtnTrade_Callback, OnCancelOrder_Callback);

	api->RegisterSpi((CThostFtdcTraderSpi*)(m_trade_spi.get()));
	api->SubscribePublicTopic(THOST_TERT_RESTART);
	api->SubscribePrivateTopic(THOST_TERT_RESTART);
	api->RegisterFront(const_cast<char*>(Config::Instance()->CtpTradeFront().c_str()));
	api->Init();
}

ClientSession::~ClientSession()
{
}

//may access by mdThread and m_exeOrderThread
bool ClientSession::AppendOrder(const Order& order){
	std::lock_guard<std::mutex> lk(m_mtx);
	if (m_pending_order.get()){
		return false;
	}
	else{
		//m_pending_order.swap(std::make_unique<Order>(order));
		m_pending_order.reset(new Order(order));
		m_con.notify_all();
		return true;
	}
}

//may access by mdThread and m_exeOrderThread
void ClientSession::WaitAndPopCurrentOrder(Order& ord){

	std::unique_lock<std::mutex> lk(m_mtx);

	m_con.wait(lk, [this]{return m_pending_order.get(); });
	ord = *(m_pending_order.get());
	m_pending_order.reset();
}

void ClientSession::ExecutePendingOrder(){
	while (m_isTrading){
		Order ord;
		WaitAndPopCurrentOrder(ord);

		m_trade_spi->CancelOrder(ord.GetTriggerTick(), 6, ord.GetInstrumentId());
		m_trade_spi->ReqOrderInsert(ord);
	}
}

bool ClientSession::StartTrade(const std::string& instru, const std::string& strategyName, Transmission::ErrorCode& errcode){
	if (m_isTrading){
		errcode = Transmission::TradingNow;
		return false;
	}

	Account::Meta meta = Account::Manager::Instance().GetMeta(m_userId);

	if (std::find(meta.m_Instruments.begin(), meta.m_Instruments.end(), instru) != meta.m_Instruments.end()){
		auto strategyPtr = TriggerFactory::Instance()->GetTrigger(m_userId, strategyName);
		if (strategyPtr){
			strategyPtr->BindWithAccount(m_detailMgr.get());
			m_realtimedata_processor = std::make_shared<RealTimeDataProcessor>(strategyPtr, instru, this);
			RealTimeDataProcessorPool::getInstance()->AddProcessor(m_realtimedata_processor);
			m_isTrading.store(true);

			// This will start the thread. Notice move semantics!
			m_exeOrderThread = std::thread(&ClientSession::ExecutePendingOrder, this);
#ifdef FAKE_MD
			m_fakeMDThread = std::thread(&ClientSession::ReturnMDFakeTick, this);
#endif
			return true;
		}
		else{
			errcode = Transmission::InvalidTradeArguments;
			return false;
		}
	}
	else{

	}
}
#ifdef FAKE_MD
void ClientSession::ReturnMDFakeTick(){
	while (m_isTrading){
		CThostFtdcDepthMarketDataField odata;
		STRCPY(odata.TradingDay, "20160214");
		STRCPY(odata.InstrumentID, "rb1605");
		STRCPY(odata.UpdateTime, "10:06:00");
		odata.UpdateMillisec = 0;
		odata.OpenPrice = 2650 + rand() % 20;
		odata.ClosePrice = 2650 + rand() % 20;
		odata.HighestPrice = odata.OpenPrice > odata.ClosePrice ? odata.OpenPrice : odata.ClosePrice;
		odata.LowestPrice = odata.OpenPrice > odata.ClosePrice ? odata.ClosePrice : odata.OpenPrice;
		odata.Volume = rand() % 20; 
		TickWrapper info(&odata);
		this->InformClientViewer(info);
		sleep(500);
	}
}
#endif

void ClientSession::InformClientViewer(const TickWrapper& tick){
	Transmission::Utils::SendMDInfo(m_session, tick.LastPrice(), tick.LastPrice(), tick.LastPrice(), tick.LastPrice(), tick.Volume(), tick.toTimeStamp());
}

void ClientSession::StopTrade(){
	if (!m_isTrading.load())
		return;
	m_isTrading.store(false);
}

void ClientSession::OnAccountInitFinished(){
	double posMoney = 0.0;
	double balance = 0.0;
	AP::Direction posDirection = AP::Long;
	m_detailMgr->getPosition(posMoney, posDirection, balance);
	std::string instru = m_detailMgr->getInstrumentList();
	int todayPos = 0;    AP::Direction todayDirection = AP::Long;
	int ydPos = 0;       AP::Direction ydDirection = AP::Long;
	
	int posVol = m_detailMgr->getPositionVolume(instru, todayDirection, todayPos, ydDirection, ydPos);
	Transmission::Utils::SendAccountStatus(m_session, balance, posVol, m_detailMgr->getInstrumentList(), (int)(posMoney / posVol));
}

void ClientSession::OnRtnOrder(CThostFtdcOrderField* pOrder){
	long long timeStamp = CommonUtils::DateTimeToTimestamp(pOrder->InsertDate, pOrder->InsertTime) * 2;
	Transmission::Utils::SendDealInfo(m_session, Transmission::INSERT_ORDER, pOrder->Direction, pOrder->LimitPrice, pOrder->VolumeTotalOriginal, pOrder->OrderSysID, timeStamp);
}

void ClientSession::OnRtnTrade(CThostFtdcTradeField* pTrade){
	long long timeStamp = CommonUtils::DateTimeToTimestamp(pTrade->TradeDate, pTrade->TradeTime) * 2;
	Transmission::Utils::SendDealInfo(m_session, Transmission::INSERT_ORDER, pTrade->Direction, pTrade->Price, pTrade->Volume, pTrade->OrderSysID, timeStamp);
}

void ClientSession::OnCancelOrder(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo){

}

void ClientSession::OnLoginRequest()
{
	//if the ClientSession is created which mean login already, just send Account Info back.
	const std::vector<std::string>& instu = Account::Manager::Instance().GetMeta(m_userId).m_Instruments;
	const std::vector<std::string>& strategies = TriggerFactory::Instance()->GetStrategies(m_userId);
	Transmission::Utils::SendAccountInfo(m_session, instu, strategies);
}

void ClientSession::OnStartTradeRequest(const std::string& instru, const std::string& strategyName)
{
	if (IsTrading()){
		Transmission::Utils::SendStartTradeResultInfo(m_session, Transmission::TradingNow);
	}
	else{
		Transmission::ErrorCode err_code;
		if (StartTrade(instru, strategyName, err_code)){
			Transmission::Utils::SendStartTradeResultInfo(m_session, Transmission::Succeed);
		}
		else{
			Transmission::Utils::SendStartTradeResultInfo(m_session, err_code);
		}
	}
}