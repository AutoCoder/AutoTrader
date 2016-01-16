#include "stdafx.h"
#include "ThostFtdcTraderApi.h"
#include "Account.h"
#include "ConfigV2.h"
#include "AccountMgr.h"
#include "RealTimeDataProcessor.h"
#include "RealTimeDataProcessorPool.h"
#include "Order.h"
#include "tradespi.h"
#include "AP_Mgr.h"
#include <functional>
#include "TriggerFactory.h"

Account::Account()
	:m_isLogin(true)
{
}

Account::Account(const std::string& brokerId, const std::string& un, const std::string& pw)
	: m_brokderId(brokerId)
	, m_ctp_username(un)
	, m_ctp_password(pw)
	, m_isLogin(false)
	, m_isTrading(false)
	, m_detailMgr(std::make_unique<AP::AccountDetailMgr>())
{
}

Account::~Account()
{
	m_instrumentList.clear();
	m_strategyList.clear();
}

//may access by mdThread and m_exeOrderThread
bool Account::AppendOrder(const Order& order){
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
void Account::WaitAndPopCurrentOrder(Order& ord){

	std::unique_lock<std::mutex> lk(m_mtx);

	m_con.wait(lk, [this]{return m_pending_order.get(); });
	ord = *(m_pending_order.get());
	m_pending_order.reset();
}

void Account::ExecutePendingOrder(){
	while (m_isTrading){
		Order ord;
		WaitAndPopCurrentOrder(ord);

		m_trade_spi->CancelOrder(ord.GetTriggerTick(), 6, ord.GetInstrumentId());
		m_trade_spi->ReqOrderInsert(ord);
	}
}

bool Account::Login(const std::shared_ptr<Transmission::socket_session>& s, const std::string& pw) {
	if (pw == m_ctp_password){
		m_session = s;
		m_isLogin = true;

		std::function<void(void)> accountInitFinished_Callback = std::bind(&Account::OnAccountInitFinished, this);
		std::function<void(void)> onRtnOrder_Callback = std::bind(&Account::OnRtnOrder, this);
		std::function<void(void)> OnRtnTrade_Callback = std::bind(&Account::OnRtnTrade, this);
		std::function<void(void)> OnCancelOrder_Callback = std::bind(&Account::OnCancelOrder, this);

		m_trade_spi = std::make_unique<CtpTradeSpi>(AccountMgr::getInstance()->TradeApiPtr(), \
			m_brokderId.c_str(), m_ctp_username.c_str(), m_ctp_password.c_str(), ConfigV2::Instance()->ProductName().c_str(), *(m_detailMgr.get()), \
			accountInitFinished_Callback, onRtnOrder_Callback, std::bind(&Account::OnRtnTrade, this), OnCancelOrder_Callback);

		AccountMgr::getInstance()->TradeApiPtr()->RegisterSpi((CThostFtdcTraderSpi*)(m_trade_spi.get()));
		AccountMgr::getInstance()->TradeApiPtr()->SubscribePublicTopic(THOST_TERT_RESTART);
		AccountMgr::getInstance()->TradeApiPtr()->SubscribePrivateTopic(THOST_TERT_RESTART);
		AccountMgr::getInstance()->TradeApiPtr()->RegisterFront(const_cast<char*>(ConfigV2::Instance()->CtpTradeFront().c_str()));
		return true;
	}
	else{
		return false;
	}
}

bool Account::Logout() {
	m_session.reset();
	m_isLogin = false;

	StopTrade();// must
	return true;
}

bool Account::StartTrade(const std::string& instru, const std::string& strategyName){
	if (m_isTrading)
		return false;

	//verify arguments
	if (std::find(m_instrumentList.begin(), m_instrumentList.end(), instru) != m_instrumentList.end()
		&& std::find(m_strategyList.begin(), m_strategyList.end(), strategyName) != m_strategyList.end())
	{
		if (!m_realtimedata_processor.get()){
			auto strategyPtr = TriggerFactory::Instance()->GetTrigger(Id(), strategyName);
			m_realtimedata_processor = std::make_shared<RealTimeDataProcessor>(strategyPtr, instru, this);
			RealTimeDataProcessorPool::getInstance()->AddProcessor(m_realtimedata_processor);
		}
		m_isTrading.store(true);

		// This will start the thread. Notice move semantics!
		m_exeOrderThread = std::thread(&Account::ExecutePendingOrder, this);

		return true;
	}
	else{
		return false;
	}
}

void Account::StopTrade(){
	if (!m_isTrading)
		return;
	m_isTrading = false;
}


void Account::OnAccountInitFinished(){

}

void Account::OnRtnOrder(){}

void Account::OnRtnTrade(){}

void Account::OnCancelOrder(){}