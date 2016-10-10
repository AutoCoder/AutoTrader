#include "stdafx.h"
#include <functional>
#include <algorithm>
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
#include "Config.h"
#include "TickWrapper.h"
#include "Transmission.h"
#include "Utils.h"
#include "CommonUtils.h"
#include "crossplatform.h"
#include "BaseClientSession.h"
#include "PPMgr.h"


BaseClientSession::BaseClientSession(const std::string& userId)
: m_userId(userId)
, m_isTrading(false)
, m_semiAuto(false)
, m_PPMgr(std::unique_ptr<PPMgr>(new PPMgr()))
, m_total_vol(0)
, m_ReleaseingCtpAccount(false)
, m_PositionInfo_ready(false)
, m_runningInstrument("")
, m_runningStrategy("")
{

}

BaseClientSession::~BaseClientSession()
{
	m_ReleaseingCtpAccount = true; 
	m_con.notify_all();// notify stop executing thread if it's pending for new order.
	if (m_trade_api)
	{
		m_trade_api->RegisterSpi(NULL);
		m_trade_api->Release();
		m_trade_api = nullptr;
	}

	if (m_trade_spi){
		delete m_trade_spi;
		m_trade_spi = nullptr;
	}

	SYNC_LOG << "Account:" << m_userId << "shutdown...";
}

bool BaseClientSession::Init_CTP(){
	
	m_trade_api = CThostFtdcTraderApi::CreateFtdcTraderApi();
	assert(m_trade_api);

	Account::Meta meta = Account::Manager::Instance().GetMeta(m_userId);
	InitedAccountCallback onInitedAccount_Callback = [](){};
	RtnOrderCallback onRtnOrder_Callback = std::bind(&BaseClientSession::OnRtnOrder, this, std::placeholders::_1);
	RtnTradeCallback OnRtnTrade_Callback = std::bind(&BaseClientSession::OnRtnTrade, this, std::placeholders::_1);

	m_trade_spi = new CtpTradeSpi(m_trade_api, meta.m_BrokerId.c_str(), meta.m_UserId.c_str(), meta.m_Password.c_str(), \
		Config::Instance()->ProductName().c_str(), *(m_PPMgr.get()), onInitedAccount_Callback, onRtnOrder_Callback, \
		OnRtnTrade_Callback);

	m_trade_api->RegisterSpi((CThostFtdcTraderSpi*)m_trade_spi);
	m_trade_api->SubscribePublicTopic(THOST_TERT_RESTART);
	m_trade_api->SubscribePrivateTopic(THOST_TERT_RESTART);
	m_trade_api->RegisterFront(const_cast<char*>(Config::Instance()->CtpTradeFront().c_str()));
	m_trade_api->Init();
	SYNC_LOG << "CTP Account of " << m_userId << "...Setup";
	
	return true;
}

//may access by mdThread and m_exeOrderThread
bool BaseClientSession::AppendOrder(const Order& order)
{
	std::lock_guard<std::mutex> lk(m_mtx);
	if (m_pending_order.get()){
		return false;
	}
	else{
		m_pending_order.reset(new Order(order));
		m_con.notify_all();
		return true;
	}
}

//may access by mdThread and m_exeOrderThread
void BaseClientSession::WaitAndPopCurrentOrder(Order& ord){

	std::unique_lock<std::mutex> lk(m_mtx);

	m_con.wait(lk, [this]{return m_pending_order.get() || m_ReleaseingCtpAccount; });

	if (m_ReleaseingCtpAccount)
		return;

	ord = *(m_pending_order.get());
	m_pending_order.reset();
}

bool BaseClientSession::ExecutePendingOrder(){
	while (m_isTrading.load()){
		Order ord;
		WaitAndPopCurrentOrder(ord);//blocking
		if (m_ReleaseingCtpAccount)
			break;
		
		OnOrderTrigger(ord);
	}
	return true;
}

void BaseClientSession::OnRtnOrder(CThostFtdcOrderField* pOrder){

}

void BaseClientSession::OnRtnTrade(CThostFtdcTradeField* pTrade){
	//SYNC_TRADE_LOG << CommonUtils::ConvertTradeToString(*pTrade);
}

void BaseClientSession::StopTrade(){
	if (!m_isTrading.load())
		return;
	m_isTrading.store(false);
}

void BaseClientSession::OnOrderTrigger(const Order& ord){
	if (m_isTrading.load()){ //Check this bool var again, prevent to execute new pushed order after user stop trade.
		m_trade_spi->CancelOrder(ord.GetTriggerTick(), 0, ord.GetInstrumentId());
		m_trade_spi->ReqOrderInsert(ord);
	}
}

bool BaseClientSession::StartTrade(const std::string& instru, const std::string& strategyName, ErrorCode& errcode){
	if (m_isTrading.load()){
		errcode = Transmission::TradingNow;
		return false;
	}

	Account::Meta meta = Account::Manager::Instance().GetMeta(m_userId);

	if (std::find(meta.m_Instruments.begin(), meta.m_Instruments.end(), instru) != meta.m_Instruments.end()){
		auto strategyPtr = TriggerFactory::Instance()->GetTrigger(m_userId, strategyName);
		if (strategyPtr){
			strategyPtr->BindWithAccount(m_PPMgr.get());
			MdProcessorPool::getInstance()->AddProcessor(instru, strategyPtr, this);
			m_runningInstrument = instru;
			m_runningStrategy = strategyName;
			m_isTrading.store(true);

			if (m_orderExecuteThreadF.valid()==false || m_orderExecuteThreadF.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) // if thread is not started or finished.
				m_orderExecuteThreadF = std::async(std::launch::async, std::bind(&BaseClientSession::ExecutePendingOrder, this));
			else if (m_orderExecuteThreadF.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
				bool toRemoved = true;
				//!!!!Note: if thread is finished, this->joinable() still == true.
				//m_exeOrderThread = std::thread(&ClientSession::ExecutePendingOrder, this);// This will start the thread. Notice move semantics!
			//else{   //if m_exeOrderThread is running, should not call move ctor
			//}

			return true;
		}
		else{
			errcode = Transmission::InvalidTradeArguments;
			return false;
		}
	}
	else{
		errcode = Transmission::InvalidTradeArguments;
		return false;
	}
}

