// RCUserSpiImpl.h : interface of the CRiskSpi class
//

#ifndef RISK_RISKSPI_H_
#define RISK_RISKSPI_H_
#pragma once
#include "FtdcRiskUserApi.h"

class CtpRiskSpi : public CShfeFtdcRiskUserSpi
{
public:

  CtpRiskSpi(CShfeFtdcRiskUserApi *p):pUserApi(p){};
	///µ±¿Í»§¶ËÓë½»Ò×ºóÌ¨½¨Á¢ÆðÍ¨ÐÅÁ¬½ÓÊ±£¨»¹Î´µÇÂ¼Ç°£©£¬¸Ã·½·¨±»µ÷ÓÃ¡£
	void OnFrontConnected();
	
	///µ±¿Í»§¶ËÓë½»Ò×ºóÌ¨Í¨ÐÅÁ¬½Ó¶Ï¿ªÊ±£¬¸Ã·½·¨±»µ÷ÓÃ¡£µ±·¢ÉúÕâ¸öÇé¿öºó£¬API»á×Ô¶¯ÖØÐÂÁ¬½Ó£¬¿Í»§¶Ë¿É²»×ö´¦Àí¡£
	///@param nReason ´íÎóÔ­Òò
	///        0x1001 ÍøÂç¶ÁÊ§°Ü
	///        0x1002 ÍøÂçÐ´Ê§°Ü
	///        0x2001 ½ÓÊÕÐÄÌø³¬Ê±
	///        0x2002 ·¢ËÍÐÄÌøÊ§°Ü
	///        0x2003 ÊÕµ½´íÎó±¨ÎÄ
	void OnFrontDisconnected(int nReason);
		
	///ÐÄÌø³¬Ê±¾¯¸æ¡£µ±³¤Ê±¼äÎ´ÊÕµ½±¨ÎÄÊ±£¬¸Ã·½·¨±»µ÷ÓÃ¡£
	///@param nTimeLapse ¾àÀëÉÏ´Î½ÓÊÕ±¨ÎÄµÄÊ±¼ä
	void OnHeartBeatWarning(int nTimeLapse);	

