#include "stdafx.h"
#include "config.h"
#include "DBWrapper.h"
#include "Order.h"
#include "MACrossStratgy.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include <sstream>
#include <assert.h>

MACrossStratgy::MACrossStratgy()
:m_curOrder(new Order())//m_order is a pointer so that it will only update so, create it at constructor.
{
}


MACrossStratgy::~MACrossStratgy()
{
	delete m_curOrder;
	m_curOrder = nullptr;
}

// common MA 
double MACrossStratgy::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const
{
	//datetime to timestamp
	double totalExchangeLastPrice = current.LastPrice();
	long long count = 1;

	long long leftedge = current.toTimeStamp() - seconds * 2;
	for (auto it = data.begin(); it != data.end(); it++)
	{
		if (it->toTimeStamp() > leftedge){
			totalExchangeLastPrice += it->LastPrice();
			++count;
		}
		else{
			break;
		}
	}

	//assert(totalVolume != 0);
	//assert(totalExchangePrice >= 0.0);

	return totalExchangeLastPrice / count;
}

MACrossStratgyTechVec* MACrossStratgy::generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const{
	return (new MACrossStratgyTechVec(CrossStratgyType::MA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}

bool MACrossStratgy::tryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info)
{
	TickType direction = TickType::Commom;
	const size_t breakthrough_confirm_duration = 100; //50ms
	MACrossStratgyTechVec* curPtr = generateTechVec(info);
	bool orderSingal = false;
	double k3 = calculateK(data, info, 3 * 60);
	double k5 = calculateK(data, info, 5 * 60);
	curPtr->setK3m(k3);
	curPtr->setK5m(k5);

	if (!data.empty()){
		if (curPtr->IsUpThough()){ // up
			if (!data.empty() && data.size() > 500){
				std::list<CThostFtdcDepthMDFieldWrapper>::const_iterator stoper = data.begin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = data.begin(); it != stoper; it++){
					StrategyTechVec* prePtr = it->GetTechVec();
					// if prePtr == NULL, mean it's recovered from db, so that md is not continuous. so it's should not be singal point.
					if (prePtr == NULL || !prePtr->IsUpThough())
					{
						// not special point
						orderSingal = false;
						break;
					}
					orderSingal = true;
				}
				//special point
				if (orderSingal){
					//update m_curOrder
					m_curOrder->SetInstrumentId(info.InstrumentId());
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(ExchangeDirection::Buy);
					m_curOrder->SetCombOffsetFlagType(Order::FAK);
					curPtr->SetTickType(TickType::BuyPoint);
				}
			}
		}
		else{ // down
			if (!data.empty() && data.size() > 500){
				std::list<CThostFtdcDepthMDFieldWrapper>::const_iterator stoper = data.begin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = data.begin(); it != stoper; it++){
					StrategyTechVec* prePtr = it->GetTechVec();
					if (prePtr == NULL || prePtr->IsUpThough())
					{
						// not special point
						orderSingal = false;
						break;
					}
					orderSingal = true;
				}
				if (orderSingal){
					//special point
					m_curOrder->SetInstrumentId(info.InstrumentId());
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(ExchangeDirection::Sell);
					m_curOrder->SetCombOffsetFlagType(Order::FAK);
					curPtr->SetTickType(TickType::SellPoint);
				}
			}
		}
	}

	//info.SetTechVec((StrategyTechVec*)curPtr);
	info.m_techvec = curPtr;
	return orderSingal;
}

Order MACrossStratgy::generateOrder(){
	assert(m_curOrder);
	return *m_curOrder;
}


bool MACrossStratgyTechVec::IsTableCreated = false;

MACrossStratgyTechVec::MACrossStratgyTechVec(CrossStratgyType type, long long uuid, const std::string& instrumentID, const std::string& time, double lastprice)
: m_type(type)
, m_id(uuid)
, m_ticktype(TickType::Commom)
, m_lastprice(lastprice)
{
	strcpy_s(m_time, time.c_str());
	strcpy_s(m_instrumentId, instrumentID.c_str());
}

bool MACrossStratgyTechVec::IsUpThough() const {
	return m_k3m > m_k5m;
}

int MACrossStratgyTechVec::CreateTableIfNotExists(const std::string& dbname, const std::string& tableName)
{
	if (MACrossStratgyTechVec::IsTableCreated == true){
		return 0;
	}
	else
	{
		MACrossStratgyTechVec::IsTableCreated = true;
		const char* sqltempl = "CREATE TABLE IF NOT EXISTS `%s`.`%s` (\
								`id` INT NOT NULL AUTO_INCREMENT, \
								`uuid` BIGINT NOT NULL, \
								`k5m` Double(20,5) NULL, \
								`k3m` Double(20,5) NULL, \
								`Ticktype` int NULL, \
								`Time` VARCHAR(64) NULL, \
								`LastPrice` Double NULL, \
								PRIMARY KEY(`id`));";
		char sqlbuf[2046];
		sprintf_s(sqlbuf, sqltempl, dbname.c_str(), tableName.c_str());
		DBWrapper db;
		return db.ExecuteNoResult(sqlbuf);
	}
}

void MACrossStratgyTechVec::serializeToDB(DBWrapper& db, const std::string& mark)
{
	std::string&& tableName = std::string(m_instrumentId) + "_" + StratgyType::toString(this->m_type) + "_CrossK3K5_" + mark;

	MACrossStratgyTechVec::CreateTableIfNotExists(Config::Instance()->DBName(), tableName);

	std::stringstream sql;
	sql.precision(12);
	sql << "INSERT INTO `" << tableName << "` (`";
	sql << "uuid" << "`,`";
	sql << "k5m" << "`,`";
	sql << "k3m" << "`,`";
	sql << "Ticktype" << "`,`";
	sql << "Time" << "`,`";
	sql << "LastPrice" << "`";
	sql << ") VALUES(";
	sql << m_id << ", ";
	sql << m_k5m << ", ";
	sql << m_k3m << ", ";
	sql << (int)m_ticktype << ", \"";
	sql << m_time << "\", ";
	sql << m_lastprice << ")";

	//std::cerr << sql.str() << std::endl;
	db.ExecuteNoResult(sql.str());
}