#include "stdafx.h"
#include <iostream>
#include <vector>
#include "stdafx.h"
using namespace std;
#include "ThostFtdcTraderApi.h"
#include "traderspi.h"
#include "windows.h"

#pragma warning(disable :4996)

TThostFtdcBrokerIDType appId;		
TThostFtdcUserIDType userId;		


extern int requestId; 
extern HANDLE g_hEvent;

int	 frontId;	
int	 sessionId;	
char orderRef[13];

vector<CThostFtdcOrderField*> orderList;
vector<CThostFtdcTradeField*> tradeList;

char MapDirection(char src, bool toOrig);
char MapOffset(char src, bool toOrig);
    


void CtpTraderSpi::OnFrontConnected()
{
	cerr << __FUNCTION__ << endl;
  SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,
	        TThostFtdcUserIDType	vUserId,	TThostFtdcPasswordType	vPasswd)
{
  
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, vAppId); strcpy(appId, vAppId); 
	strcpy(req.UserID, vUserId);  strcpy(userId, vUserId); 
	strcpy(req.Password, vPasswd);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);
	cerr << " Request | send logging ..." << ((ret == 0) ? "success" : "fail") << endl;
}

void CtpTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if ( !IsErrorRspInfo(pRspInfo) && pRspUserLogin ) {  
    // ±£´æ»á»°²ÎÊý	
		frontId = pRspUserLogin->FrontID;
		sessionId = pRspUserLogin->SessionID;
		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		sprintf(orderRef, "%d", ++nextOrderRef);
    cerr<<" Response | login successfully...CurrentDate:"
      <<pRspUserLogin->TradingDay<<endl;     
  }
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(req.InvestorID, userId);
	int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++requestId);
	cerr << " Request | sending settlementInfo confirmation..." << ((ret == 0) ? "success" : "fail") << endl;
}

void CtpTraderSpi::OnRspSettlementInfoConfirm(
        CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, 
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	if( !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
    cerr<<" Response | settlementInfo..."<<pSettlementInfoConfirm->InvestorID
      <<"...<"<<pSettlementInfoConfirm->ConfirmDate
      <<" "<<pSettlementInfoConfirm->ConfirmTime<<">...Confirm"<<endl;
  }
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
  strcpy(req.InstrumentID, instId);
	int ret = pUserApi->ReqQryInstrument(&req, ++requestId);
	cerr << " Request | send Instrument Query..." << ((ret == 0) ? "success" : "fail") << endl;
}

void CtpTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, 
         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 	
	if ( !IsErrorRspInfo(pRspInfo) &&  pInstrument){
    cerr<<" Response | Instrument:"<<pInstrument->InstrumentID
      <<" DeliveryMonth:"<<pInstrument->DeliveryMonth
      <<" LongMarginRatio:"<<pInstrument->LongMarginRatio
      <<" ShortMarginRatio:"<<pInstrument->ShortMarginRatio<<endl;    
  }
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(req.InvestorID, userId);
	int ret = pUserApi->ReqQryTradingAccount(&req, ++requestId);
	cerr << " Request | send trading account query..." << ((ret == 0) ? "success" : "fail")<<endl;

}

void CtpTraderSpi::OnRspQryTradingAccount(
    CThostFtdcTradingAccountField *pTradingAccount, 
   CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	if (!IsErrorRspInfo(pRspInfo) &&  pTradingAccount){
    cerr<<" Response | Balance:"<<pTradingAccount->Balance
      <<" Available:"<<pTradingAccount->Available   
      <<" CurrMargin:"<<pTradingAccount->CurrMargin
      <<" CloseProfit:"<<pTradingAccount->CloseProfit
      <<" PositionProfit:"<<pTradingAccount->PositionProfit
      <<" Commission:"<<pTradingAccount->Commission
      <<" FrozenMargin:"<<pTradingAccount->FrozenMargin
      //<<" ¶³½áÊÖÐø·Ñ:"<<pTradingAccount->FrozenCommission 
      << endl;    
  }
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(req.InvestorID, userId);
	strcpy(req.InstrumentID, instId);	
	int ret = pUserApi->ReqQryInvestorPosition(&req, ++requestId);
	cerr << " Request | send InvestorPosition query..." << ((ret == 0) ? "success" : "fail") << endl;
}

