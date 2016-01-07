#pragma once
#include <string>
#include <vector>

class IPositionControl;
class Strategy;

class Account
{
public:
	Account();
	Account(const std::string& un, const std::string& pw);
	~Account();
private:
	bool                              m_anybody;
	std::string                       m_username;
	std::string                       m_password;
	std::vector<std::string>          m_instrumentList;
	std::vector<Strategy*>            m_strategyList;
	std::vector<IPositionControl*>    m_positionControlList;
};

