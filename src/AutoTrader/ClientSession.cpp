#include "stdafx.h"
#include <functional>
#include <algorithm>
#include "spdlog/spdlog.h"

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
#include "crossplatform.h"

#ifdef FAKE_MD
#include<ctime>
#endif

ClientSession::ClientSession(const std::string& userId, const std::shared_ptr<Transmission::socket_session>& s)
: BaseClientSession(userId) 
, m_session(s)
{
	Init_CTP();
}

ClientSession::~ClientSession()
{
}

bool ClientSession::Init_CTP()
{
	m_trade_api = CThostFtdcTraderApi::CreateFtdcTraderApi();
	assert(m_trade_api);

	Account::Meta meta = Account::Manager::Instance().GetMeta(m_userId);
	InitedAccountCallback accountInitFinished_Callback = std::bind(&ClientSession::OnAccountInitFinished, this);
	RtnOrderCallback onRtnOrder_Callback = std::bind(&ClientSession::OnRtnOrder, this, std::placeholders::_1);
	RtnTradeCallback OnRtnTrade_Callback = std::bind(&ClientSession::OnRtnTrade, this, std::placeholders::_1);
	CancelOrderCallback OnCancelOrder_Callback = std::bind(&ClientSession::OnCancelOrder, this, std::placeholders::_1, std::placeholders::_2);

	m_trade_spi = new CtpTradeSpi(m_trade_api, meta.m_BrokerId.c_str(), meta.m_UserId.c_str(), meta.m_Password.c_str(), \
		Config::Instance()->ProductName().c_str(), *(m_detailMgr.get()), \
		accountInitFinished_Callback, onRtnOrder_Callback, OnRtnTrade_Callback, OnCancelOrder_Callback);

	m_trade_api->RegisterSpi((CThostFtdcTraderSpi*)m_trade_spi);
	m_trade_api->SubscribePublicTopic(THOST_TERT_RESTART);
	m_trade_api->SubscribePrivateTopic(THOST_TERT_RESTART);
	m_trade_api->RegisterFront(const_cast<char*>(Config::Instance()->CtpTradeFront().c_str()));
	m_trade_api->Init();
	SYNC_LOG << "CTP Account of " << m_userId << "...Setup";

	return true;
}

