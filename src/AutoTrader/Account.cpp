#include "stdafx.h"
#include "Account.h"


Account::Account()
	:m_anybody(true)
{
}


Account::Account(const std::string& un, const std::string& pw)
	: m_username(un)
	, m_password(pw)
	, m_anybody(false)
{
}

Account::~Account()
{
	m_instrumentList.clear();
	m_strategyList.clear();
	m_positionControlList.clear();
}
