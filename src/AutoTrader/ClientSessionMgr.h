#ifndef ACCOUNT_POOL_H
#define ACCOUNT_POOL_H
#include <string>
#include <map>
#include <memory>
//#include "socket_session.h"

namespace Transmission{
	class socket_session;
};

typedef Transmission::socket_session SockSession;
typedef std::shared_ptr<SockSession> SockSessionSP;

class ClientSession;
class CThostFtdcTraderApi;

typedef std::shared_ptr<ClientSession> ClientSessionSP;

//*********************************************************
//This singleton must be launched before mutli thread start
//*********************************************************
class ClientSessionMgr
{
public:
	~ClientSessionMgr(){};
	static ClientSessionMgr* getInstance();

public:
	void AddClientSession(const ClientSessionSP& newAcc);

	void LoginAccount(const std::string& userId, const std::string& pw, const SockSessionSP& session);

	void StartTrade(const std::string& instru, const std::string& strategyName, const SockSessionSP& session);

	void LogoutAccount(const SockSessionSP& session);

	void StopTrade(const SockSessionSP& session);

	void QueryPosition(const SockSessionSP& session);

	void TurnOffTick(bool off);

	void TurnOffAutoTrading(bool off);

private:
	ClientSessionMgr();
	ClientSessionMgr(const ClientSessionMgr&) = delete;
	ClientSessionMgr& operator=(const ClientSessionMgr &) = delete;
	static ClientSessionMgr *_instance;

private:
	ClientSessionSP GetClientSession(const SockSessionSP& session, const std::string& userId = "");
	ClientSessionSP TryBindToSession(const std::string& userId, const SockSessionSP& session);

private:
	std::map<SockSessionSP, ClientSessionSP>		m_client_sessions;
};

#endif