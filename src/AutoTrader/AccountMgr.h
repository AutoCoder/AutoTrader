#ifndef ACCOUNT_POOL_H
#define ACCOUNT_POOL_H
#include <string>
#include <map>
#include <memory>
//#include "socket_server.h"

class Account;

class AccountMgr
{
public:
	~AccountMgr(){};
	static AccountMgr* getInstance();

public:
	std::shared_ptr<Account> GetAccount(const std::string& userId) const {
		if (m_pool.find(userId) != m_pool.end())
			return m_pool.at(userId);
		else
			return NULL;
	}

	//std::shared_ptr<Account> GetAccount(Transmission::socket_session_ptr sessionPtr) const {
	//	if (m_pool.find(sessionPtr) != m_pool.end())
	//		return m_pool.at(sessionPtr);
	//	else
	//		return NULL;
	//}

	void AddAccount(const std::shared_ptr<Account>& newAcc);

private:
	AccountMgr(){}
	AccountMgr(const AccountMgr&) = delete;
	AccountMgr& operator=(const AccountMgr &) = delete;
	static AccountMgr *_instance;

private:
	std::map<std::string/*id*/, std::shared_ptr<Account> >                m_pool;
	//std::map<Transmission::socket_session_ptr, std::shared_ptr<Account> > m_AccountIndex;
};

#endif