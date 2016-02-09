#ifndef TRANSMISSION_H
#define TRANSMISSION_H

namespace Transmission{

	enum TradeEventType{
		ACCOUNT_INIT,
		INSERT_ORDER,
		CANCELL_ORDER,
		TRADE,
	};

	enum ErrorCode{
		Succeed = 0,
		LoginRepeatedly = 1,
		LoginFailed_PW = 2,
		LoginFailed_AC = 3,
		TradingNow = 4,
		NoTradingNow = 5,
		LoginNeeded = 6,
		InvalidTradeArguments = 7,
		Unknown = 8,
	};
}

#endif