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
		return true;
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

bool Account::StartTrade(const std::string& instru, int strategyId, int PositionCtlId){
	if (m_isTrading)
		return false;

	//verify arguments
	if (std::find(m_instrumentList.begin(), m_instrumentList.end(), instru) != m_instrumentList.end()
		&& std::find(m_strategyList.begin(), m_strategyList.end(), strategyId) != m_strategyList.end()
		&& std::find(m_positionControlList.begin(), m_positionControlList.end(), PositionCtlId) != m_positionControlList.end())
	{
		//todo: start the traderprocessor, register strategy
		return true;
	}
	else{
		return false;
	}
}

void Account::StopTrade(){

}