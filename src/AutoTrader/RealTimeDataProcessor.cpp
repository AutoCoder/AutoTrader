#include <iostream>
#include <assert.h>

#include "stdafx.h"
#include "RealTimeDataProcessor.h"
#include "TickWrapper.h"
#include "KData.h"
#include "Order.h"
#include "ThreadSafeQueue.h"
#include "Config.h"
#include "DBWrapper.h"
#include "CommonUtils.h"
#include "spdlog/spdlog.h"
#include "crossplatform.h"
#include "BaseClientSession.h"
#include "OrderTrigger.h"
#include "mdspi.h"

//#define UseKDataToInvoke 1

RealTimeDataProcessor::RealTimeDataProcessor(OrderTriggerBase* trigger, const std::string& InstrumentName, BaseClientSession* owner, CtpMdSpi* spi, bool replay)
	: m_Name(InstrumentName)
	, m_trigger(trigger)
	, m_owner(owner)
	, m_dbptr(new DBWrapper)
	, m_TickVec(spi->GetTickVec(InstrumentName))
	, m_KDataVec(spi->GetKDataVec(InstrumentName))
	, m_TickSet60(spi->GetTickVec60(InstrumentName))
{
	if (!replay)
		recoverHistoryData(600);
}
  
RealTimeDataProcessor::~RealTimeDataProcessor()
{
	StoreStrategySequenceToDB("Test_Dest");
}

void RealTimeDataProcessor::StoreStrategySequenceToDB(const std::string& mark)
{
	SYNC_LOG << "Start to store Strategy trigger point to db..." << mark;
	//store Strategy data in memory into db
	long long pre_uuid = 0;
	TickType pre_type = TickType::Commom;
	for (auto iter = m_TickVec.begin(); iter != m_TickVec.end(); iter++){
		if (iter->m_techvec != nullptr && iter->m_techvec->GetTickType() != TickType::Commom && iter->toTimeStamp() != (pre_uuid + 1) && iter->m_techvec->GetTickType() != pre_type){
			pre_uuid = iter->toTimeStamp();
			pre_type = iter->m_techvec->GetTickType();
			iter->m_techvec->serializeToDB(*(m_dbptr.get()), mark);
		}
	}
	SYNC_LOG << "Finish...";
}

//main thread
void RealTimeDataProcessor::AppendRealTimeData(TickWrapper& info){
	//(in)front-------------back(out)
	// if m_strategy == nullptr, that means RealTimeDataProcessor is in data-recording mode
	if (m_trigger){
		Order ord;
#ifdef UseKDataToInvoke
		bool triggered = m_trigger->tryInvoke(m_TickVec, m_KDataVec, m_TickSet60, info, ord);
#else
		bool triggered = m_trigger->tryInvoke(m_TickVec, info, ord);
#endif
		if (triggered){
			ord.SetTriggerTick(info.toTimeStamp());

			if (m_owner){
				m_owner->AppendOrder(ord);
			}
		}
	}
	if (m_owner)
		m_owner->SendTickToClient(info);

#ifdef SHOW_PROGRESS
	SYNC_PRINT << "> Data queue size :" << m_DataSeq.size();
#endif
}

void RealTimeDataProcessor::recoverHistoryData(int beforeSeconds)
{
	//the previous tradeDay's 1200
	const char * sqlselect = "select * from (select * from %s.%s order by id desc limit %d) as tbl order by tbl.id;";

	char sqlbuf[512];
	SPRINTF(sqlbuf, sqlselect, Config::Instance()->DBName().c_str(), m_Name.c_str(), beforeSeconds * 2); //beforeSeconds*2  ==  n(s) * (1 call back /500ms)

	std::map<int, std::vector<std::string>> map_results;
	m_dbptr->Query(sqlbuf, map_results);
	
	for (auto item : map_results){
		m_TickVec.push_back(TickWrapper::RecoverFromDB(item.second));
	}

}

bool RealTimeDataProcessor::IsTrading() const 
{ 
	return m_owner ? m_owner->IsTrading() : false;
}
