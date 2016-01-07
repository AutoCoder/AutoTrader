#include "stdafx.h"
#include "Account.h"
#include "AccountPool.h"

AccountPool::AccountPool()
{
}


void AccountPool::AddAccount(const std::shared_ptr<Account>& newAcc){
	m_pool[newAcc->Id()] = newAcc;
}