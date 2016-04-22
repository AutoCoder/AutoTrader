#include "stdafx.h"
#include "Utils.h"
#include "json/json.h"
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

	void Utils::SendMDInfo(const std::shared_ptr<Transmission::socket_session>& session, int openPrice, int closePrice, int highPrice, int lowPrice, int vol,  long long timestamp){
		Json::Value root;
		root["Info"] = "MD";
		root["Details"] = Json::Value::nullRef;
		root["Details"]["OpenPrice"] = openPrice;
		root["Details"]["LastPrice"] = closePrice;
		root["Details"]["HighPrice"] = highPrice;
		root["Details"]["LowPrice"] = lowPrice;
		root["Details"]["TIMESTAMP"] = timestamp;
		root["Details"]["Volume"] = vol;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}

	void Utils::SendTechInfo(const std::shared_ptr<Transmission::socket_session>& session, const std::string& jsondata, long long timestamp){
		Json::Value root;
		root["Info"] = "TECH";
		root["Details"] = jsondata;
		root["Details"]["TIMESTAMP"] = timestamp;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}

	void Utils::SendPositionInfo(const std::shared_ptr<Transmission::socket_session>& session, double blance, int posmoney, const std::string& detailList){
		Json::Value root;
		root["Info"] = "POSITION_INFO";
		Json::Value details;
		details["Balance"] = blance;
		details["PositionMoney"] = posmoney;
		details["PositionOfInstruments"] = detailList;
		root["Details"] = details;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		//{"Type":"ACCOUNT_STATUS","Details":{"Blance":122313,"Position":20, "Instrument":"rb1605", "Price":2555}}
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}

	void Utils::SendAccountInfo(const std::shared_ptr<Transmission::socket_session>& session, const std::vector<std::string>& instruments, const std::vector<std::string>& strategies, bool isTrading, const std::string& runningInstru, const std::string& runningStrgy){
		Json::Value root;
		root["Info"] = "ACCOUNT_INFO";
		Json::Value details;
		Json::Value arrayInstru;
		Json::Value arrayStrategy;
		for (auto instru : instruments){
			arrayInstru.append(instru);
		}
		for (auto st : strategies){
			arrayStrategy.append(st);
		}
		details["Instruments"] = arrayInstru;
		details["Strategies"] = arrayStrategy;
		details["IsTrading"] = isTrading;
		details["RunningInstrument"] = runningInstru;
		details["RunningStrategy"] = runningStrgy;

		root["Details"] = details;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		//{"Type":"ACCOUNT_INFO","Details":{"Instruments": [ "122313", "12233" ], "Strategies": [ "CROSS_3_5" ], "IsTrading" : true, "RunningInstrument" : "", "RunningStrategy" : ""}}
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}

	void Utils::SendDealInfo(const std::shared_ptr<Transmission::socket_session>& session, TradeEventType type, const char* instrument, int direction, char combOffsetFlag, int price, int vol, char* orderRef, long long timestamp){
		Json::Value root;
		switch (type){
		case INSERT_ORDER:
			root["Info"] = "INSERT_ORDER";
			break;
		case CANCELL_ORDER:
			root["Info"] = "CANCELL_ORDER";
			break;
		case TRADE:
			root["Info"] = "TRADE";
			break;
		default:
			assert(false);
			break;
		}
		root["Details"] = Json::Value::nullRef;
		root["Details"]["Direction"] = (direction == '0' ? 0 : 1);
		root["Details"]["OffsetFlag"] = (int)combOffsetFlag;
		root["Details"]["Price"] = price;
		root["Details"]["Vol"] = vol;
		root["Details"]["ORDER_ID"] = orderRef;
		root["Details"]["TIMESTAMP"] = timestamp;
		root["Details"]["Instrument"] = instrument;
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
		root["Action"] = "Login";
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
			case Succeed:
				err_msg = "Succeed.";
				break;
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
		root["Action"] = "StartTrade";
		root["ErrorCode"] = code;
		root["ErrorMsg"] = err_msg;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}

	void Utils::SendLogOutResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code){
		std::string err_msg;
		int err_code = code;
		switch (code){
			case Succeed:
				err_msg = "Succeed";
				break;
			default:
			{
				err_code = -1;
				err_msg = "Unknown.";
			}
		}
		Json::Value root;
		root["Action"] = "Logout";
		root["ErrorCode"] = code;
		root["ErrorMsg"] = err_msg;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}

	void Utils::SendStopTradeResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code){
		std::string err_msg;
		int err_code = code;
		switch (code){
		case Succeed:
			err_msg = "Succeed";
			break;
		default:
		{
			err_code = -1;
			err_msg = "Unknown.";
		}
		}
		Json::Value root;
		root["Action"] = "StopTrade";
		root["ErrorCode"] = code;
		root["ErrorMsg"] = err_msg;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}

	void Utils::SendQueryPositionResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code){
		std::string err_msg;
		int err_code = code;
		switch (code){
		case Succeed:
			err_msg = "Succeed";
			break;
		case PositionInfoIsNotReady:
			err_msg = "The postion is not ready to return.";
			break;
		default:
		{
			err_code = -1;
			err_msg = "Unknown.";
		}
		}
		Json::Value root;
		root["Action"] = "QueryPosition";
		root["ErrorCode"] = code;
		root["ErrorMsg"] = err_msg;
		Json::FastWriter writer;
		std::string ret = writer.write(root);
		ret = str(boost::format("%1%%2%") % ret.length() % ret);
		Transmission::GetFIFOActionQueue().Push_back(Transmission::RemoteServerAction(session, ret));
	}
}