void CtpTraderSpi::OnRspQryInvestorPosition(
    CThostFtdcInvestorPositionField *pInvestorPosition, 
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
  if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPosition ){
    cerr<<" Response| Instrument:"<<pInvestorPosition->InstrumentID
      <<" PosiDirection:"<<MapDirection(pInvestorPosition->PosiDirection-2,false)
      <<" Position:"<<pInvestorPosition->Position
      <<" Yesterday Position:"<<pInvestorPosition->YdPosition 
      <<" Today Position:"<<pInvestorPosition->TodayPosition
      <<" Position Profit:"<<pInvestorPosition->PositionProfit
      <<" UseMargin:"<<pInvestorPosition->UseMargin<<endl;
  }
  if(bIsLast) SetEvent(g_hEvent);	
}

void CtpTraderSpi::ReqOrderInsert(TThostFtdcInstrumentIDType instId,
    TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
    TThostFtdcPriceType price,   TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, appId);  //Ó¦ÓÃµ¥Ôª´úÂë	
	strcpy(req.InvestorID, userId); //Í¶×ÊÕß´úÂë	
	strcpy(req.InstrumentID, instId); //ºÏÔ¼´úÂë	
	strcpy(req.OrderRef, orderRef);  //±¨µ¥ÒýÓÃ
  int nextOrderRef = atoi(orderRef);
  sprintf(orderRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//¼Û¸ñÀàÐÍ=ÏÞ¼Û	
	req.Direction = MapDirection(dir,true);  //ÂòÂô·½Ïò	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //THOST_FTDC_OF_Open; //×éºÏ¿ªÆ½±êÖ¾:¿ª²Ö
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //×éºÏÍ¶»úÌ×±£±êÖ¾
	req.LimitPrice = price;	//¼Û¸ñ
	req.VolumeTotalOriginal = vol;	///ÊýÁ¿	
	req.TimeCondition = THOST_FTDC_TC_GFD;  //ÓÐÐ§ÆÚÀàÐÍ:µ±ÈÕÓÐÐ§
	req.VolumeCondition = THOST_FTDC_VC_AV; //³É½»Á¿ÀàÐÍ:ÈÎºÎÊýÁ¿
	req.MinVolume = 1;	//×îÐ¡³É½»Á¿:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //´¥·¢Ìõ¼þ:Á¢¼´
	
  //TThostFtdcPriceType	StopPrice;  //Ö¹Ëð¼Û
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//Ç¿Æ½Ô­Òò:·ÇÇ¿Æ½	
	req.IsAutoSuspend = 0;  //×Ô¶¯¹ÒÆð±êÖ¾:·ñ	
	req.UserForceClose = 0;   //ÓÃ»§Ç¿ÆÀ±êÖ¾:·ñ

	int ret = pUserApi->ReqOrderInsert(&req, ++requestId);
	cerr << " Request | insert order..." << ((ret == 0) ? "success" : "fail") << endl;
}

void CtpTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
          CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
  if( !IsErrorRspInfo(pRspInfo) && pInputOrder ){
    cerr<<"Response | Insert order success...Order Reference:"<<pInputOrder->OrderRef<<endl;  
  }
  if(bIsLast) SetEvent(g_hEvent);	
}

void CtpTraderSpi::ReqOrderAction(TThostFtdcSequenceNoType orderSeq)
{
  bool found=false; unsigned int i=0;
  for(i=0;i<orderList.size();i++){
    if(orderList[i]->BrokerOrderSeq == orderSeq){ found = true; break;}
  }
  if(!found){cerr<<" Request | Insert order doesn't exist."<<endl; return;} 

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);   //¾­¼Í¹«Ë¾´úÂë	
	strcpy(req.InvestorID, userId); //Í¶×ÊÕß´úÂë
	//strcpy(req.OrderRef, pOrderRef); //±¨µ¥ÒýÓÃ	
	//req.FrontID = frontId;           //Ç°ÖÃ±àºÅ	
	//req.SessionID = sessionId;       //»á»°±àºÅ
  strcpy(req.ExchangeID, orderList[i]->ExchangeID);
  strcpy(req.OrderSysID, orderList[i]->OrderSysID);
	req.ActionFlag = THOST_FTDC_AF_Delete;  //²Ù×÷±êÖ¾ 

	int ret = pUserApi->ReqOrderAction(&req, ++requestId);
	cerr << " Request | backout order..." << ((ret == 0) ? "success" : "fail") << endl;
}

