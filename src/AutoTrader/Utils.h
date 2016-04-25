#ifndef TRANSMISSION_UTILS_H
#define TRANSMISSION_UTILS_H

#include <memory>
#include <vector>
#include "socket_session.h"
#include "Transmission.h"

namespace Transmission{



	class Utils
	{
	public:
		Utils();
		~Utils();

		/* extradata:
		 *	{ "Type" : "MA", "Short" : 1000, "Long" : 2000 }
		 */
		static void SendMDInfo(const std::shared_ptr<Transmission::socket_session>& session, \
			int openPrice, int closePrice, int highPrice, int lowPrice, int vol, long long timestamp \
			const std::string& extradata);
		static void SendDealInfo(const std::shared_ptr<Transmission::socket_session>& session, TradeEventType type, const char* instrument, int direction, char combOffsetFlag, int price, int vol, char*  orderRef, long long timestamp);
		static void SendLoginResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code);
		static void SendLogOutResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code);
		static void SendPositionInfo(const std::shared_ptr<Transmission::socket_session>& session, double blance, int posmoney, const std::string& details);
		static void SendAccountInfo(const std::shared_ptr<Transmission::socket_session>& session, const std::vector<std::string>& instruments, const std::vector<std::string>& strategies, bool isTrading, const std::string& instru, const std::string& strategy);
		static void SendStartTradeResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code);
		static void SendStopTradeResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code);
		static void SendQueryPositionResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code);
	};
}
#endif