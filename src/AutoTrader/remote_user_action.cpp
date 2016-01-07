#include "stdafx.h"
#include <assert.h>
#include "json/json.h"
#include "Account.h"
#include "AccountPool.h"
#include "remote_user_action.h"
//#include "socket_server_impl.h"
#include "socket_server.h"




namespace{
	bool is_digit(int c){
		return c >= '0' && c <= '9';
	}
}
//Login Action Socket
//{
//	"ActionType" : "Login", or "Logout"
//	"Arguments" : 
//	{
//		"BrokerId" " "9999",
//      "UserName" : "123",
//		"Password" : "123"
//	}
//}

//{"ActionType":"Login","Arguments":{"BrokerId":"9999","UserName":"123","Password":"123"}}

//Start Trade Action
//{
//	"ActionType" : "StartTrade", or "StopTrade"
//	"Arguments" : 
//	{
//		"InstrumentId" : "rb1604",
//      "StrategyId" : 1,
//		"PositionCtlId" : 0
//	}
//}

//{"ActionType":"StartTrade","Arguments":{"InstrumentId":"rb1604","StrategyId":1,"PositionCtlId":0}}

namespace Transmission{

	std::string ReadActionType(const std::string& input){
		Json::Reader reader;

		Json::Value root;

		if (reader.parse(input, root))
		{
			auto& actionTypeNode = root["ActionType"];
			return actionTypeNode.isNull() ? actionTypeNode.asString() : "";
		}
		else
			return "";
	};

	bool LoginActionMeta::Parse(const std::string& input)
	{
		Json::Reader reader;

		Json::Value root;

		if (reader.parse(input, root))
		{
			auto& arguments = root["Arguments"];
			if (arguments.isNull())
				return false;

			auto readfun = [&arguments](const std::string& subName, std::string& out) -> bool {
				auto& subNode = arguments[subName];
				if (subNode.isNull())
					return false;
				else{
					out = subNode.asString();
					return true;
				}
			};

			return readfun("BrokerId", brokerId) && readfun("UserName", userName) && readfun("Password", passWord);
		}
		else{
			return false;
		}
	}

	bool TradeActionMeta::Parse(const std::string& input)
	{
		Json::Reader reader;

		Json::Value root;

		if (reader.parse(input, root))
		{
			auto& arguments = root["Arguments"];
			if (arguments.isNull())
				return false;

			auto& instruNode = arguments["InstrumentId"];
			if (instruNode.isNull())
				return false;
			else{
				instrumentId = instruNode.asString();
			}

			auto readfun = [&arguments](const std::string& subName, int& out) -> bool {
				auto& subNode = arguments[subName];
				if (subNode.isNull())
					return false;
				else{
					out = subNode.asInt();
					return true;
				}
			};

			return readfun("StrategyId", strategyId) && readfun("PositionCtlId", positionCtlId);
		}
		else{
			return false;
		}
	}

	RemoteUserAction::RemoteUserAction(socket_session* session)
		: length_(0)
		, session_(session)
	{
	}


	RemoteUserAction::~RemoteUserAction()
	{
	}


	RemoteUserAction::Parse_Result RemoteUserAction::Parse(char* begin, char* end){
		assert(begin);
		assert(end > begin);
		while (begin < end && original_data_.empty() && is_digit(*begin)){ // read header(msg length)
			length_ = 10 * length_ + (*begin - '0');
			begin++;
		}

		while (begin < end && original_data_.size() < length_){
			original_data_.push_back(*begin++);
		}

		if (begin == end)
		{
			if (original_data_.size() < length_){
				return Parse_Result::indeterminate;
			}
			else if (original_data_.size() == length_){
				//parse actiontype first
				const std::string& actionType = ReadActionType(original_data_);
				if (actionType == "Login"){
					login_meta_ = std::make_unique<LoginActionMeta>();
					if (login_meta_->Parse(original_data_)){
						action_type_ = ActionType::Login;
						return Parse_Result::Good;
					}
					else{
						action_type_ = ActionType::Invalid;
						return Parse_Result::Bad;
					}
				}
				else if (actionType == "StartTrade"){
					trade_meta_ = std::make_unique<TradeActionMeta>();
					if (trade_meta_->Parse(original_data_)){
						action_type_ = ActionType::StartTrade;
						return Parse_Result::Good;
					}
					else{
						action_type_ = ActionType::Invalid;
						return Parse_Result::Bad;
					}
				}
				else if (actionType == "Logout"){
					action_type_ = ActionType::Logout;
					return Parse_Result::Good;
				}
				else if (actionType == "StopTrade"){
					action_type_ = ActionType::StopTrade;
					return Parse_Result::Good;
				}
				else{
					action_type_ = ActionType::Invalid;
					return Parse_Result::Bad;
				}
			}
			else{
				//it's impossible original_data_.size() > length_
				assert(false);
				return Parse_Result::Bad;
			}
		}
		else{
			return Parse_Result::Bad;
		}
	}

	void RemoteUserAction::Invoke(){

		switch (action_type_){
			case ActionType::Login:
			{
				//todo: execute login
				auto accout = AccountPool::getInstance()->GetAccount(login_meta_->Id());
				accout->Login(std::make_shared<socket_session>(session_));
			}
			break;
			case ActionType::StartTrade:
			{
				//todo: execute startTrade
			}
			break;
			case ActionType::Logout:
			{
				//todo: execute loginOUT

			}
			break;
			case ActionType::StopTrade:
			{
				//todo: execute StopTrade
			}
			break;
			default:
				break;
		}

	}


	void RemoteUserAction::Reset(){
		original_data_.clear();
		length_ = 0;
	}

}