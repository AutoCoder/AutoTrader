#include "stdafx.h"
#include <assert.h>
#include "json/json.h"
#include "ClientSession.h"
#include "ClientSessionMgr.h"
#include "remote_user_action.h"
#include "socket_session.h"




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

//96{"ActionType":"Login","Arguments":{"BrokerId":"9999","UserName":"021510","Password":"wodemima"}}

//Start Trade Action
//{
//	"ActionType" : "StartTrade", or "StopTrade"
//	"Arguments" : 
//	{
//		"InstrumentId" : "rb1604",
//      "StrategyName" : "Pos20Precent_3_5_MACrossStratgy"
//	}
//}

//114{"ActionType":"StartTrade","Arguments":{"InstrumentId":"rb1604","StrategyName":"Pos20Precent_3_5_MACrossStratgy"}}

namespace Transmission{

	std::string ReadActionType(const std::string& input){
		Json::Reader reader;

		Json::Value root;

		if (reader.parse(input, root))
		{
			auto& actionTypeNode = root["Action"];
			return actionTypeNode.isNull() ? "" : actionTypeNode.asString();
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

			auto readfun = [&arguments](const std::string& subName, std::string& out) -> bool {
				auto& subNode = arguments[subName];
				if (subNode.isNull())
					return false;
				else{
					out = subNode.asString();
					return true;
				}
			};

			return readfun("InstrumentId", instrumentId) && readfun("StrategyName", strategyName);
		}
		else{
			return false;
		}
	}

	RemoteUserAction::RemoteUserAction()
		: length_(0)
	{
	}


	RemoteUserAction::~RemoteUserAction()
	{
	}

	void RemoteUserAction::SetSession(const std::shared_ptr<socket_session>& session){
		session_ = session;
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
					login_meta_ = std::unique_ptr<LoginActionMeta>(new LoginActionMeta());
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
					trade_meta_ = std::unique_ptr<TradeActionMeta>(new TradeActionMeta());
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
				else if (actionType == "QueryPosition"){
					action_type_ = ActionType::QueryPosition;
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
				//execute login
				ClientSessionMgr::getInstance()->LoginAccount(login_meta_->Id(), login_meta_->passWord, session_);
			}
			break;
			case ActionType::StartTrade:
			{
				ClientSessionMgr::getInstance()->StartTrade(trade_meta_->instrumentId, trade_meta_->strategyName, session_);
			}
			break;
			case ActionType::Logout:
			{
				//todo: execute loginOUT
				ClientSessionMgr::getInstance()->LogoutAccount(session_);
			}
			break;
			case ActionType::StopTrade:
			{
				ClientSessionMgr::getInstance()->StopTrade(session_);
			}
			break;
			case ActionType::QueryPosition:
			{
				ClientSessionMgr::getInstance()->QueryPosition(session_);
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