bool ClientSession::StartTrade(const std::string& instru, const std::string& strategyName, ErrorCode& errcode){
#ifdef FAKE_MD
	if (m_fakeMdThreadF.valid()==false || m_fakeMdThreadF.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
		m_fakeMdThreadF = std::async(std::launch::async, std::bind(&ClientSession::ReturnFakeCTPMessage, this));
	else if (m_fakeMdThreadF.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
		bool toRemoved = true;
#else 
	return BaseClientSession::StartTrade(instru, strategyName, errcode);
#endif
}

#ifdef FAKE_MD
bool ClientSession::ReturnFakeCTPMessage(){

	int volume= 0;
	time_t now_time;
	int openPrice = 2000 + rand() % 20;
	int highestPrice = openPrice;
	int lowestPrice = openPrice;
	int idx = 0;
	int OrderSysID = 1;
	int direction = 0;
	while (m_isTrading.load()){
		idx++;
		now_time = time(NULL);
		int lastPrice = 2000 + rand() % 20;
		if (highestPrice < lastPrice)
			highestPrice = lastPrice;
		if (lowestPrice > lastPrice)
			lowestPrice = lastPrice;
		volume = rand() % 100; 
		Transmission::Utils::SendMDInfo(m_session, openPrice, lastPrice, highestPrice, lowestPrice, volume, now_time*2);

		direction = rand() & 1;  // 0 or 1 randomly
		OrderSysID++;
		char orderId[256] = { 0 };
		SPRINTF(orderId, "%d", OrderSysID);
		if (idx % 10 == 0){
			Transmission::Utils::SendDealInfo(m_session, Transmission::INSERT_ORDER, direction, lastPrice, volume, orderId, now_time * 2);
			if (idx % 15 == 0)
				Transmission::Utils::SendDealInfo(m_session, Transmission::TRADE, direction, lastPrice, volume, orderId, now_time * 2 + 2);
			else
				Transmission::Utils::SendDealInfo(m_session, Transmission::CANCELL_ORDER, direction, lastPrice, volume, orderId, now_time * 2 + 2);
		}
		sleep(500);
	}
	return false;
}
#endif

void ClientSession::SendTickToClient(const TickWrapper& tick){
	if (m_total_vol != 0)
		Transmission::Utils::SendMDInfo(m_session, tick.OpenPrice(), tick.LastPrice(), tick.HighestPrice(), tick.LowestPrice(), tick.Volume() - m_total_vol, tick.toTimeStamp());
	
	m_total_vol = tick.Volume();
}

void ClientSession::SendTechToClient(const TickWrapper& tick){
	if (auto tech_ptr = tick.GetTechVec())
		Transmission::Utils::SendTechInfo(m_session, tech_ptr->ToJson(), tick.toTimeStamp());
}

void ClientSession::SendPostionInfoToClient(){
	double posMoney = 0.0;
	double balance = 0.0;
	AP::Direction posDirection = AP::Long;
	m_detailMgr->getPosition(posMoney, posDirection, balance);
	std::string instru = m_detailMgr->getInstrumentList();
	int todayPos = 0;    AP::Direction todayDirection = AP::Long;
	int ydPos = 0;       AP::Direction ydDirection = AP::Long;

	int posVol = m_detailMgr->getPositionVolume(instru, todayDirection, todayPos, ydDirection, ydPos);
	Transmission::Utils::SendPositionInfo(m_session, balance, posMoney, m_detailMgr->getPositionOfInstruments());
}

void ClientSession::OnAccountInitFinished(){
	m_PositionInfo_ready = true;
	SendPostionInfoToClient();
}

void ClientSession::OnRtnOrder(CThostFtdcOrderField* pOrder){
	BaseClientSession::OnRtnOrder(pOrder);
	
	long long timeStamp = CommonUtils::DateTimeToTimestamp(pOrder->InsertDate, pOrder->InsertTime) * 2;
	Transmission::Utils::SendDealInfo(m_session, Transmission::INSERT_ORDER, pOrder->InstrumentID ,pOrder->Direction, pOrder->CombOffsetFlag[0], pOrder->LimitPrice, pOrder->VolumeTotalOriginal, pOrder->OrderRef, timeStamp);
}

void ClientSession::OnRtnTrade(CThostFtdcTradeField* pTrade){
	BaseClientSession::OnRtnTrade(pTrade);

	long long timeStamp = CommonUtils::DateTimeToTimestamp(pTrade->TradeDate, pTrade->TradeTime) * 2;
	Transmission::Utils::SendDealInfo(m_session, Transmission::TRADE, pTrade->InstrumentID, pTrade->Direction, pTrade->OffsetFlag, pTrade->Price, pTrade->Volume, pTrade->OrderRef, timeStamp);
}

void ClientSession::OnCancelOrder(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo){
	Transmission::Utils::SendDealInfo(m_session, Transmission::CANCELL_ORDER, pInputOrderAction->InstrumentID, 0, 0, 0, 0, pInputOrderAction->OrderRef, 0);
}

void ClientSession::OnLoginRequest()
{
	//if the ClientSession is created which mean login already, just send Account Info back.
	const std::vector<std::string>& instu = Account::Manager::Instance().GetMeta(m_userId).m_Instruments;
	const std::vector<std::string>& strategies = TriggerFactory::Instance()->GetStrategies(m_userId);
	Transmission::Utils::SendAccountInfo(m_session, instu, strategies, IsTrading(), RunningInstrument(), RunningStrategy());
}

void ClientSession::OnStartTradeRequest(const std::string& instru, const std::string& strategyName)
{
	if (IsTrading()){
		Transmission::Utils::SendStartTradeResultInfo(m_session, Transmission::TradingNow);
	}
	else{
		ErrorCode err_code;
		if (StartTrade(instru, strategyName, err_code)){
			Transmission::Utils::SendStartTradeResultInfo(m_session, Transmission::Succeed);
		}
		else{
			Transmission::Utils::SendStartTradeResultInfo(m_session, (Transmission::ErrorCode)err_code);
		}
	}
}
