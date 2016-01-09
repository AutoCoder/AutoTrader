#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <vector>
#include <memory>

class IPositionControl;
class Strategy;

namespace Transmission{
	class socket_session;
};

class Account
{
public:
	Account();
	Account(const std::string& brokerId, const std::string& un, const std::string& pw);

	void AddInstrument(const std::string & instr){
		m_instrumentList.push_back(instr);
	}

	void AddStrategy(int strategyIdx){
		m_strategyList.push_back(strategyIdx);
	}

	void AddPositionControl(int pcIdx){
		m_positionControlList.push_back(pcIdx);
	}

	bool IsLogged() const { return m_isLogin; }

	bool IsTrading() const { return m_isTrading;  }

	bool Login(const std::shared_ptr<Transmission::socket_session>& s, const std::string& pw);
	bool Logout(); //identify User By session

	bool StartTrade(const std::string& instru, int strategyId, int PositionCtlId);

	void StopTrade();

	//Transmission::socket_session_ptr SessionPtr() const { return m_session.get(); }

	std::string Id() const { return m_ctp_username + m_brokderId; };

	~Account();
private:
	bool											m_isLogin;
	bool                                            m_isTrading;
	std::string										m_brokderId;
	std::string										m_ctp_username;
	std::string										m_ctp_password;
	std::vector<std::string>						m_instrumentList;
	std::vector<int>							    m_strategyList;
	std::vector<int>								m_positionControlList;
	std::shared_ptr<Transmission::socket_session>   m_session;
};

#endif