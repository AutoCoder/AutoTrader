#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

//#define FAKE_MD

#include "BaseClientSession.h"

namespace Transmission{
	class socket_session;
};

typedef Transmission::socket_session SockSession;
typedef std::shared_ptr<SockSession> SockSessionSP;

struct CThostFtdcOrderField;
struct CThostFtdcTradeField;
struct CThostFtdcInputOrderActionField;
struct CThostFtdcRspInfoField;

class ClientSession : public BaseClientSession
{
public:
	ClientSession(const std::string& userId, const SockSessionSP& s);

	virtual bool Init_CTP();

	virtual void OnOrderTrigger(const Order& ord);

	virtual void SendTickToClient(const TickWrapper& tick);

	bool StartTrade(const std::string& instru, const std::string& strategyName, ErrorCode& errcode);

	void SendPostionInfoToClient();

	void OnLoginRequest();

	void OnStartTradeRequest(const std::string& instru, const std::string& strategyName);

	void UpdateSocketSession(const SockSessionSP& s) { m_session = s;  }

	void SetTickOff(bool off) { m_tickoff = off; }

	~ClientSession();
private:

	//send out Account status to fifo, finally got by client
	void OnAccountInitFinished();

	//send out Account status to fifo, finally got by client
	void OnRtnOrder(CThostFtdcOrderField* pOrder);

	//send out Account status to fifo, finally got by client
	void OnRtnTrade(CThostFtdcTradeField* pTrade);

	//send out Account status to fifo, finally got by client
	void OnCancelOrder(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo);
#ifdef FAKE_MD
	bool ReturnFakeCTPMessage(const std::string& instru);
#endif

private:
	bool                           m_tickoff;
	std::shared_ptr<SockSession>   m_session;

#ifdef FAKE_MD
	std::future<bool>                               m_fakeMdThreadF;
#endif

};

#endif