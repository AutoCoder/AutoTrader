#include <iostream>
#include <assert.h>

#include "stdafx.h"
#include "MdProcessor.h"
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

MdProcessor::MdProcessor(OrderTriggerBase* trigger, const std::string& InstrumentName, BaseClientSession* owner, CtpMdSpi* spi)
	: m_Name(InstrumentName)
	, m_trigger(trigger)
	, m_owner(owner)
	, m_dbptr(new DBWrapper)
	, m_replay(false)
	, m_TickVec(spi->GetTickVec(InstrumentName))
	, m_KDataVec(spi->GetKDataVec(InstrumentName))
	, m_TickSet60(spi->GetTickVec60(InstrumentName))
{
	assert(spi);
//	if (!replay)
//		recoverHistoryData(600);
}

MdProcessor::MdProcessor(OrderTriggerBase* trigger, const std::string& InstrumentName, BaseClientSession* owner, \
	std::vector<TickWrapper>& tickVec, std::vector<KData>& kdataVec, std::vector<TickWrapper>& tick60)
	: m_Name(InstrumentName)
	, m_trigger(trigger)
	, m_owner(owner)
	, m_dbptr(new DBWrapper)
	, m_replay(true)
	, m_TickVec(tickVec)
	, m_KDataVec(kdataVec)
	, m_TickSet60(tick60)
{
}
  
MdProcessor::~MdProcessor()
{
	if (!m_replay)
		StoreStrategySequenceToDB("Test_Dest");
}

void MdProcessor::StoreStrategySequenceToDB(const std::string& mark)
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
void MdProcessor::AppendTick(TickWrapper& info){
	// if m_trigger == nullptr, that means MdProcessor is in data-recording mode
	if (m_trigger){
		//Order ord;
		OrderVec orders;
#ifdef UseKDataToInvoke
		bool triggered = m_trigger->tryInvoke(m_TickVec, m_KDataVec, m_TickSet60, info, orders);
#else
		bool triggered = m_trigger->tryInvoke(m_TickVec, info, orders);
#endif
		if (triggered){
			for (auto& ord : orders){
				ord.SetTriggerTick(info.toTimeStamp());
			}

			if (m_owner){
				for (auto ord : orders){
					m_owner->AppendOrder(ord);
				}
			}
		}
	}

	if (m_owner){
		m_owner->SendTickToClient(info);
	}

#ifdef SHOW_PROGRESS
	SYNC_PRINT << "> Data queue size :" << m_DataSeq.size();
#endif
}

void MdProcessor::recoverHistoryData(int beforeSeconds)
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

bool MdProcessor::IsTrading() const 
{ 
	return m_owner ? m_owner->IsTrading() : false;
}
