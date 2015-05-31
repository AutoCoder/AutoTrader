#include "stdafx.h"
#include "stdafx.h"
#include "config.h"
#include "DBWrapper.h"
#include "TechVec.h"
#include "Order.h"
#include "K3AVEThoughK5AVE.h"

#include <sstream>



K3AVEThoughK5AVE::K3AVEThoughK5AVE()
:m_curOrder(new Order())
{
}

K3AVEThoughK5AVE::~K3AVEThoughK5AVE()
{
}

double K3AVEThoughK5AVE::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const
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

bool K3AVEThoughK5AVE::TryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info)
{
	K3AVEThoughK5AVETechVec* curPtr = new K3AVEThoughK5AVETechVec(info.UUID(), info.InstrumentId());
	bool orderSingal = false;
	double k3 = calculateK(data, info, 3 * 60);
	double k5 = calculateK(data, info, 5 * 60);
	curPtr->setK3m(k3);
	curPtr->setK5m(k5);

	//assert(!data.empty());
	if (!data.empty())
	{
		auto preNode = data.begin();

		K3AVEThoughK5AVETechVec* prePtr = static_cast<K3AVEThoughK5AVETechVec*>(preNode->GetTechVec());
		if (prePtr){
			if (prePtr->K5m() > prePtr->K3m())
			{
				if (curPtr->K3m() > curPtr->K5m()){
					// Buy Singal
					// construct Buy Order ptr
					//std::cout << "[Buy Signal]" << std::endl;
					//std::cout << "LastPrice: " << info.LastPrice() << std::endl;
					//Order ord(info.InstrumentId(), info.LastPrice(), ExchangeDirection::Buy);
					m_curOrder->SetInstrumentId(info.InstrumentId());
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(ExchangeDirection::Buy);
					curPtr->SetTickType(TickType::BuyPoint);
					orderSingal = true;
				}
			}
			else{
				if (curPtr->K3m() < curPtr->K5m()){
					//Sell Singal
					// construct Sell Order ptr
					//std::cout << "[Sell Signal]" << std::endl;
					//std::cout << "LastPrice: " << info.LastPrice() << std::endl;
					//Order ord(info.InstrumentId(), info.LastPrice(), ExchangeDirection::Sell);
					m_curOrder->SetInstrumentId(info.InstrumentId());
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(ExchangeDirection::Sell);
					curPtr->SetTickType(TickType::SellPoint);
					orderSingal = true;
				}
			}
		}

	}

	//info.SetTechVec((StrategyTechVec*)curPtr);
	info.m_techvec = curPtr;
	return orderSingal;
}

Order K3AVEThoughK5AVE::generateOrder(){
	return *m_curOrder;
}

bool K3AVEThoughK5AVETechVec::IsTableCreated = false;

K3AVEThoughK5AVETechVec::K3AVEThoughK5AVETechVec(long long uuid, const std::string& instrumentID)
: m_id(uuid)
, m_instrumentId(instrumentID)
, m_ticktype(TickType::Commom)
{
}

int K3AVEThoughK5AVETechVec::CreateTableIfNotExists(const std::string& dbname, const std::string& tableName){
	if (K3AVEThoughK5AVETechVec::IsTableCreated == true){
		return 0;
	}
	else
	{
		K3AVEThoughK5AVETechVec::IsTableCreated = true;
		const char* sqltempl = "CREATE TABLE IF NOT EXISTS `%s`.`%s` (\
				`id` INT NOT NULL AUTO_INCREMENT, \
				`uuid` BIGINT NOT NULL, \
				`m_k5closepriceave` Double(20,5) NULL, \
				`m_k3closepriceave` Double(20,5) NULL, \
				`Ticktype` int NULL, \
				PRIMARY KEY(`id`));";
		char sqlbuf[2046];
		sprintf_s(sqlbuf, sqltempl, dbname.c_str(), tableName.c_str());
		DBWrapper db;
		return db.ExecuteNoResult(sqlbuf);
	}
}

void K3AVEThoughK5AVETechVec::serializeToDB(DBWrapper& db, const std::string& mark)
{
	std::string&& tableName = m_instrumentId + "_K3AVEThoughK5AVE_" + mark;

	K3AVEThoughK5AVETechVec::CreateTableIfNotExists(Config::Instance()->DBName(), tableName);

	std::stringstream sql;
	sql << "INSERT INTO `" << tableName << "` (`";
	sql << "uuid" << "`,`";
	sql << "m_k5closepriceave" << "`,`";
	sql << "m_k3closepriceave" << "`,`";
	sql << "Ticktype" << "`";
	sql << ") VALUES(";
	sql << m_id << ", ";
	sql << m_k5closepriceave << ", ";
	sql << m_k3closepriceave << ", ";
	sql << (int)m_ticktype << ")";

	//std::cerr << sql.str() << std::endl;
	db.ExecuteNoResult(sql.str());
}