#include <algorithm>

#include "stdafx.h"
#include "ClientSession.h"
#include "ClientSessionMgr.h"
#include "ThostFtdcTraderApi.h"
#include "AccountMgr.h"
#include "TriggerFactory.h"
#include "Utils.h"
#include "Transmission.h"
#include "socket_session.h"

ClientSessionMgr* ClientSessionMgr::_instance = NULL;

ClientSessionMgr* ClientSessionMgr::getInstance(){
	if (_instance == NULL)
	{
		_instance = new ClientSessionMgr();
	}
	return _instance;
}

ClientSessionMgr::ClientSessionMgr()
{
}

void ClientSessionMgr::LoginAccount(const std::string& userId, const std::string& pw, const SockSessionSP& session){
	bool isExisted = Account::Manager::Instance().IsAccountExisted(userId);
	if (isExisted){
		bool success = Account::Manager::Instance().CheckPassword(userId, pw);
		if (success){
			ClientSessionSP clientSessionSp = GetClientSession(session);

			if (!clientSessionSp) // if user logout, and the clientSession is lost, rebind the socket session with client session
				clientSessionSp = TryBindToSession(userId, session);

			if (!clientSessionSp)
			{
				Transmission::Utils::SendLoginResultInfo(session, Transmission::Succeed);
				clientSessionSp = std::make_shared<ClientSession>(userId, session);
				m_client_sessions[session] = clientSessionSp;
			}else{
				if (clientSessionSp->IsPositionInfoReady())
					clientSessionSp->SendPostionInfoToClient();
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

void ClientSessionMgr::StartTrade(const std::string& instru, const std::string& strategyName, const SockSessionSP& session){
	ClientSessionSP clientSessionSp = GetClientSession(session);
	if (clientSessionSp){
		clientSessionSp->OnStartTradeRequest(instru, strategyName);
	}
	else{
		Transmission::Utils::SendStartTradeResultInfo(session, Transmission::LoginNeeded);
	}
}

void ClientSessionMgr::LogoutAccount(const SockSessionSP& session){
	ClientSessionSP clientSessionSp = GetClientSession(session);
	auto pair = std::find_if(m_client_sessions.begin(), m_client_sessions.end(), [&session](const std::pair<SockSessionSP, ClientSessionSP >& item){
		return (*session.get()) == (*item.first.get());
	});

	if (pair != m_client_sessions.end())// logged
	{
		m_client_sessions.erase(session);
	}
	Transmission::Utils::SendLogOutResultInfo(session, Transmission::Succeed);
}

void ClientSessionMgr::StopTrade(const SockSessionSP& session){
	ClientSessionSP clientSessionSp = GetClientSession(session);
	if (clientSessionSp){
		clientSessionSp->StopTrade();
		Transmission::Utils::SendStopTradeResultInfo(session, Transmission::Succeed);
	}
	else{
		Transmission::Utils::SendStartTradeResultInfo(session, Transmission::LoginNeeded);
	}
}

void ClientSessionMgr::QueryPosition(const SockSessionSP& session){
	ClientSessionSP clientSessionSp = GetClientSession(session);
	if (clientSessionSp){
		if (clientSessionSp->IsPositionInfoReady())
			clientSessionSp->SendPostionInfoToClient();
		else
			Transmission::Utils::SendQueryPositionResultInfo(session, Transmission::PositionInfoIsNotReady);
	}
	else{
		Transmission::Utils::SendStartTradeResultInfo(session, Transmission::LoginNeeded);
	}
}

void ClientSessionMgr::TurnOffTick(const SockSessionSP& session, bool off){
	ClientSessionSP clientSessionSp = GetClientSession(session);
	if (clientSessionSp){
		clientSessionSp->SetTickOff(off);
	}
}

void ClientSessionMgr::TurnOnSemiAutoTrading(const SockSessionSP& session, bool on){
	ClientSessionSP clientSessionSp = GetClientSession(session);
	if (clientSessionSp){
		clientSessionSp->TurnOnSemiAutoTrading(on);
	}	
}

ClientSessionSP ClientSessionMgr::GetClientSession(const SockSessionSP& session, const std::string& userId){
	auto iter = std::find_if(m_client_sessions.begin(), m_client_sessions.end(), [&](const std::pair<SockSessionSP, ClientSessionSP >& pair){
		return (*session.get()) == (*pair.first.get());
	});
	if (iter != m_client_sessions.end()){
		return m_client_sessions[session];
	}
	else{
		return nullptr;	
	}
}

ClientSessionSP ClientSessionMgr::TryBindToSession(const std::string& userId, const SockSessionSP& session){
	auto foundIter = std::find_if(m_client_sessions.begin(), m_client_sessions.end(), [&userId](const std::pair<SockSessionSP, ClientSessionSP >& pair){
		return pair.second->UserId() == userId;
	});

	if (foundIter != m_client_sessions.end())
	{
		auto clientSession = foundIter->second;
		m_client_sessions.erase(foundIter);
		clientSession->UpdateSocketSession(session);
		m_client_sessions[session] = clientSession;
		return clientSession;
	}
	else{
		return nullptr;
	}
}