#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

class IPositionControl;
class Strategy;
class Order;
class CtpTradeSpi;
class RealTimeDataProcessor;
class CThostFtdcTraderApi;

namespace AP{
	class AccountDetailMgr;
};

namespace Transmission{
	class socket_session;
};

namespace Account{
	class Meta;
}

class ClientSession
{
public:
	ClientSession(const std::string& userId, const std::shared_ptr<Transmission::socket_session>& s, CThostFtdcTraderApi* api);
	//ClientSession(const std::string& id);

	//void AddInstrument(const std::string & instr){
	//	m_instrumentList.push_back(instr);
	//}

	//void AddStrategy(const std::string& strategyIdx){
	//	m_strategyList.push_back(strategyIdx);
	//}

	bool AppendOrder(const Order& order);//multi-thread notice

	void ExecutePendingOrder();

	//bool IsLogged() const { return m_isLogin; }

	bool IsTrading() const { return m_isTrading.load(); }

	//bool Login(const std::shared_ptr<Transmission::socket_session>& s, const std::string& pw);
	bool Logout(); //identify User By session

	bool StartTrade(const std::string& instru, const std::string& strategyName, std::string& errmsg);

	void StopTrade();

	//Transmission::socket_session_ptr SessionPtr() const { return m_session.get(); }

	//std::string Id() const { return m_brokderId + m_ctp_username; };

	~ClientSession();
private:
	void WaitAndPopCurrentOrder(Order& ord);//multi-thread notice

	//send out Account status to fifo, finally got by client
	void OnAccountInitFinished();

	//send out Account status to fifo, finally got by client
	void OnRtnOrder();

	//send out Account status to fifo, finally got by client
	void OnRtnTrade();

	//send out Account status to fifo, finally got by client
	void OnCancelOrder();

private:
	bool											m_isLogin;
	std::atomic<bool>                               m_isTrading;
	std::string										m_userId;
	std::string										m_ctp_username;
	std::string										m_ctp_password;
	std::vector<std::string>						m_instrumentList;
	std::vector<std::string>						m_strategyList;
	std::unique_ptr<Order>                          m_pending_order;
	std::unique_ptr<CtpTradeSpi>                    m_trade_spi;
	std::shared_ptr<IPositionControl>               m_position_ctl;
	std::shared_ptr<Strategy>                       m_strategy;
	std::shared_ptr<RealTimeDataProcessor>          m_realtimedata_processor;
	std::unique_ptr<AP::AccountDetailMgr>           m_detailMgr;
	std::shared_ptr<Transmission::socket_session>   m_session;

	std::mutex                                      m_mtx;
	std::condition_variable                         m_con;
	std::thread                                     m_exeOrderThread;
};

#endif