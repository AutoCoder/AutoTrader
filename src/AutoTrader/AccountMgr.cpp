#include "stdafx.h"
#include "Account.h"
#include "AccountMgr.h"


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
		accout->Login(session, pw);
	}
	else{
		session->do_write(LoginFailed_AC, strlen(LoginFailed_AC));
	}
}