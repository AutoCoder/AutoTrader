#include "stdafx.h"
#include <functional>
#include <algorithm>
#include "json/json.h"
#include "spdlog/spdlog.h"

#include "ThostFtdcTraderApi.h"
#include "ClientSession.h"
#include "ClientSessionMgr.h"
#include "AccountMgr.h"

#include "TriggerFactory.h"
#include "MdProcessor.h"
#include "MdProcessorPool.h"
#include "Order.h"
#include "tradespi.h"
#include "OrderTrigger.h"
#include "AP_Mgr.h"
#include "PPMgr.h"
#include "Config.h"
#include "TickWrapper.h"
#include "Transmission.h"
#include "Utils.h"
#include "CommonUtils.h"
#include "crossplatform.h"

#ifdef FAKE_MD
#include<ctime>
#endif

ClientSession::ClientSession(const std::string& userId, const SockSessionSP& s)
: BaseClientSession(userId)
, m_tickoff(false)
, m_session(s)
{
#ifndef FAKE_MD
	Init_CTP();
#endif
}

ClientSession::~ClientSession()
{
	SYNC_DEBUG_LOG << "PositionInfo(AfterTrade)" << m_PPMgr->ToString();
	DEBUG_LOGGER->flush();

	if (m_trade_api)
	{
		m_trade_api->RegisterSpi(NULL);
		m_trade_api->Release();
		m_trade_api = NULL;
	}
	if (m_trade_spi)
	{
		delete m_trade_spi;
		m_trade_spi = NULL;
	}
	SYNC_LOG << "CTP Account of " << m_userId << "...Logout";
}

bool ClientSession::Init_CTP()
{
	m_trade_api = CThostFtdcTraderApi::CreateFtdcTraderApi();
	assert(m_trade_api);

	Account::Meta meta = Account::Manager::Instance().GetMeta(m_userId);

	InitedAccountCallback onInitedAccount_Callback = std::bind(&ClientSession::OnAccountInitFinished, this);
	RtnOrderCallback onRtnOrder_Callback = std::bind(&ClientSession::OnRtnOrder, this, std::placeholders::_1);
	RtnTradeCallback OnRtnTrade_Callback = std::bind(&ClientSession::OnRtnTrade, this, std::placeholders::_1);
	CancelOrderCallback OnCancelOrder_Callback = std::bind(&ClientSession::OnCancelOrder, this, std::placeholders::_1, std::placeholders::_2);

	m_trade_spi = new CtpTradeSpi(m_trade_api, meta.m_BrokerId.c_str(), meta.m_UserId.c_str(), meta.m_Password.c_str(), \
		Config::Instance()->ProductName().c_str(), *(m_PPMgr.get()), \
		onInitedAccount_Callback, onRtnOrder_Callback, OnRtnTrade_Callback, OnCancelOrder_Callback);

	m_trade_api->RegisterSpi((CThostFtdcTraderSpi*)m_trade_spi); 
	m_trade_api->SubscribePublicTopic(THOST_TERT_RESTART);
	m_trade_api->SubscribePrivateTopic(THOST_TERT_RESTART);
	m_trade_api->RegisterFront(const_cast<char*>(Config::Instance()->CtpTradeFront().c_str()));
	m_trade_api->Init();
	SYNC_LOG << "CTP Account of " << m_userId << "...Setup";

	return true;
}

void ClientSession::OnOrderTrigger(const Order& ord){
	if (!m_isTrading.load())
		return;

	if (m_semiAuto){
		Transmission::Utils::SendOrderPrompt(m_session, ord.GetInstrumentId(), ord.GetExchangeDirection(), ord.GetCombOffsetFlagType(), ord.GetRefExchangePrice(), ord.GetVolume(), ord.GetTriggerTick());
	}
	else{
		if (m_isTrading.load()){ //Check this bool var again, prevent to execute new pushed order after user stop trade.
			m_trade_spi->CancelOrder(ord.GetTriggerTick(), 0, ord.GetInstrumentId());
			m_trade_spi->ReqOrderInsert(ord);
		}
	}
}

bool ClientSession::StartTrade(const std::string& instru, const std::string& strategyName, ErrorCode& errcode){
#ifdef FAKE_MD
	if (m_fakeMdThreadF.valid() == false || m_fakeMdThreadF.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready){
		m_isTrading.store(true);
		m_fakeMdThreadF = std::async(std::launch::async, std::bind(&ClientSession::ReturnFakeCTPMessage, this, instru));
	}

	return true;
#else 
	return BaseClientSession::StartTrade(instru, strategyName, errcode);
#endif
}

