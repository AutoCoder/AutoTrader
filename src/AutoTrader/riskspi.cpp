#include "string.h"
#include "math.h"
#include "windows.h"
#include "FtdcRiskUserApi.h"
#include "riskspi.h"

#include "stdafx.h"
#include <iostream>
#include <vector>
using namespace std;

extern TShfeFtdcBrokerIDType appId;		// ¾­¼Í¹«Ë¾´úÂë
extern TShfeFtdcUserIDType userId;		// ²Ù×÷Ô±

extern int requestId;
extern HANDLE g_hEvent;

vector<CShfeFtdcSequencialPositionField*> posList;
vector<CShfeFtdcRiskSyncAccountField* > capList;
//Á¬½Ó³É¹¦
void CtpRiskSpi::OnFrontConnected()
{
	cerr<<" Á¬½Ó½»Ò×Ç°ÖÃ...³É¹¦"<<endl;
  SetEvent(g_hEvent);
}

//ÇëÇóµÇÂ¼
void CtpRiskSpi::ReqUserLogin(  TShfeFtdcBrokerIDType	vAppId,
      TShfeFtdcUserIDType	vUserId,  TShfeFtdcPasswordType	vPasswd)
{
	CShfeFtdcReqRiskUserLoginField req;
  memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, vAppId); strcpy(appId, vAppId); 
	strcpy(req.UserID,   vUserId);strcpy(userId,vUserId);	
	strcpy(req.Password, vPasswd);
	req.Version=2;  
	int ret=pUserApi->ReqRiskUserLogin(&req,++requestId);	
  cerr<<" ÇëÇó | ·¢ËÍµÇÂ¼..."<<((ret == 0) ? "³É¹¦" :"Ê§°Ü") << endl;	
}

