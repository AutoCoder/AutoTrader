// AutoTrader.cpp : Defines the entry point for the console application.
//

#include "tradespi.h"
#include "Config.h"
#include "crossplatform.h"
#include "InstrumentInfoMgr.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

int main(int argc, const char* argv[]){
	auto console = spdlog::stdout_logger_mt("console");
	auto config = Config::Instance();

	CThostFtdcTraderApi* trade_api = CThostFtdcTraderApi::CreateFtdcTraderApi();
	assert(trade_api);

	CtpTradeSpi* trade_spi = new CtpTradeSpi(trade_api, config->DefaultCtpBrokerID().c_str(), config->DefaultCtpUserID().c_str(), config->DefaultCtpPassword().c_str(), "LyATTrader");

	trade_api->RegisterSpi((CThostFtdcTraderSpi*)trade_spi); 
	trade_api->SubscribePublicTopic(THOST_TERT_RESTART);
	trade_api->SubscribePrivateTopic(THOST_TERT_RESTART);
	trade_api->RegisterFront(const_cast<char*>(Config::Instance()->CtpTradeFront().c_str()));
	trade_api->Init();
	SYNC_PRINT << "Start to update the factor of instruments";

	trade_spi->WaitQueryFinshed();
	InstrumentManager.serialize();
	return 0;
}
