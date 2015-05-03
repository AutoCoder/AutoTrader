#ifndef PRINTER_H_
#define PRINTER_H_

#include "config.h"
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "mdspi.h"
#include "traderspi.h"
#include "windows.h"
#include <iostream>
using namespace std;

//
//
//
//void ShowTraderCommand(CtpTraderSpi* p, bool print = false){
//	if (print){
//		cerr << "-----------------------------------------------" << endl;
//		cerr << " [1] ReqUserLogin              -- 登录" << endl;
//		cerr << " [2] ReqSettlementInfoConfirm  -- 结算单确认" << endl;
//		cerr << " [3] ReqQryInstrument          -- 查询合约" << endl;
//		cerr << " [4] ReqQryTradingAccount      -- 查询资金" << endl;
//		cerr << " [5] ReqQryInvestorPosition    -- 查询持仓" << endl;
//		cerr << " [6] ReqOrderInsert            -- 报单" << endl;
//		cerr << " [7] ReqOrderAction            -- 撤单" << endl;
//		cerr << " [8] PrintOrders               -- 显示报单" << endl;
//		cerr << " [9] PrintOrders               -- 显示成交" << endl;
//		cerr << " [0] Exit                      -- 退出" << endl;
//		cerr << "----------------------------------------------" << endl;
//	}
//	TThostFtdcBrokerIDType	appId;
//	TThostFtdcUserIDType	userId;
//	TThostFtdcPasswordType	passwd;
//	TThostFtdcInstrumentIDType instId;
//	TThostFtdcDirectionType dir;
//	TThostFtdcCombOffsetFlagType kpp;
//	TThostFtdcPriceType price;
//	TThostFtdcVolumeType vol;
//	TThostFtdcSequenceNoType orderSeq;
//
//	int cmd;  cin >> cmd;
//	switch (cmd){
//	case 1: {
//				cerr << " BrokerID > "; cin >> appId;
//				cerr << " UserID > "; cin >> userId;
//				cerr << " Password > "; cin >> passwd;
//				p->ReqUserLogin(appId, userId, passwd); break;
//	}
//	case 2: p->ReqSettlementInfoConfirm(); break;
//	case 3: {
//				cerr << " Instrument > "; cin >> instId;
//				p->ReqQryInstrument(instId); break;
//	}
//	case 4: p->ReqQryTradingAccount(); break;
//	case 5: {
//				cerr << " Instrument > "; cin >> instId;
//				p->ReqQryInvestorPosition(instId); break;
//	}
//	case 6: {
//				cerr << " Instrument > "; cin >> instId;
//				cerr << " fangxiang > "; cin >> dir;
//				cerr << " kaiping > "; cin >> kpp;
//				cerr << " price > "; cin >> price;
//				cerr << " count > "; cin >> vol;
//				p->ReqOrderInsert(instId, dir, kpp, price, vol); break;
//	}
//	case 7: {
//				cerr << " index > "; cin >> orderSeq;
//				p->ReqOrderAction(orderSeq); break;
//	}
//	case 8: p->PrintOrders(); break;
//	case 9: p->PrintTrades(); break;
//	case 0: exit(0);
//	}
//	WaitForSingleObject(g_hEvent, INFINITE);
//	ResetEvent(g_hEvent);
//	ShowTraderCommand(p);
//}
//
//
//void ShowMdCommand(CtpMdSpi* p, bool print = false){
//	if (print){
//		cerr << "-----------------------------------------------" << endl;
//		cerr << " [1] ReqUserLogin              -- login" << endl;
//		cerr << " [2] SubscribeMarketData       -- md subscribe" << endl;
//		cerr << " [0] Exit                      -- quit" << endl;
//		cerr << "----------------------------------------------" << endl;
//	}
//	TThostFtdcBrokerIDType appId;
//	TThostFtdcUserIDType	 userId;
//	TThostFtdcPasswordType passwd;
//	char instIdList[100];
//
//	int cmd;  cin >> cmd;
//	switch (cmd){
//	case 1: {
//				cerr << " BrokerID > "; cin >> appId;
//				cerr << " UserID > "; cin >> userId;
//				cerr << " Password > "; cin >> passwd;
//				p->ReqUserLogin(appId, userId, passwd); break;
//	}
//	case 2: {
//				WaitForSingleObject(g_hEvent, INFINITE);
//				ResetEvent(g_hEvent);
//				cerr << " BrokerID > "; cin >> appId;
//				cerr << " UserID > "; cin >> userId;
//				cerr << " Password > "; cin >> passwd;
//				//ResetEvent(g_hEvent);
//				p->ReqUserLogin(appId, userId, passwd);
//				WaitForSingleObject(g_hEvent, INFINITE);
//				ResetEvent(g_hEvent);
//				cerr << " Instrument > "; cin >> instIdList;
//				p->SubscribeMarketData(instIdList); break;
//				WaitForSingleObject(g_hEvent, INFINITE);
//				ResetEvent(g_hEvent);
//	}
//	case 0: exit(0);
//	}
//
//	//ShowMdCommand(p);
//}

//void ShowRiskCommand(CtpRiskSpi* p, bool print = false){
//	if (print){
//		cerr << "-----------------------------------------------" << endl;
//		cerr << " [1] ReqUserLogin              -- 登录" << endl;
//		cerr << " [2] ReqRiskQryBrokerDeposit   -- 查询席位资金" << endl;
//		cerr << " [3] ReqRiskOrderInsert        -- 强平" << endl;
//		cerr << " [4] PrintAcounts              -- 显示资金账户" << endl;
//		cerr << " [5] PrintPositions            -- 显示持仓信息" << endl;
//		cerr << " [0] Exit                      -- 退出" << endl;
//		cerr << "----------------------------------------------" << endl;
//	}
//
//	TShfeFtdcBrokerIDType	appId;
//	TShfeFtdcUserIDType	 userId;
//	TShfeFtdcPasswordType	passwd;
//	TShfeFtdcExchangeIDType exchangeId;
//	TShfeFtdcSequenceNoType seqNo;
//	TShfeFtdcPriceType price;
//	int cmd;  cin >> cmd;
//	switch (cmd){
//	case 1: {
//				cerr << " 应用单元 > "; cin >> appId;
//				cerr << " 操作员 > "; cin >> userId;
//				cerr << " 密码 > "; cin >> passwd;
//				p->ReqUserLogin(appId, userId, passwd); break;
//	}
//	case 2: {
//				cerr << " 交易所 > "; cin >> exchangeId;
//				p->ReqRiskQryBrokerDeposit(exchangeId); break;
//	}
//	case 3: {
//				cerr << " 序号 > "; cin >> seqNo;
//				cerr << " 价格 > "; cin >> price;
//				p->ReqRiskOrderInsert(seqNo, price); break;
//	}
//	case 4: p->PrintAcounts(); break;
//	case 5: p->PrintPositions(); break;
//	case 0: exit(0);
//	}
//	WaitForSingleObject(g_hEvent, INFINITE);
//	ResetEvent(g_hEvent);
//	ShowRiskCommand(p);
//}
#endif //PRINTER_H_