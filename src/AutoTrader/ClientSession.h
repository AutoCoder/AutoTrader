#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

//#define FAKE_MD

#include "BaseClientSession.h"


namespace Transmission{
	class socket_session;
};

struct CThostFtdcOrderField;
struct CThostFtdcTradeField;
struct CThostFtdcInputOrderActionField;
struct CThostFtdcRspInfoField;

class ClientSession : public BaseClientSession
{
public:
	ClientSession(const std::string& userId, const std::shared_ptr<Transmission::socket_session>& s);

	virtual bool Init_CTP();

	bool StartTrade(const std::string& instru, const std::string& strategyName, ErrorCode& errcode);

	void SendTickToClient(const TickWrapper& tick);

	void SendTechToClient(const TickWrapper& tick);

	void SendPostionInfoToClient();

	void OnLoginRequest();

	void OnStartTradeRequest(const std::string& instru, const std::string& strategyName);

	void UpdateSocketSession(const std::shared_ptr<Transmission::socket_session>& s) { m_session = s;  }

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
	bool ReturnFakeCTPMessage();
#endif

private:
	std::shared_ptr<Transmission::socket_session>   m_session;

#ifdef FAKE_MD
	std::future<bool>                               m_fakeMdThreadF;
#endif

};

#endif