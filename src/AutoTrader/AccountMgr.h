#ifndef ACCOUNT_POOL_H
#define ACCOUNT_POOL_H
#include <string>
#include <map>
#include <memory>
#include "socket_session.h"

class Account;
class CThostFtdcTraderApi;

//*********************************************************
//This singleton must be launched before mutli thread start
//*********************************************************
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

	CThostFtdcTraderApi* TradeApiPtr() const { return m_pTradeUserApi; }

	void AddAccount(const std::shared_ptr<Account>& newAcc);

	void LoginAccount(const std::string& userId, const std::string& pw, const std::shared_ptr<Transmission::socket_session>& session);

	void StartTrade(const std::string& instru, const std::string& strategyName, const std::shared_ptr<Transmission::socket_session>& session);

	void LogoutAccount(const std::shared_ptr<Transmission::socket_session>& session);

	void StopTrade(const std::shared_ptr<Transmission::socket_session>& session);

private:
	AccountMgr();
	AccountMgr(const AccountMgr&) = delete;
	AccountMgr& operator=(const AccountMgr &) = delete;
	static AccountMgr *_instance;

private:
	CThostFtdcTraderApi*															   m_pTradeUserApi;
	std::map<std::string/*id*/, std::shared_ptr<Account> >                             m_pool;
	std::map<std::shared_ptr<Transmission::socket_session>, std::shared_ptr<Account>>  m_LoggedAccount;
};

#endif