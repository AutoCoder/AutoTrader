#ifndef TRANSMISSION_UTILS_H
#define TRANSMISSION_UTILS_H

#include <memory>
#include <vector>

#include "Transmission.h"

namespace Transmission{

	class socket_session;

	typedef std::shared_ptr<socket_session> SockSessionSP;

	class Utils
	{
	public:
		Utils();
		~Utils();

		/* extradata:
		 *	{ "Type" : "MA", "Short" : 1000, "Long" : 2000 }
		 */
		static void SendMDInfo(const SockSessionSP& session, int preSettlementPrice,\
			int openPrice, int closePrice, int highPrice, int lowPrice, int vol, int total_vol, double turnover, long long timestamp, \
			const std::string& instru, const std::string& extradata);
		static void SendDealInfo(const SockSessionSP& session, TradeEventType type, const char* instrument, int direction, char combOffsetFlag, int price, int vol, char*  orderRef, long long timestamp);
		static void SendLoginResultInfo(const SockSessionSP& session, ErrorCode code);
		static void SendLogOutResultInfo(const SockSessionSP& session, ErrorCode code);
		static void SendPositionInfo(const SockSessionSP& session, double blance, int posmoney, const std::string& details);
		static void SendAccountInfo(const SockSessionSP& session, const std::vector<std::string>& instruments, const std::vector<std::string>& strategies, bool isTrading, const std::string& instru, const std::string& strategy);
		static void SendStartTradeResultInfo(const SockSessionSP& session, ErrorCode code);
		static void SendStopTradeResultInfo(const SockSessionSP& session, ErrorCode code);
		static void SendQueryPositionResultInfo(const SockSessionSP& session, ErrorCode code);
	};
}
#endif