///ÇëÇóµÇÂ¼ÏìÓ¦
void CtpRiskSpi:: OnRspRiskUserLogin(
      CShfeFtdcRspRiskUserLoginField *pRspRiskUserLogin, 
      CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{	 
  if( !IsErrorRspInfo(pRspInfo) && pRspRiskUserLogin){
    cerr<<" ÏìÓ¦ | µÇÂ¼³É¹¦...µ±Ç°½»Ò×ÈÕ:"
      <<pRspRiskUserLogin->TradingDay<<endl;
  }
  if(bIsLast) SetEvent(g_hEvent);
}

///¿Í»§¶Ë½áËã×éÊý¾ÝÍ¬²½¿ªÊ¼
//½«Í¬²½: Ê±¼ä,²úÆ·ÐÅÏ¢,ºÏÔ¼ÐÅÏ¢,Í¶×ÊÕß×é, Ëæºó×Ô¶¯·µ»Ø
void CtpRiskSpi:: OnRtnClientSGDataSyncStart(
       CShfeFtdcSettlementSessionField *pSettlementSession) {	}
///Ê±¼äÍ¬²½
void CtpRiskSpi::OnRtnTimeSync(CShfeFtdcCurrentTimeField *pCurrentTime){}
///½»Ò×ËùÐÅÏ¢
void CtpRiskSpi::OnRtnExchange(CShfeFtdcExchangeField *pExchange){ }
///²úÆ·ÐÅÏ¢
void CtpRiskSpi::OnRtnProduct(CShfeFtdcProductField *pProduct) { }
///ºÏÔ¼ÐÅÏ¢
void CtpRiskSpi::OnRtnInstrument(CShfeFtdcInstrumentField *pInstrument){}
///Í¶×ÊÕß×é
void CtpRiskSpi::OnRtnInvestorGroup(
            CShfeFtdcInvestorGroupField *pInvestorGroup){ }
//×éÖ¯¼Ü¹¹Ãû³Æ
void CtpRiskSpi::OnRtnDepartment(CShfeFtdcDepartmentField *pDepartment){}
///¿Í»§¶Ë½áËã×éÊý¾ÝÍ¬²½½áÊø
void CtpRiskSpi:: OnRtnClientSGDataSyncEnd(
         CShfeFtdcSettlementSessionField *pSettlementSession) 
{
  cerr<<" »Ø±¨ | ÖÕ¶ËÍ¬²½Êý¾Ý...Íê³É"<<endl;  
	ReqUserData();	
  SetEvent(g_hEvent);
}

//²éÑ¯»ò¶©ÔÄ×Ô¼ºËùÐèÒªµÄÊý¾Ý
void CtpRiskSpi::ReqUserData()
{
  //²éÑ¯Í¶×ÊÕßÐÅÏ¢
	CShfeFtdcInvestorIDRangeField reqField={0};
	pUserApi->ReqQryInvestorLinkMan(&reqField,1,++requestId);	

	//²éÑ¯Í¶×ÊÕß×éÖ¯¼Ü¹¹ÐÅÏ¢	
	pUserApi->ReqQryInvestorLinkMan(&reqField,1,++requestId);	

	//¶©ÔÄ±¨µ¥ÐÅÏ¢	 
  //×¢:
  //   Èç¹û×Ô¼º¶ÔÒÑÊÕµ½µÄÊý¾Ý½øÐÐÁËÎÄ¼þ±£´æ,ÔòÔÙ´ÎµÇÂ½Ê±,
  //    pNotifySequence.SequenceNo¿ÉÌî×îºóÊÕµ½µÄ
  //    CShfeFtdcSequencialOrderField::UniqSequenceNo+1;
  //   ÒÔ¼õÉÙÊý¾Ý´«ÊäÁ¿;Óà½ÔÀàËÆ
	CShfeFtdcNotifySequenceField pNotifySequence={0};			
	pNotifySequence.SequenceNo=0;							   
	pUserApi->ReqSubscribeOrder(&pNotifySequence,++requestId);
															   
	//¶©ÔÄ³É½»ÐÅÏ¢	
	pNotifySequence.SequenceNo=0;
	pUserApi->ReqSubscribeTrade(&pNotifySequence,++requestId);

	//¶©ÔÄ³Ö²ÖÐÅÏ¢	
	pNotifySequence.SequenceNo=0;
	pUserApi->ReqSubscribePosition(&pNotifySequence,++requestId);

	//¶©ÔÄÓÃ»§ÊÂ¼þ	
	pNotifySequence.SequenceNo=0;
	pUserApi->ReqSubBrokerUserEvent(&pNotifySequence,++requestId);

	//¶©ÔÄÔ¤Âñµ¥
	pNotifySequence.SequenceNo=0;
	pUserApi->ReqSubRiskParkedOrder(&pNotifySequence,++requestId);


	//¶©ÔÄ×ò×Ê½ð
	CShfeFtdcRiskNtfSequenceField pNotifySequence1={0};		
	pNotifySequence1.SequenceNo=0;
	pUserApi->ReqSubPreRiskAccount(&pNotifySequence1,++requestId);

	//¶©ÔÄ³öÈë½ð
	pNotifySequence1.SequenceNo=0;
	pUserApi->ReqSubSeqDeposit(&pNotifySequence1,++requestId);

	//¶©ÔÄ½»Ò×±àÂë 	
	pNotifySequence1.SequenceNo=0;
	pUserApi->ReqSubSeqTradingCode(&pNotifySequence1,++requestId);

	//¶©ÔÄ·çÏÕÍ¨ÖªÐÅÏ¢
	pNotifySequence1.SequenceNo=0;
	pUserApi->ReqSubSeqRiskNotifyB(&pNotifySequence1,++requestId);

	//¶©ÔÄÒµÎñÍ¨Öª
	pNotifySequence1.SequenceNo=0;
	strcpy(pNotifySequence1.DataType,"BizNotice");
	pUserApi->ReqSubSeqData(&pNotifySequence1,++requestId);


}
///²éÑ¯Í¶×ÊÕßÐÅÏ¢Ó¦´ð,¶ÔÓ¦ReqQryInvestorLinkManÇëÇó
void CtpRiskSpi::OnRspQryInvestorLinkMan(
    CShfeFtdcInvestorLinkManField *pInvestorLinkMan, 
    CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){ }

///²éÑ¯Í¶×ÊÕß×éÖ¯¼Ü¹¹Ó¦´ð,¶ÔÓ¦ReqQryInvestorLinkManÇëÇó
void CtpRiskSpi::OnRspQryInvestorDepartment(
    CShfeFtdcInvestorDepartmentFlatField *pInvestorDepartmentFlat, 
    CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){ }

///ÓÐÐòµÄ±¨µ¥»Ø±¨,¶ÔÓ¦ReqSubscribeOrder¶©ÔÄ
void CtpRiskSpi::OnRtnSequencialOrder(
    CShfeFtdcSequencialOrderField *pSequencialOrder){}

///ÓÐÐòµÄ³É½»»Ø±¨,¶ÔÓ¦ReqSubscribeTrade¶©ÔÄ
void CtpRiskSpi::OnRtnSequencialTrade(
    CShfeFtdcSequencialTradeField *pSequencialTrade){}

///ÓÐÐòµÄ³Ö²Ö»Ø±¨,¶ÔÓ¦ReqSubscribePosition¶©ÔÄ
void CtpRiskSpi::OnRtnSequencialPosition(
    CShfeFtdcSequencialPositionField *pSequencialPosition)
{
  CShfeFtdcSequencialPositionField* pos= new CShfeFtdcSequencialPositionField();
  memcpy(pos,pSequencialPosition,sizeof(CShfeFtdcSequencialPositionField));
  bool founded=false;    unsigned int i=0;
  for(i=0; i<posList.size(); i++){
    if(strcmp(posList[i]->InstrumentID, pos->InstrumentID)==0
      && strcmp(posList[i]->InvestorID, pos->InvestorID)==0
      && posList[i]->PosiDirection == pos->PosiDirection
      && posList[i]->HedgeFlag == pos->HedgeFlag) {
      founded=true;    break;
    }
  }
  if(founded) posList[i]= pos;   
  else  posList.push_back(pos);
  //cerr<<" »Ø±¨ | ³Ö²Ö»Ø±¨½ÓÊÕ...ÐòºÅ:"<<pos->UniqSequenceNo<<endl;
  //SetEvent(g_hEvent);	
}

///ÓÐÐòµÄ¾­¼Í¹«Ë¾ÓÃ»§ÊÂ¼þ»Ø±¨,¶ÔÓ¦ReqSubBrokerUserEvent¶©ÔÄ
void CtpRiskSpi::OnRtnSequencialBrokerUserEvent(
    CShfeFtdcSequencialBrokerUserEventField *pSequencialBrokerUserEvent){}

///ÓÐÐòµÄ×òÈÕ×Ê½ð»Ø±¨£¬¶ÔÓ¦¶©ÔÄÇëÇóReqSubPreRiskAccount
void CtpRiskSpi::OnRtnSeqPreRiskAccount(
    CShfeFtdcSeqPreRiskAccountField *pSeqPreRiskAccount){}

///Í¶×ÊÕßµ±ÈÕ×Ê½ð»Ø±¨,²»ÐèÒª¶©ÔÄ¾Í×Ô¶¯·µ»Ø
void CtpRiskSpi::OnRtnRiskSyncAccount(
    CShfeFtdcRiskSyncAccountField *pRiskSyncAccount){
  CShfeFtdcRiskSyncAccountField* cap= new CShfeFtdcRiskSyncAccountField();
  memcpy(cap, pRiskSyncAccount,sizeof(CShfeFtdcRiskSyncAccountField));
  bool founded=false;    unsigned int i=0;
  for(i=0; i<capList.size(); i++){
    if(capList[i]->InvestorID == cap->InvestorID) {
      founded=true;    break;
    }
  }
  if(founded) capList[i]= cap;   
  else  capList.push_back(cap);
  //cerr<<" »Ø±¨ | ×Ê½ð»Ø±¨½ÓÊÕ...Í¶×ÊÕß:"<<cap->InvestorID<<endl;
  //SetEvent(g_hEvent);
}

///ÓÐÐòµÄ³öÈë½ð»Ø±¨£¬¶ÔÓ¦¶©ÔÄÇëÇóReqSubSeqDeposit
void CtpRiskSpi::OnRtnSeqDeposit(CShfeFtdcSeqDepositField *pSeqDeposit){}

///ÓÐÐòµÄ½»Ò×±àÂë»Ø±¨,¶ÔÓ¦¶©ÔÄÇëÇóReqSubSeqTradingCode
void CtpRiskSpi::OnRtnSeqTradingCode(
    CShfeFtdcSeqTradingCodeField *pSeqTradingCode){}

///ÓÐÐòµÄ·çÏÕÍ¨Öª»Ø±¨,¶ÔÓ¦¶©ÔÄÇëÇóReqSubSeqRiskNotifyB
void CtpRiskSpi::OnRtnSeqRiskNotifyB(
   CShfeFtdcSeqRiskNotifyBField *pSeqRiskNotifyB){ }

///ÓÐÐòµÄÒµÎñÍ¨Öª»Ø±¨,¶ÔÓ¦¶©ÔÄÇëÇóReqSubSeqData
void CtpRiskSpi::OnRtnSeqBizNotice(CShfeFtdcSeqBizNoticeField *pSeqBizNotice){}

///·ç¿ØÉî¶ÈÐÐÇéÍ¨Öª,¶ÔÓ¦¶©ÔÄÇëÇóReqSubRiskMarketData
void CtpRiskSpi::OnRtnRiskDepthMarketData(
  CShfeFtdcDepthMarketDataField *pDepthMarketData){}

///²éÑ¯±¨µ¥ÅÅÃûÓ¦´ð,¶ÔÓ¦ReqQryOrderStat²éÑ¯
void CtpRiskSpi::OnRspQryOrderStat(CShfeFtdcOrderStatField *pOrderStat,
  CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){}

///²éÑ¯³É½»ÅÅÃûÓ¦´ð,¶ÔÓ¦ReqQryTradeStat²éÑ¯
void CtpRiskSpi::OnRspQryTradeStat(CShfeFtdcTradeStatField *pTradeStat, 
  CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){}

///²éÑ¯³Ö²ÖÅÅÃûÓ¦´ð,¶ÔÓ¦ReqQryPositionStat²éÑ¯
void CtpRiskSpi::OnRspQryPositionStat(CShfeFtdcPositionStatField *pPositionStat, 
           CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){}

///ºÏÔ¼³Ö²Ö±ÈÀýÓ¦´ð,¶ÔÓ¦ReqQryInstPositionRate²éÑ¯
void CtpRiskSpi::OnRspInstPositionRate(
  CShfeFtdcRspInstPositionRateField *pRspInstPositionRate, 
  CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){}

///²úÆ·³Ö²Ö±ÈÀýÓ¦´ð,¶ÔÓ¦ReqQryProductPositionRate²éÑ¯
void CtpRiskSpi::OnRspProductPositionRate(
  CShfeFtdcRspProductPositionRateField *pRspProductPositionRate,
  CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){}

///ÒÔÏÂ°Ë¸ö¹ØÓÚ±£Ö¤½ðÂÊµÄº¯Êý»á×Ô¶¯»Øµ÷,²»ÐèÒªÇëÇó
///¾­¼Í¹«Ë¾ºÏÔ¼±£Ö¤½ðÂÊ
void CtpRiskSpi::OnRtnInstrumentMarginRate(
  CShfeFtdcInstrumentMarginRateField *pInstrumentMarginRate){}

///¾­¼Í¹«Ë¾ºÏÔ¼±£Ö¤½ðÂÊµ÷Õû
void CtpRiskSpi::OnRtnInstrumentMarginRateAdjust(
  CShfeFtdcInstrumentMarginRateAdjustField *pInstrumentMarginRateAdjust){}

///½»Ò×Ëù±£Ö¤½ðÂÊ
void CtpRiskSpi::OnRtnExchangeMarginRate(
  CShfeFtdcExchangeMarginRateField *pExchangeMarginRate){}

///½»Ò×Ëù±£Ö¤½ðÂÊµ÷Õû
void CtpRiskSpi::OnRtnExchangeMarginRateAdjust(
  CShfeFtdcExchangeMarginRateAdjustField *pExchangeMarginRateAdjust){}

///É¾³ý¾­¼Í¹«Ë¾ºÏÔ¼±£Ö¤½ðÂÊ
void CtpRiskSpi::OnRtnDelInstrumentMarginRate(
  CShfeFtdcInstrumentMarginRateField *pInstrumentMarginRate){}

///É¾³ý¾­¼Í¹«Ë¾ºÏÔ¼±£Ö¤½ðÂÊµ÷Õû
void CtpRiskSpi::OnRtnDelInstrumentMarginRateAdjust(
  CShfeFtdcInstrumentMarginRateAdjustField *pInstrumentMarginRateAdjust){}

///É¾³ý½»Ò×Ëù±£Ö¤½ðÂÊ
void CtpRiskSpi::OnRtnDelExchangeMarginRate(
    CShfeFtdcExchangeMarginRateField *pExchangeMarginRate){}

///É¾³ý½»Ò×Ëù±£Ö¤½ðÂÊµ÷Õû
void CtpRiskSpi::OnRtnDelExchangeMarginRateAdjust(
    CShfeFtdcExchangeMarginRateAdjustField *pExchangeMarginRateAdjust){}

///ÏÂÇ¿Æ½µ¥
void CtpRiskSpi::ReqRiskOrderInsert(TShfeFtdcSequenceNoType seqNo,
    TShfeFtdcPriceType price)
{
  bool found=false; unsigned int i=0;
  for(i=0;i<posList.size();i++){
    if(posList[i]->UniqSequenceNo == seqNo){ found = true; break;}
  }
  if(!found){cerr<<" ÇëÇó | ³Ö²Ö²»´æÔÚ."<<endl; return;} 

	CShfeFtdcRiskForceCloseOrderField field;
  memset(&field,0,sizeof(field));
	strcpy(field.BrokerID,  appId);
	strcpy(field.UserID,  userId);
	strcpy(field.InvestorID,posList[i]->InvestorID);
	strcpy(field.InstrumentID,posList[i]->InstrumentID);	
	
	field.FCType = SHFE_FTDC_FCT_Manual;///·ç¿ØÇ¿Æ½ÀàÐÍ
	field.CombHedgeFlag[0]=posList[i]->HedgeFlag;	
	field.Direction= (posList[i]->PosiDirection==SHFE_FTDC_PD_Long) ? 
                        SHFE_FTDC_D_Sell : SHFE_FTDC_D_Buy;
	field.CombOffsetFlag[0]=SHFE_FTDC_OF_Close;//Æ½²Ö; ¶ÔÉÏÆÚËùºÏÔ¼Ò²¿ÉÆ½½ñ
	field.UserForceClose=1;//±ØÐëÎª1
	field.LimitPrice= price;
	field.VolumeTotalOriginal=1;	
	field.ContingentCondition=SHFE_FTDC_CC_Immediately;	
	field.OrderPriceType=SHFE_FTDC_OPT_LimitPrice;//ÏÞ¼Ûµ¥£¬ÉÏÆÚËùÒ²Ö§³ÖÊÐ¼Ûµ¥
	field.TimeCondition=SHFE_FTDC_TC_GFD;
	field.VolumeCondition=SHFE_FTDC_VC_AV;
	field.ForceCloseReason=SHFE_FTDC_FCC_NotForceClose;
	
	int ret=pUserApi->ReqRiskOrderInsert(&field,++requestId);	
  cerr<< " ÇëÇó | ·¢ËÍÇ¿Æ½µ¥..." <<((ret == 0)?"³É¹¦":"Ê§°Ü") << endl;
}

///ÏÂÇ¿Æ½µ¥ÇëÇóÏìÓ¦
void CtpRiskSpi::OnRspRiskOrderInsert(CShfeFtdcInputOrderField *pInputOrder, 
   CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if ( !IsErrorRspInfo(pRspInfo) && pInputOrder){
		cerr <<" ÏìÓ¦ | ÏÂÇ¿Æ½µ¥³É¹¦..."<<endl;
	}
  if(bIsLast) SetEvent(g_hEvent);
}
//ÏÂÇ¿Æ½Ô¤Âñµ¥
void CtpRiskSpi::ReqRiskParkedOrderInsert(
  CShfeFtdcSequencialPositionField *pData)
{
  CShfeFtdcRiskParkedOrderField field={0};	
	field.UserType=SHFE_FTDC_RUT_SuperUser;
	field.TriggerType=SHFE_FTDC_OTT_Customize;//Ö¸¶¨°´Ê±¼ä´¥·¢;Ò²¿É°´½»Ò×½×¶Î´¥·¢
	field.TradeSegment=0;
	strcpy(field.Time2,"14:30:00");
	strcpy(field.ExchangeID,"SHFE");//ÕæÊµ»·¾³ÖÐÓ¦¸Ã´ÓºÏÔ¼ÐÅÏ¢ÖÐÈ¡µÃ¸Ã×Ö¶Î
	
	strcpy(field.BrokerID,appId);
	strcpy(field.UserID,userId);
	strcpy(field.InvestorID,  pData->InvestorID);
	strcpy(field.InstrumentID,  pData->InstrumentID);
	
	field.FCType = SHFE_FTDC_FCT_Manual;///·ç¿ØÇ¿Æ½ÀàÐÍ
	field.CombHedgeFlag[0] = pData->HedgeFlag;	
	field.Direction= (pData->PosiDirection==SHFE_FTDC_PD_Long ) ?
                      SHFE_FTDC_D_Sell : SHFE_FTDC_D_Buy;
	field.CombOffsetFlag[0]=SHFE_FTDC_OF_Close;//Æ½²Ö; ¶ÔÉÏÆÚËùºÏÔ¼Ò²¿ÉÆ½½ñ
	field.UserForceClose=1; //±ØÐëÎª1
	field.LimitPrice=  1000;
	field.VolumeTotalOriginal=1;	
	field.ContingentCondition=SHFE_FTDC_CC_Immediately;	
	field.OrderPriceType=SHFE_FTDC_OPT_LimitPrice;//ÏÞ¼Ûµ¥£¬ÉÏÆÚËùÒ²Ö§³ÖÊÐ¼Ûµ¥
	field.TimeCondition=SHFE_FTDC_TC_GFD;
	field.VolumeCondition=SHFE_FTDC_VC_AV;
	field.ForceCloseReason=SHFE_FTDC_FCC_NotForceClose;
	field.ForceCloseRelease=SHFE_FTDC_FCR_RealTime;
	int ret=pUserApi->ReqRiskParkedOrderInsert(&field,++requestId);
  cerr<< " ÇëÇó | ·¢ËÍÔ¤ÂñÇ¿Æ½µ¥..." <<((ret == 0)?"³É¹¦":"Ê§°Ü") << endl;
}

//ÏÂÔ¤Âñµ¥ÇëÇóÏìÓ¦
void CtpRiskSpi::OnRspRiskParkedOrderInsert(
  CShfeFtdcRiskParkedOrderField *pRiskParkedOrder, 
  CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if ( !IsErrorRspInfo(pRspInfo) && pRiskParkedOrder)	{
		cerr <<" ÏìÓ¦ | Ô¤ÂñÇ¿Æ½µ¥...³É¹¦"<<endl;
	}
  if (bIsLast) SetEvent(g_hEvent);
}
//³·µ¥
void CtpRiskSpi::ReqRiskOrderAction(CShfeFtdcSequencialOrderField *pData)
{
	CShfeFtdcInputOrderActionField req={0};
	strcpy(req.BrokerID,pData->BrokerID);
	strcpy(req.InvestorID,pData->InvestorID);
	req.OrderActionRef=0;
	strcpy(req.OrderRef,pData->OrderRef);
	req.RequestID=pData->RequestID;
	req.FrontID=pData->FrontID;
	req.SessionID=pData->SessionID;
	strcpy(req.ExchangeID,pData->ExchangeID);
	strcpy(req.OrderSysID,pData->OrderSysID);
	req.ActionFlag=SHFE_FTDC_AF_Delete;//ÔÝÊ±²»Ö§³Ö¸Äµ¥
	req.LimitPrice=pData->LimitPrice;
	req.VolumeChange=0;
	strcpy(req.UserID,userId);
	strcpy(req.InstrumentID,pData->InstrumentID);
  
  int ret=	pUserApi->ReqRiskOrderAction(&req,++requestId);
  cerr<< " ÇëÇó | ·¢ËÍ³·µ¥..." <<((ret == 0)?"³É¹¦":"Ê§°Ü") << endl;
}

///³·µ¥ÏìÓ¦
void CtpRiskSpi::OnRspRiskOrderAction(
    CShfeFtdcInputOrderActionField *pInputOrderAction, 
    CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if ( !IsErrorRspInfo(pRspInfo) && pInputOrderAction){
    cerr <<" ÏìÓ¦ | ³·µ¥³É¹¦...½»Ò×Ëù:"
      <<pInputOrderAction->ExchangeID
      <<" ±¨µ¥±àºÅ:"<<pInputOrderAction->OrderSysID<<endl;
	}
  if (bIsLast) SetEvent(g_hEvent);
}
//³·Ô¤Âñµ¥
void CtpRiskSpi::ReqRemoveRiskParkedOrder(
    CShfeFtdcRemoveRiskParkedOrderField* pData){ }

///³·Ô¤Âñµ¥ÏìÓ¦
void CtpRiskSpi::OnRspRemoveRiskParkedOrder(
    CShfeFtdcRemoveRiskParkedOrderField *pRemoveRiskParkedOrder, 
    CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){}

//Ï¯Î»×Ê½ð²éÑ¯
void CtpRiskSpi::ReqRiskQryBrokerDeposit(TShfeFtdcExchangeIDType	exchangeId)
{
	CShfeFtdcQueryBrokerDepositField field={0};
	strcpy(field.BrokerID,appId);
	strcpy(field.ExchangeID,exchangeId);
	int ret = pUserApi->ReqRiskQryBrokerDeposit(&field,++requestId);
  cerr<<" ÇëÇó | ·¢ËÍ»áÔ±×Ê½ð²éÑ¯..."<<((ret == 0) ? "³É¹¦" :"Ê§°Ü")<<endl;	
}

///²éÑ¯Ï¯Î»×Ê½ðÓ¦´ð
void CtpRiskSpi::OnRspRiskQryBrokerDeposit(
    CShfeFtdcQueryBrokerDepositField *pQueryBrokerDeposit, 
    CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	if (!IsErrorRspInfo(pRspInfo) && pQueryBrokerDeposit)	{
		cerr<<" ÏìÓ¦ | ²éÑ¯Ï¯Î»×Ê½ð...³É¹¦"<<endl;
	}
  if (bIsLast) SetEvent(g_hEvent);
}

///Ï¯Î»×Ê½ð»Ø±¨,ÓÉReqRiskQryBrokerDeposit´¥·¢
void CtpRiskSpi::OnRtnBrokerDeposit(CShfeFtdcBrokerDepositField *pBrokerDeposit)
{
  cerr<<" »Ø±¨ | »áÔ±:"<<pBrokerDeposit->ParticipantID
    <<" ½áËã×¼±¸½ð:"<<pBrokerDeposit->Balance
    <<" µ±Ç°±£Ö¤½ð:"<<pBrokerDeposit->CurrMargin
    <<" ¿ÉÓÃ×Ê½ð:"<<pBrokerDeposit->Available
    <<" Æ½²ÖÓ¯¿÷:"<<pBrokerDeposit->CloseProfit<<endl;
  SetEvent(g_hEvent);	
}

bool CtpRiskSpi::IsErrorRspInfo(CShfeFtdcRspInfoField *pRspInfo)
{
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
    cerr<<" ÏìÓ¦ | "<<pRspInfo->ErrorMsg<<endl;
  }
  return ret;
}

