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

		static void SendMDInfo(const std::shared_ptr<Transmission::socket_session>& session, int openPrice, int closePrice, int highPrice, int lowPrice, long long timestamp);
		static void SendDealInfo(const std::shared_ptr<Transmission::socket_session>& session, TradeEventType type, int direction, int price, int vol, char*  orderRef, long long timestamp);
		static void SendLoginResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code);
		static void SendLogOutResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code);
		static void SendAccountStatus(const std::shared_ptr<Transmission::socket_session>& session, double blance, int position, const std::string& instrument, int currentPrice);
		static void SendAccountInfo(const std::shared_ptr<Transmission::socket_session>& session, const std::vector<std::string>& instruments, const std::vector<std::string>& strategies);
		static void SendStartTradeResultInfo(const std::shared_ptr<Transmission::socket_session>& session, ErrorCode code);
	};
}
#endif