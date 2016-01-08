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