///Á¬½Ó¶Ï¿ª
void CtpRiskSpi::OnFrontDisconnected(int nReason)
{
  cerr<<" ÏìÓ¦ | Á¬½ÓÖÐ¶Ï..." 
    << " reason=" << nReason << endl;
}

///ÐÄÌø
void CtpRiskSpi::OnHeartBeatWarning(int nTimeLapse)
{
  cerr<<" ÏìÓ¦ | ÐÄÌø³¬Ê±¾¯¸æ..." 
    << " TimerLapse = " << nTimeLapse << endl;
}

///´íÎóÓ¦´ð	(response to wrong/invalid quest)
void CtpRiskSpi:: OnRspError(CShfeFtdcRspInfoField *pRspInfo, 
    int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}
void CtpRiskSpi::PrintAcounts(){
  CShfeFtdcRiskSyncAccountField* cap; 
  for(unsigned int i=0; i<capList.size(); i++){
    cap = capList[i];
    cerr<<" ×Ê½ð | Í¶×ÊÕß:"<<cap->InvestorID
      <<" È¨Òæ:"<< cap->Balance
      <<" ¿ÉÓÃ:"<<cap->Available
      <<" µ±Ç°±£Ö¤½ð:"<< cap->CurrMargin 
      <<" ¶³½á±£Ö¤½ð:"<< cap->FrozenMargin      
      <<" ¶³½áÊÖÐø·Ñ:"<< cap->FrozenCommission
      <<" Æ½²ÖÓ¯¿÷:"<<cap->CloseProfit 
      <<" ³Ö²ÖÓ¯¿÷:"<<cap->PositionProfit<<endl;
  }
  SetEvent(g_hEvent);
}
void CtpRiskSpi::PrintPositions(){
  CShfeFtdcSequencialPositionField* pos; 
  for(unsigned int i=0; i<posList.size(); i++){
    pos = posList[i];
    cerr<<" ³Ö²Ö | ÐòºÅ:"<<pos->UniqSequenceNo
      <<" Í¶×ÊÕß:"<<pos->InvestorID 
      <<" ºÏÔ¼:"<< pos->InstrumentID
      <<" ·½Ïò:"<< pos->PosiDirection 
      <<" ×Ü³Ö²Ö:"<<pos->Position
      <<" ½ñ²Ö:"<< pos->TodayPosition
      <<" ³Ö²ÖÓ¯¿÷:"<<pos->PositionProfit
      <<" Õ¼ÓÃ±£Ö¤½ð:"<<pos->UseMargin<<endl;
  }
  SetEvent(g_hEvent);
}
