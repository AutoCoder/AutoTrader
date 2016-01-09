#include "stdafx.h"
#include "Account.h"
#include "AccountMgr.h"
#include "remote_server_action.h"
#include "fifo_action_queue.h"

static const char* LoginSucceed = "Login Successfully!";
static const char* LoginRepeatedly = "Login Repeatedly!";
static const char* LoginFailed_PW = "Login fail, Reason:Wrong password.";
static const char* LoginFailed_AC = "Login fail, Reason:unexisted account.";
static const char* TradingNow = "Trading now, Please stop current trading first.";
static const char* LoginNeeded = "Please login first before start to trade.";
static const char* StartTradeSucceed = "Start To Trade Successfully";
static const char* InvalidTradeArguments = "Trade arguments is invalid.";

namespace {
	void AppendReplyBySession(const std::shared_ptr<Transmission::socket_session>& session, const std::string& data){
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, data));
	}
}


AccountMgr* AccountMgr::_instance = NULL;

AccountMgr* AccountMgr::getInstance(){
	return _instance;
}

void AccountMgr::AddAccount(const std::shared_ptr<Account>& newAcc){
	m_pool[newAcc->Id()] = newAcc;
}

void AccountMgr::LoginAccount(const std::string& userId, const std::string& pw, const std::shared_ptr<Transmission::socket_session>& session){
	auto accout = GetAccount(userId);

	if (accout){
		if (accout->IsLogged())
			AppendReplyBySession(session, LoginRepeatedly);

		if(accout->Login(session, pw) == false)
			AppendReplyBySession(session, LoginFailed_PW);
		else{
			assert(m_pool.find(userId) != m_pool.end());
			m_LoggedAccount[session] = m_pool[userId];
			AppendReplyBySession(session, LoginSucceed);
		}	
	}
	else{
		AppendReplyBySession(session, LoginFailed_AC);
	}
}

void AccountMgr::StartTrade(const std::string& instru, int strategyId, int PositionCtlId, const std::shared_ptr<Transmission::socket_session>& session){
	auto accoutIter = m_LoggedAccount.find(session);
	if (accoutIter != m_LoggedAccount.end())// logged
	{
		if (accoutIter->second->IsTrading()){
			AppendReplyBySession(session, TradingNow);
			return;
		}
		if (accoutIter->second->StartTrade(instru, strategyId, PositionCtlId)){//succeed
			AppendReplyBySession(session, StartTradeSucceed);
		}
		else{//failed
			AppendReplyBySession(session, InvalidTradeArguments);
		}
	}
	else{
		AppendReplyBySession(session, LoginNeeded);
	}
}