#ifdef FAKE_MD
bool ClientSession::ReturnFakeCTPMessage(const std::string& instru){

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

		Json::Value root;
		root["Type"] = "MA";
		root["Data"] = Json::Value::nullRef;
		root["Data"]["Long"] = 2005;
		root["Data"]["Short"] = 1995;
		Json::FastWriter writer;
		std::string extraData = writer.write(root);
		Transmission::Utils::SendMDInfo(m_session, lastPrice, openPrice, lastPrice, highestPrice, lowestPrice, volume, volume, 10000.0, now_time * 2, instru, extraData);

		direction = rand() & 1;  // 0 or 1 randomly
		OrderSysID++;
		char orderId[256] = { 0 };
		SPRINTF(orderId, "%d", OrderSysID);
		if (idx % 10 == 0){
			Transmission::Utils::SendDealInfo(m_session, Transmission::INSERT_ORDER, "rb1610", direction, '0', lastPrice, volume, orderId, now_time * 2);
			if (idx % 15 == 0)
				Transmission::Utils::SendDealInfo(m_session, Transmission::TRADE, "rb1610", direction, '0', lastPrice, volume, orderId, now_time * 2 + 2);
			else
				Transmission::Utils::SendDealInfo(m_session, Transmission::CANCELL_ORDER, "rb1610", direction, '0', lastPrice, volume, orderId, now_time * 2 + 2);
		}
		sleep(500);
	}
	return false;
}
#endif

void ClientSession::SendTickToClient(const TickWrapper& tick){
	if (m_tickoff)
		return;

	if (m_total_vol != 0)
		Transmission::Utils::SendMDInfo(m_session, tick.PreSettlementPrice(), tick.OpenPrice(), tick.LastPrice(), tick.HighestPrice(), tick.LowestPrice(), tick.Volume() - m_total_vol, tick.Volume(), tick.TurnOver(), tick.toTimeStamp(), tick.InstrumentId(), tick.GetTechVec()->ToJson());
	
	m_total_vol = tick.Volume();
}

void ClientSession::SendPostionInfoToClient(){
	Transmission::Utils::SendPositionInfo(m_session, m_PPMgr->GetBalanceMoney(), m_PPMgr->GetAvailableMoney(), m_PPMgr->GetUsedMargin(), \
		m_PPMgr->GetFrozenMargin(), m_PPMgr->GetCommission(), m_PPMgr->GetFrozenCommission(), m_PPMgr->GetPositionProfit(), \
		m_PPMgr->PositionOfInstruments());
}

void ClientSession::OnAccountInitFinished(){
	m_PositionInfo_ready.store(true);
	//log position info
	SYNC_DEBUG_LOG << "PositionInfo(BeforeTrade):" << m_PPMgr->ToString();
	DEBUG_LOGGER->flush();
	SendPostionInfoToClient();
}

void ClientSession::OnRtnOrder(CThostFtdcOrderField* pOrder){
	BaseClientSession::OnRtnOrder(pOrder);
	
	long long timeStamp = CommonUtils::DateTimeToTimestamp(pOrder->InsertDate, pOrder->InsertTime) * 2;
	Transmission::Utils::SendDealInfo(m_session, Transmission::INSERT_ORDER, pOrder->InstrumentID ,pOrder->Direction, pOrder->CombOffsetFlag[0], pOrder->LimitPrice, pOrder->VolumeTotalOriginal, pOrder->OrderRef, timeStamp);
	if (m_PositionInfo_ready)
		SendPostionInfoToClient();
}

void ClientSession::OnRtnTrade(CThostFtdcTradeField* pTrade){
	BaseClientSession::OnRtnTrade(pTrade);

	long long timeStamp = CommonUtils::DateTimeToTimestamp(pTrade->TradeDate, pTrade->TradeTime) * 2;
	Transmission::Utils::SendDealInfo(m_session, Transmission::TRADE, pTrade->InstrumentID, pTrade->Direction, pTrade->OffsetFlag, pTrade->Price, pTrade->Volume, pTrade->OrderRef, timeStamp);
	if (m_PositionInfo_ready)
		SendPostionInfoToClient();
}

void ClientSession::OnCancelOrder(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo){
	Transmission::Utils::SendDealInfo(m_session, Transmission::CANCELL_ORDER, pInputOrderAction->InstrumentID, 0, 0, 0, 0, pInputOrderAction->OrderRef, 0);
	if (m_PositionInfo_ready)
		SendPostionInfoToClient();
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
