#include "stdafx.h"
#include "Utils.h"
#include "json\json.h"
#include <boost/format.hpp>
#include "fifo_action_queue.h"
#include "remote_server_action.h"

namespace Transmission{

	Utils::Utils()
	{
	}


	Utils::~Utils()
	{
	}

	void Utils::SendMDInfo(const std::shared_ptr<Transmission::socket_session>& session, int openPrice, int closePrice, int highPrice, int lowPrice, long long timestamp){
		Json::Value root;
		root["Type"] = "MD";
		root["Details"] = Json::Value::nullRef;
		root["Details"]["OpenPrice"] = openPrice;
		root["Details"]["ClosePrice"] = closePrice;
		root["Details"]["HighPrice"] = highPrice;
		root["Details"]["LowPrice"] = lowPrice;
		root["Details"]["TIMESTAMP"] = timestamp;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}

	void Utils::SendDealInfo(const std::shared_ptr<Transmission::socket_session>& session, TradeEventType type, int direction, int price, int vol, char* orderRef, long long timestamp){
		Json::Value root;
		switch (type){
		case INSERT_ORDER:
			root["Type"] = "INSERT_ORDER";
			break;
		case CANCELL_ORDER:
			root["Type"] = "CANCELL_ORDER";
			break;
		case TRADE:
			root["Type"] = "TRADE";
		default:
			assert(false);
			break;
		}
		root["Details"] = Json::Value::nullRef;
		root["Details"]["Direction"] = direction;
		root["Details"]["Price"] = price;
		root["Details"]["Vol"] = vol;
		root["Details"]["ORDER_ID"] = orderRef;
		root["Details"]["TIMESTAMP"] = timestamp;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}

	void Utils::SendLoginResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code){
		std::string err_msg;
		int err_code = code;
		switch (code){
			case Succeed:
				err_msg = "Succeed";
				break;
			case LoginRepeatedly:
				err_msg = "Already logined.";
				break;
			case LoginFailed_PW:
				err_msg = "Wrong password.";
				break;
			case LoginFailed_AC:
				err_msg = "Unexisted Account.";
				break;
			default:
			{
				err_code = -1;
				err_msg = "Unknown.";
			}
		}
		Json::Value root;
		root["Type"] = "Login";
		root["ErrorCode"] = code;
		root["ErrorMsg"] = err_msg;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}

	void Utils::SendStartTradeResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code){
		std::string err_msg;
		int err_code = code;
		switch (code){
			case TradingNow:
				err_msg = "Trading now, Please stop current trading first.";
				break;
			case LoginNeeded:
				err_msg = "Please login first before start to trade.";
				break;
			case InvalidTradeArguments:
				err_msg = "Trade arguments is invalid.";
				break;
			default:
			{
				err_code = -1;
				err_msg = "Unknown.";
			}
		}
		Json::Value root;
		root["Type"] = "StartTrade";
		root["ErrorCode"] = code;
		root["ErrorMsg"] = err_msg;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}
}