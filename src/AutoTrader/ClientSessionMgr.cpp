#include "stdafx.h"
#include "ClientSession.h"
#include "ClientSessionMgr.h"
#include "ThostFtdcTraderApi.h"
#include "AccountMgr.h"
#include "TriggerFactory.h"
#include "Utils.h"
#include <algorithm>
#include "Transmission.h"

ClientSessionMgr* ClientSessionMgr::_instance = NULL;

ClientSessionMgr* ClientSessionMgr::getInstance(){
	if (_instance == NULL)
	{
		_instance = new ClientSessionMgr();
	}
	return _instance;
}

ClientSessionMgr::ClientSessionMgr()
	:m_pTradeUserApi(CThostFtdcTraderApi::CreateFtdcTraderApi())
{
}

void ClientSessionMgr::LoginAccount(const std::string& userId, const std::string& pw, const std::shared_ptr<Transmission::socket_session>& session){
	bool isExisted = Account::Manager::Instance().IsAccountExisted(userId);
	if (isExisted){
		bool success = Account::Manager::Instance().CheckPassword(userId, pw);
		if (success){
			std::shared_ptr<ClientSession> clientSessionSp = GetClientSession(session);
			if (!clientSessionSp)
			{
				clientSessionSp = std::make_shared<ClientSession>(userId, session, m_pTradeUserApi);
				m_client_sessions[session] = clientSessionSp;
				Transmission::Utils::SendLoginResultInfo(session, Transmission::Succeed);
			}
			clientSessionSp->OnLoginRequest();
		}
		else{
			Transmission::Utils::SendLoginResultInfo(session, Transmission::LoginFailed_PW);
		}
	}
	else{
		Transmission::Utils::SendLoginResultInfo(session, Transmission::LoginFailed_AC);
	}
}

void ClientSessionMgr::StartTrade(const std::string& instru, const std::string& strategyName, const std::shared_ptr<Transmission::socket_session>& session){
	std::shared_ptr<ClientSession> clientSessionSp = GetClientSession(session);
	if (clientSessionSp){
		clientSessionSp->OnStartTradeRequest(instru, strategyName);
	}
	else{
		Transmission::Utils::SendStartTradeResultInfo(session, Transmission::LoginNeeded);
	}
}

void ClientSessionMgr::LogoutAccount(const std::shared_ptr<Transmission::socket_session>& session){
	std::shared_ptr<ClientSession> clientSessionSp = GetClientSession(session);
	auto pair = std::find_if(m_client_sessions.begin(), m_client_sessions.end(), [&session](const std::pair<std::shared_ptr<Transmission::socket_session>, std::shared_ptr<ClientSession> >& item){
		return (*session.get()) == (*item.first.get());
	});

	if (pair != m_client_sessions.end())// logged
	{
		m_client_sessions.erase(session);
	}
	Transmission::Utils::SendLogOutResultInfo(session, Transmission::Succeed);
}

void ClientSessionMgr::StopTrade(const std::shared_ptr<Transmission::socket_session>& session){
	std::shared_ptr<ClientSession> clientSessionSp = GetClientSession(session);
	if (clientSessionSp){
		clientSessionSp->StopTrade();
		Transmission::Utils::SendStopTradeResultInfo(session, Transmission::Succeed);
	}
}

std::shared_ptr<ClientSession> ClientSessionMgr::GetClientSession(const std::shared_ptr<Transmission::socket_session>& session){
	auto iter = std::find_if(m_client_sessions.begin(), m_client_sessions.end(), [&](const std::pair<std::shared_ptr<Transmission::socket_session>, std::shared_ptr<ClientSession> >& pair){
		return (*session.get()) == (*pair.first.get());
	});
	if (iter != m_client_sessions.end()){
		return m_client_sessions[session];
	}
	else{
		return nullptr;
	}
}