	///´íÎóÓ¦´ð
	virtual void OnRspError(CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	
	///¿Í»§¶Ë½áËã×éÊý¾ÝÍ¬²½¿ªÊ¼
	virtual void OnRtnClientSGDataSyncStart(CShfeFtdcSettlementSessionField *pSettlementSession) ;	

	///µÇÂ¼ÇëÇóÏìÓ¦
	virtual void OnRspRiskUserLogin(CShfeFtdcRspRiskUserLoginField *pRspRiskUserLogin, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	
	///Ê±¼äÍ¬²½
	virtual void OnRtnTimeSync(CShfeFtdcCurrentTimeField *pCurrentTime);

	///½»Ò×ËùÐÅÏ¢
	virtual void OnRtnExchange(CShfeFtdcExchangeField *pExchange);
	
	///²úÆ·ÐÅÏ¢
	virtual void OnRtnProduct(CShfeFtdcProductField *pProduct);

	///ºÏÔ¼ÐÅÏ¢
	virtual void OnRtnInstrument(CShfeFtdcInstrumentField *pInstrument);

	///Í¶×ÊÕß×é
	virtual void OnRtnInvestorGroup(CShfeFtdcInvestorGroupField *pInvestorGroup);

	///×éÖ¯¼Ü¹¹Ãû³Æ
	virtual void OnRtnDepartment(CShfeFtdcDepartmentField *pDepartment);

	///¿Í»§¶Ë½áËã×éÊý¾ÝÍ¬²½½áÊø
	virtual void OnRtnClientSGDataSyncEnd(CShfeFtdcSettlementSessionField *pSettlementSession) ;

	///²éÑ¯Í¶×ÊÕßÐÅÏ¢Ó¦´ð
	virtual void OnRspQryInvestorLinkMan(CShfeFtdcInvestorLinkManField *pInvestorLinkMan, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///²éÑ¯Í¶×ÊÕß×éÖ¯¼Ü¹¹Ó¦´ð
	virtual void OnRspQryInvestorDepartment(CShfeFtdcInvestorDepartmentFlatField *pInvestorDepartmentFlat, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///ÓÐÐòµÄ±¨µ¥»Ø±¨
	virtual void OnRtnSequencialOrder(CShfeFtdcSequencialOrderField *pSequencialOrder);

	///ÓÐÐòµÄ³É½»»Ø±¨
	virtual void OnRtnSequencialTrade(CShfeFtdcSequencialTradeField *pSequencialTrade);

	///ÓÐÐòµÄ³Ö²Ö»Ø±¨
	virtual void OnRtnSequencialPosition(CShfeFtdcSequencialPositionField *pSequencialPosition);

	///ÓÐÐòµÄ¾­¼Í¹«Ë¾ÓÃ»§ÊÂ¼þ»Ø±¨
	virtual void OnRtnSequencialBrokerUserEvent(CShfeFtdcSequencialBrokerUserEventField *pSequencialBrokerUserEvent);

	///ÓÐÐòµÄ×òÈÕ×Ê½ð»Ø±¨
	virtual void OnRtnSeqPreRiskAccount(CShfeFtdcSeqPreRiskAccountField *pSeqPreRiskAccount);
	
	///Í¶×ÊÕßµ±ÈÕ×Ê½ð»Ø±¨
	virtual void OnRtnRiskSyncAccount(CShfeFtdcRiskSyncAccountField *pRiskSyncAccount);

	///ÓÐÐòµÄ³öÈë½ð»Ø±¨
	virtual void OnRtnSeqDeposit(CShfeFtdcSeqDepositField *pSeqDeposit);

	///ÓÐÐòµÄ½»Ò×±àÂë»Ø±¨
	virtual void OnRtnSeqTradingCode(CShfeFtdcSeqTradingCodeField *pSeqTradingCode);

	///ÓÐÐòµÄ·çÏÕÍ¨Öª»Ø±¨
	virtual void OnRtnSeqRiskNotifyB(CShfeFtdcSeqRiskNotifyBField *pSeqRiskNotifyB);

	///ÓÐÐòµÄÒµÎñÍ¨Öª»Ø±¨
	virtual void OnRtnSeqBizNotice(CShfeFtdcSeqBizNoticeField *pSeqBizNotice);

	///·ç¿ØÉî¶ÈÐÐÇéÍ¨Öª
	virtual void OnRtnRiskDepthMarketData(CShfeFtdcDepthMarketDataField *pDepthMarketData);

	///²éÑ¯±¨µ¥ÅÅÃûÓ¦´ð
	virtual void OnRspQryOrderStat(CShfeFtdcOrderStatField *pOrderStat, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///²éÑ¯³É½»ÅÅÃûÓ¦´ð
	virtual void OnRspQryTradeStat(CShfeFtdcTradeStatField *pTradeStat, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///²éÑ¯³Ö²ÖÅÅÃûÓ¦´ð
	virtual void OnRspQryPositionStat(CShfeFtdcPositionStatField *pPositionStat, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///ºÏÔ¼³Ö²Ö±ÈÀýÓ¦´ð
	virtual void OnRspInstPositionRate(CShfeFtdcRspInstPositionRateField *pRspInstPositionRate, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///²úÆ·³Ö²Ö±ÈÀýÓ¦´ð
	virtual void OnRspProductPositionRate(CShfeFtdcRspProductPositionRateField *pRspProductPositionRate, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	
	///¾­¼Í¹«Ë¾ºÏÔ¼±£Ö¤½ðÂÊ
	virtual void OnRtnInstrumentMarginRate(CShfeFtdcInstrumentMarginRateField *pInstrumentMarginRate);

	///¾­¼Í¹«Ë¾ºÏÔ¼±£Ö¤½ðÂÊµ÷Õû
	virtual void OnRtnInstrumentMarginRateAdjust(CShfeFtdcInstrumentMarginRateAdjustField *pInstrumentMarginRateAdjust);

	///½»Ò×Ëù±£Ö¤½ðÂÊ
	virtual void OnRtnExchangeMarginRate(CShfeFtdcExchangeMarginRateField *pExchangeMarginRate);

	///½»Ò×Ëù±£Ö¤½ðÂÊµ÷Õû
	virtual void OnRtnExchangeMarginRateAdjust(CShfeFtdcExchangeMarginRateAdjustField *pExchangeMarginRateAdjust);

	///É¾³ý¾­¼Í¹«Ë¾ºÏÔ¼±£Ö¤½ðÂÊ
	virtual void OnRtnDelInstrumentMarginRate(CShfeFtdcInstrumentMarginRateField *pInstrumentMarginRate);	

	///É¾³ý¾­¼Í¹«Ë¾ºÏÔ¼±£Ö¤½ðÂÊµ÷Õû
	virtual void OnRtnDelInstrumentMarginRateAdjust(CShfeFtdcInstrumentMarginRateAdjustField *pInstrumentMarginRateAdjust);
	
	///É¾³ý½»Ò×Ëù±£Ö¤½ðÂÊ
	virtual void OnRtnDelExchangeMarginRate(CShfeFtdcExchangeMarginRateField *pExchangeMarginRate);

	///É¾³ý½»Ò×Ëù±£Ö¤½ðÂÊµ÷Õû
	virtual void OnRtnDelExchangeMarginRateAdjust(CShfeFtdcExchangeMarginRateAdjustField *pExchangeMarginRateAdjust);

	///ÏÂÇ¿Æ½µ¥ÇëÇóÏìÓ¦
	virtual void OnRspRiskOrderInsert(CShfeFtdcInputOrderField *pInputOrder, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	//ÏÂÔ¤Âñµ¥ÇëÇóÏìÓ¦
	virtual void OnRspRiskParkedOrderInsert(CShfeFtdcRiskParkedOrderField *pRiskParkedOrder, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///³·µ¥ÏìÓ¦
	virtual void OnRspRiskOrderAction(CShfeFtdcInputOrderActionField *pInputOrderAction, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		
	///³·Ô¤Âñµ¥ÏìÓ¦
	virtual void OnRspRemoveRiskParkedOrder(CShfeFtdcRemoveRiskParkedOrderField *pRemoveRiskParkedOrder, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///²éÑ¯Ï¯Î»×Ê½ðÓ¦´ð
	virtual void OnRspRiskQryBrokerDeposit(CShfeFtdcQueryBrokerDepositField *pQueryBrokerDeposit, CShfeFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///Ï¯Î»×Ê½ð»Ø±¨
	virtual void OnRtnBrokerDeposit(CShfeFtdcBrokerDepositField *pBrokerDeposit);

public:	
	//ÇëÇóµÇÂ¼
	void ReqUserLogin( TShfeFtdcBrokerIDType	appId,
      TShfeFtdcUserIDType	userId,  TShfeFtdcPasswordType	passwd);
	//ÇëÇóÊý¾Ý
	void ReqUserData();
	//ÏÂÇ¿Æ½µ¥
	void ReqRiskOrderInsert(TShfeFtdcSequenceNoType seqNo,
              TShfeFtdcPriceType price);
	//ÏÂÇ¿Æ½Ô¤Âñµ¥
	void ReqRiskParkedOrderInsert(CShfeFtdcSequencialPositionField *pData);
	//³·µ¥
	void ReqRiskOrderAction(CShfeFtdcSequencialOrderField *pData);
	//³·Ô¤Âñµ¥
	void ReqRemoveRiskParkedOrder(CShfeFtdcRemoveRiskParkedOrderField* pData);
	//Ï¯Î»×Ê½ð²éÑ¯
	void ReqRiskQryBrokerDeposit(TShfeFtdcExchangeIDType	ExchangeID);
	//ÊÇ·ñÊÕµ½³É¹¦»ØÓ¦
	bool IsErrorRspInfo(CShfeFtdcRspInfoField *pRspInfo);
    
  void PrintAcounts();
  void PrintPositions();

private:
  CShfeFtdcRiskUserApi *pUserApi;
};



#endif
