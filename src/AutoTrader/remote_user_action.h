#ifndef USER_ACTION_H
#define USER_ACTION_H

#include <vector>
#include <string>

namespace Transmission{


	class socket_session;

	struct LoginActionMeta{
		bool Parse(const std::string& input);

		std::string Id() {
			return brokerId + userName;
		}

		std::string brokerId;
		std::string userName;
		std::string passWord;
	};

	struct TradeActionMeta{
		bool Parse(const std::string& input);

		std::string strategyName;
		std::string instrumentId;
	};

	class RemoteUserAction
	{
	public:
		RemoteUserAction();
		~RemoteUserAction();

		enum class Parse_Result {
			Good,
			Bad,
			indeterminate
		};

		enum class ActionType{
			Invalid,
			Login,
			Logout,
			StartTrade,
			StopTrade
		};

		Parse_Result Parse(char* begin, char* end);
		void Invoke();
		void Reset();

		void SetSession(const std::shared_ptr<socket_session>& session);

	private:
		size_t                            length_;
		std::string						  original_data_;
		ActionType                        action_type_;
		std::shared_ptr<socket_session>	  session_;
		std::shared_ptr<LoginActionMeta>  login_meta_;
		std::shared_ptr<TradeActionMeta>  trade_meta_;
	};
}
#endif 