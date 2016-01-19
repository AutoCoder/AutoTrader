#include "stdafx.h"
#include "ClientSession.h"
#include "ClientSessionMgr.h"
#include "remote_server_action.h"
#include "fifo_action_queue.h"
#include "ThostFtdcTraderApi.h"
#include "ConfigV2.h"
#include "AccontMetaMgr.h"

static const char* LoginSucceed = "Login Successfully!\n";
static const char* LoginRepeatedly = "Login Repeatedly!\n";
static const char* LoginFailed_PW = "Login fail, Reason:Wrong password.\n";
static const char* LoginFailed_AC = "Login fail, Reason:unexisted account.\n";
static const char* TradingNow = "Trading now, Please stop current trading first.\n";
static const char* LoginNeeded = "Please login first before start to trade.\n";
static const char* StartTradeSucceed = "Start To Trade Successfully.\n";
static const char* InvalidTradeArguments = "Trade arguments is invalid.\n";

namespace {
	void AppendReplyBySession(const std::shared_ptr<Transmission::socket_session>& session, const std::string& data){
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, data));
	}
}


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
	//auto Account = GetAccount(userId);
	bool isExisted = Account::Manager::Instance().IsAccountExisted(userId);
	if (isExisted){
		bool success = Account::Manager::Instance().CheckPassword(userId, pw);
		if (success){
			if (m_client_sessions.find(session) == m_client_sessions.end()){
				m_client_sessions[session] = std::make_shared<ClientSession>(userId, session, m_pTradeUserApi);
				AppendReplyBySession(session, LoginSucceed);
			}
			else{
				AppendReplyBySession(session, LoginRepeatedly);
			}
		}
		else{
			AppendReplyBySession(session, LoginFailed_PW);
		}
	}
	else{
		AppendReplyBySession(session, LoginFailed_AC);
	}
}

void ClientSessionMgr::StartTrade(const std::string& instru, const std::string& strategyName, const std::shared_ptr<Transmission::socket_session>& session){
	if (m_client_sessions.find(session) != m_client_sessions.end()){
		if (m_client_sessions[session]->IsTrading()){
			AppendReplyBySession(session, TradingNow);
		}
		else{
			std::string err_string;
			if (m_client_sessions[session]->StartTrade(instru, strategyName, err_string)){
				AppendReplyBySession(session, StartTradeSucceed);
			}
			else{
				AppendReplyBySession(session, err_string);
			}
		}

	}
	else{
		AppendReplyBySession(session, LoginNeeded);
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