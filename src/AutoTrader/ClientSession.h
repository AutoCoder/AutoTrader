#ifndef ACCOUNT_H
#define ACCOUNT_H

#define FAKE_MD

#include <string>
#include <memory>
#include <mutex>
#include <future>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>

class Order;
class TickWrapper;
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
	struct Meta;
}

struct CThostFtdcOrderField;
struct CThostFtdcTradeField;
struct CThostFtdcInputOrderActionField;
struct CThostFtdcRspInfoField;

typedef int TransmissionErrorCode;

class ClientSession
{
public:
	ClientSession(const std::string& userId, const std::shared_ptr<Transmission::socket_session>& s, CThostFtdcTraderApi* api);

	bool AppendOrder(const Order& order);//multi-thread notice

	void ExecutePendingOrder();

	void InformClientViewer(const TickWrapper& tick);

	void OnLoginRequest();

	void OnStartTradeRequest(const std::string& instru, const std::string& strategyName);

	void StopTrade();

	bool IsTrading() const { return m_isTrading.load(); }

	std::string UserId() const { return m_userId; };

	void UpdateSocketSession(const std::shared_ptr<Transmission::socket_session>& s) { m_session = s;  }

	~ClientSession();
private:

	bool Logout(); //identify User By session

	bool StartTrade(const std::string& instru, const std::string& strategyName, TransmissionErrorCode& errcode);

	void WaitAndPopCurrentOrder(Order& ord);//multi-thread notice

	//send out Account status to fifo, finally got by client
	void OnAccountInitFinished();

	//send out Account status to fifo, finally got by client
	void OnRtnOrder(CThostFtdcOrderField* pOrder);

	//send out Account status to fifo, finally got by client
	void OnRtnTrade(CThostFtdcTradeField* pTrade);

	//send out Account status to fifo, finally got by client
	void OnCancelOrder(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo);
#ifdef FAKE_MD
	bool ReturnFakeCTPMessage();
#endif
private:
	std::atomic<bool>                               m_isTrading;
	std::string										m_userId;
	std::unique_ptr<Order>                          m_pending_order;
	std::unique_ptr<CtpTradeSpi>                    m_trade_spi;
	std::shared_ptr<RealTimeDataProcessor>          m_realtimedata_processor;
	std::shared_ptr<Transmission::socket_session>   m_session;
	std::unique_ptr<AP::AccountDetailMgr>           m_detailMgr;

	int                                             m_total_vol;
	std::mutex                                      m_mtx;
	std::condition_variable                         m_con;
	std::thread                                     m_exeOrderThread;
	bool											m_exeOrderThread_running;
};

#endif