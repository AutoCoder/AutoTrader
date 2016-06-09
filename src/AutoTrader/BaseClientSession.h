#ifndef BASE_CLIENT_SESSION_H
#define BASE_CLIENT_SESSION_H


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
class MdProcessor;
class CThostFtdcTraderApi;
struct CThostFtdcOrderField;
struct CThostFtdcTradeField;

namespace PP{
	class PositionProfitMgr;
}

namespace AP{
	class AccountDetailMgr;
}

namespace Account{
	struct Meta;
}

typedef int ErrorCode;

class BaseClientSession
{
public:
	BaseClientSession(const std::string& userId);
	virtual ~BaseClientSession();
	virtual bool Init_CTP();
	virtual void SendTickToClient(const TickWrapper& tick) {};

	//These below two Functions should be syncd with one mutex, they may conflict with m_pending_order
	bool AppendOrder(const Order& order);//multi-thread notice
	void WaitAndPopCurrentOrder(Order& ord);//multi-thread notice

	bool IsTrading() const { return m_isTrading.load(); }
	bool IsPositionInfoReady() const { return m_PositionInfo_ready.load(); }
	std::string UserId() const { return m_userId; };
	std::string RunningInstrument() const { return m_runningInstrument; }
	std::string RunningStrategy() const { return m_runningStrategy; }

	bool StartTrade(const std::string& instru, const std::string& strategyName, ErrorCode& errcode);
	void StopTrade();

protected:
	bool ExecutePendingOrder();

	void OnRtnOrder(CThostFtdcOrderField* pOrder);

	void OnRtnTrade(CThostFtdcTradeField* pTrade);

protected:
	std::string										m_userId;
	std::atomic<bool>                               m_isTrading; // access by thread-OrderExecutor and thread-ActionQueueInvoker
	std::unique_ptr<AP::AccountDetailMgr>           m_detailMgr;
	typedef PP::PositionProfitMgr PPMgr;
	std::unique_ptr<PPMgr>                          m_PPMgr;
	int                                             m_total_vol;
	bool                                            m_ReleaseingCtpAccount;
	std::atomic<bool>                               m_PositionInfo_ready;//access by thread-tradespi and thread-ActionQueueInvoker
	std::string                                     m_runningInstrument;
	std::string                                     m_runningStrategy;

	std::unique_ptr<Order>                          m_pending_order;
	CtpTradeSpi*									m_trade_spi;
	CThostFtdcTraderApi*                            m_trade_api;
	
	std::mutex                                      m_mtx;
	std::condition_variable                         m_con;
	std::future<bool>                               m_orderExecuteThreadF;
	
};

#endif
