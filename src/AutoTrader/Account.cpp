#include "stdafx.h"
#include "Account.h"


Account::Account()
	:m_isLogin(true)
{
}


Account::Account(const std::string& brokerId, const std::string& un, const std::string& pw)
	: m_brokderId(brokerId)
	, m_ctp_username(un)
	, m_ctp_password(pw)
	, m_isLogin(false)
	, m_isTrading(false)
{
}

Account::~Account()
{
	m_instrumentList.clear();
	m_strategyList.clear();
	m_positionControlList.clear();
}


bool Account::Login(const std::shared_ptr<Transmission::socket_session>& s, const std::string& pw) {
	if (pw == m_ctp_password){
		m_session = s;
		m_isLogin = true;
	}
	else{
		return false;
	}
}

bool Account::Logout() {
	m_session.reset();
	m_isLogin = false;

	StopTrade();// must
	return true;
}

void Account::StartTrade(const std::string& instru, int strategyId, int PositionCtlId){
	//todo: verify arguments, start the traderprocessor, register strategy
}

void Account::StopTrade(){

}