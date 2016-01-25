#include "stdafx.h"
#include "ClientSession.h"
#include "ClientSessionMgr.h"
#include "ThostFtdcTraderApi.h"
#include "AccountMgr.h"
#include "Utils.h"

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
			if (m_client_sessions.find(session) == m_client_sessions.end()){
				m_client_sessions[session] = std::make_shared<ClientSession>(userId, session, m_pTradeUserApi);
				Transmission::Utils::SendLoginResultInfo(session, Transmission::Succeed);
			}
			else{
				Transmission::Utils::SendLoginResultInfo(session, Transmission::LoginRepeatedly);
			}
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
	if (m_client_sessions.find(session) != m_client_sessions.end()){
		if (m_client_sessions[session]->IsTrading()){
			Transmission::Utils::SendStartTradeResultInfo(session, Transmission::TradingNow);
		}
		else{
			Transmission::ErrorCode code;
			if (m_client_sessions[session]->StartTrade(instru, strategyName, code)){
				Transmission::Utils::SendStartTradeResultInfo(session, Transmission::Succeed);
			}
			else{
				Transmission::Utils::SendStartTradeResultInfo(session, code);
			}
		}

	}
	else{
		Transmission::Utils::SendStartTradeResultInfo(session, Transmission::LoginNeeded);
	}
}


void ClientSessionMgr::LogoutAccount(const std::shared_ptr<Transmission::socket_session>& session){
	auto AccountIter = m_client_sessions.find(session);
	if (AccountIter != m_client_sessions.end())// logged
	{
		m_client_sessions.erase(session);
	}
}

void ClientSessionMgr::StopTrade(const std::shared_ptr<Transmission::socket_session>& session){
	auto AccountIter = m_client_sessions.find(session);
	if (AccountIter != m_client_sessions.end())// logged
	{
		AccountIter->second->StopTrade();
	}
}