#ifndef ACCOUNT_POOL_H
#define ACCOUNT_POOL_H
#include <string>
#include <map>
#include <memory>
#include "socket_session.h"

class ClientSession;
class CThostFtdcTraderApi;

//*********************************************************
//This singleton must be launched before mutli thread start
//*********************************************************
class ClientSessionMgr
{
public:
	~ClientSessionMgr(){};
	static ClientSessionMgr* getInstance();

public:

	CThostFtdcTraderApi* TradeApiPtr() const { return m_pTradeUserApi; }

	void AddClientSession(const std::shared_ptr<ClientSession>& newAcc);

	void LoginAccount(const std::string& userId, const std::string& pw, const std::shared_ptr<Transmission::socket_session>& session);

	void StartTrade(const std::string& instru, const std::string& strategyName, const std::shared_ptr<Transmission::socket_session>& session);

	void LogoutAccount(const std::shared_ptr<Transmission::socket_session>& session);

	void StopTrade(const std::shared_ptr<Transmission::socket_session>& session);

private:
	ClientSessionMgr();
	ClientSessionMgr(const ClientSessionMgr&) = delete;
	ClientSessionMgr& operator=(const ClientSessionMgr &) = delete;
	static ClientSessionMgr *_instance;

private:
	std::shared_ptr<ClientSession> GetClientSession(const std::shared_ptr<Transmission::socket_session>& session);

private:
	CThostFtdcTraderApi*																		m_pTradeUserApi;
	std::map<std::shared_ptr<Transmission::socket_session>, std::shared_ptr<ClientSession>>		m_client_sessions;

};

#endif