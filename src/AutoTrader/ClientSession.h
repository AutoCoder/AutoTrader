#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <memory>
#include <mutex>
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
	enum ErrorCode;
};

namespace Account{
	class Meta;
}

struct CThostFtdcOrderField;
struct CThostFtdcTradeField;
struct CThostFtdcInputOrderActionField;
struct CThostFtdcRspInfoField;

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

	~ClientSession();
private:

	bool Logout(); //identify User By session

	bool StartTrade(const std::string& instru, const std::string& strategyName, Transmission::ErrorCode& errcode);

	void WaitAndPopCurrentOrder(Order& ord);//multi-thread notice

	//send out Account status to fifo, finally got by client
	void OnAccountInitFinished();

	//send out Account status to fifo, finally got by client
	void OnRtnOrder(CThostFtdcOrderField* pOrder);

	//send out Account status to fifo, finally got by client
	void OnRtnTrade(CThostFtdcTradeField* pTrade);

	//send out Account status to fifo, finally got by client
	void OnCancelOrder(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo);



private:
	std::atomic<bool>                               m_isTrading;
	std::string										m_userId;
	std::unique_ptr<Order>                          m_pending_order;
	std::unique_ptr<CtpTradeSpi>                    m_trade_spi;
	std::shared_ptr<RealTimeDataProcessor>          m_realtimedata_processor;
	std::unique_ptr<AP::AccountDetailMgr>           m_detailMgr;
	std::shared_ptr<Transmission::socket_session>   m_session;

	std::mutex                                      m_mtx;
	std::condition_variable                         m_con;
	std::thread                                     m_exeOrderThread;
};

#endif