void CtpTraderSpi::OnRspOrderAction(
      CThostFtdcInputOrderActionField *pInputOrderAction, 
      CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
  if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction){
    cerr<< " Response | backout order success..."
      << "Exchange ID:"<<pInputOrderAction->ExchangeID
      <<" Order System ID:"<<pInputOrderAction->OrderSysID<<endl;
  }
  if(bIsLast) SetEvent(g_hEvent);	
}

///±¨µ¥»Ø±¨
void CtpTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{	
  CThostFtdcOrderField* order = new CThostFtdcOrderField();
  memcpy(order,  pOrder, sizeof(CThostFtdcOrderField));
  bool founded=false;    unsigned int i=0;
  for(i=0; i<orderList.size(); i++){
    if(orderList[i]->BrokerOrderSeq == order->BrokerOrderSeq) {
      founded=true;    break;
    }
  }
  if(founded) orderList[i]= order;   
  else  orderList.push_back(order);
  cerr<<" Response | order submitted...ID:"<<order->BrokerOrderSeq<<endl;
  SetEvent(g_hEvent);	
}

///³É½»Í¨Öª
void CtpTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
  CThostFtdcTradeField* trade = new CThostFtdcTradeField();
  memcpy(trade,  pTrade, sizeof(CThostFtdcTradeField));
  bool founded=false;     unsigned int i=0;
  for(i=0; i<tradeList.size(); i++){
    if(tradeList[i]->TradeID == trade->TradeID) {
      founded=true;   break;
    }
  }
  if(founded) tradeList[i] = trade;   
  else  tradeList.push_back(trade);
  cerr<<" Response | order traded...TradeID:"<<trade->TradeID<<endl;
  SetEvent(g_hEvent);
}

void CtpTraderSpi::OnFrontDisconnected(int nReason)
{
	cerr<<" Response | Disconnected..." 
	  << " reason=" << nReason << endl;
}
		
void CtpTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr<<"Response | heartbeat is out of time limit..." 
	  << " TimerLapse = " << nTimeLapse << endl;
}

void CtpTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}

bool CtpTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// Èç¹ûErrorID != 0, ËµÃ÷ÊÕµ½ÁË´íÎóµÄÏìÓ¦
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
    cerr<<" Response | "<<pRspInfo->ErrorMsg<<endl;
  }
	return ret;
}

void CtpTraderSpi::PrintOrders(){
  CThostFtdcOrderField* pOrder; 
  for(unsigned int i=0; i<orderList.size(); i++){
    pOrder = orderList[i];
    cerr<<" Order | InstrumentID:"<<pOrder->InstrumentID
      <<" Direction:"<<MapDirection(pOrder->Direction,false)
      <<" CombOffsetFlag:"<<MapOffset(pOrder->CombOffsetFlag[0],false)
      <<" LimitPrice:"<<pOrder->LimitPrice
      <<" Volume:"<<pOrder->VolumeTotalOriginal
      <<" BrokerOrderSeq:"<<pOrder->BrokerOrderSeq 
      <<" Order System ID:"<<pOrder->OrderSysID
      <<" Status Msg:"<<pOrder->StatusMsg<<endl;
  }
  SetEvent(g_hEvent);
}
void CtpTraderSpi::PrintTrades(){
  CThostFtdcTradeField* pTrade;
  for(unsigned int i=0; i<tradeList.size(); i++){
    pTrade = tradeList[i];
    cerr<<" Trade | InstrumentID:"<< pTrade->InstrumentID 
      <<" Direction:"<<MapDirection(pTrade->Direction,false)
      <<" OffsetFlag:"<<MapOffset(pTrade->OffsetFlag,false) 
      <<" Price:"<<pTrade->Price
      <<" Volume:"<<pTrade->Volume
      <<" Order System ID:"<<pTrade->OrderSysID
      <<" Trade ID:"<<pTrade->TradeID<<endl;
  }
  SetEvent(g_hEvent);
}
char MapDirection(char src, bool toOrig=true){
  if(toOrig){
    if('b'==src||'B'==src){src='0';}else if('s'==src||'S'==src){src='1';}
  }else{
    if('0'==src){src='B';}else if('1'==src){src='S';}
  }
  return src;
}
char MapOffset(char src, bool toOrig=true){
  if(toOrig){
    if('o'==src||'O'==src){src='0';}
    else if('c'==src||'C'==src){src='1';}
    else if('j'==src||'J'==src){src='3';}
  }else{
    if('0'==src){src='O';}
    else if('1'==src){src='C';}
    else if('3'==src){src='J';}
  }
  return src;
}

