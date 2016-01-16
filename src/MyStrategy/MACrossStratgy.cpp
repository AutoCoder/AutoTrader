//#include "stdafx.h"
#include "ConfigV2.h"
#include "DBWrapper.h"
#include "Order.h"
#include "MACrossStratgy.h"
#include "TickWrapper.h"
#include "KData.h"
#include "TechUtils.h"
#include <sstream>
#include <assert.h>
#include "crossplatform.h"

MACrossStratgy::MACrossStratgy(size_t short_ma, size_t long_ma)
: m_curOrder(new Order())//m_order is a pointer so that it will only update so, create it at constructor.
, m_shortMA(short_ma)
, m_longMA(long_ma)
{
}


MACrossStratgy::~MACrossStratgy()
{
	delete m_curOrder;
	m_curOrder = nullptr;
}

Order MACrossStratgy::GetCurOrder() const{
	return *m_curOrder;
}

// common MA 
double MACrossStratgy::calculateK(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds) const
{
	return TechUtils::CalulateMA(data, current, seconds);
}

double MACrossStratgy::calculateK(const std::vector<KData>& data, const KData& current, size_t mins) const{
	return TechUtils::CalulateMA(data, current, mins);
}

MACrossTech* MACrossStratgy::generateTechVec(const TickWrapper& info) const{
	return (new MACrossTech(CrossStratgyType::MA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}

bool MACrossStratgy::tryInvoke(const std::list<TickWrapper>& data, TickWrapper& info)
{
	TickType direction = TickType::Commom;
	const size_t breakthrough_confirm_duration = 100; //50ms
	MACrossTech* curPtr = generateTechVec(info);
	bool orderSingal = false;
	double short_ma = calculateK(data, info, m_shortMA * 60);
	double long_ma = calculateK(data, info, m_longMA * 60);
	curPtr->setShortMA(short_ma);
	curPtr->setLongMA(long_ma);

	if (!data.empty()){
		if (curPtr->IsTriggerPoint()){ // up
			if (!data.empty() && data.size() > 500){
				std::list<TickWrapper>::const_iterator stoper = data.begin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = data.begin(); it != stoper; it++){
					StrategyTech* prePtr = it->GetTechVec();
					// if prePtr == NULL, mean it's recovered from db, so that md is not continuous. so it's should not be singal point.
					if (prePtr == NULL || !prePtr->IsTriggerPoint())
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
					m_curOrder->SetOrderType(Order::LimitPriceFOKOrder);
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(THOST_FTDC_D_Buy);
					curPtr->SetTickType(TickType::BuyPoint);
				}
			}
		}
		else{ // down
			if (!data.empty() && data.size() > 500){
				std::list<TickWrapper>::const_iterator stoper = data.begin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = data.begin(); it != stoper; it++){
					StrategyTech* prePtr = it->GetTechVec();
					if (prePtr == NULL || prePtr->IsTriggerPoint())
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
					m_curOrder->SetOrderType(Order::LimitPriceFOKOrder);
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(THOST_FTDC_D_Sell);
					curPtr->SetTickType(TickType::SellPoint);
				}
			}
		}
	}

	//info.SetTechVec((StrategyTech*)curPtr);
	info.m_techvec = curPtr;
	return orderSingal;
}

bool MACrossStratgy::tryInvoke(const std::list<TickWrapper>& tickdata, const std::vector<KData>& data, std::vector<TickWrapper> curmindata, TickWrapper& info){
	TickType direction = TickType::Commom;
	const size_t breakthrough_confirm_duration = 100; //50ms
	MACrossTech* curPtr = generateTechVec(info);
	bool orderSingal = false;
	curmindata.push_back(info);
	KData curkdata(curmindata, 60);
	double short_ma = calculateK(data, curkdata, m_shortMA);
	double long_ma = calculateK(data, curkdata, m_longMA);
	curPtr->setShortMA(short_ma);
	curPtr->setLongMA(long_ma);

	if (!tickdata.empty()){
		if (curPtr->IsTriggerPoint())
		{ // up
			if (!tickdata.empty() && tickdata.size() > 500){
				std::list<TickWrapper>::const_iterator stoper = tickdata.begin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = tickdata.begin(); it != stoper; it++){
					StrategyTech* prePtr = it->GetTechVec();
					// if prePtr == NULL, mean it's recovered from db, so that md is not continuous. so it's should not be singal point.
					if (prePtr == NULL || !prePtr->IsTriggerPoint())
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
					m_curOrder->SetOrderType(Order::LimitPriceFOKOrder);
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(THOST_FTDC_D_Buy);
					curPtr->SetTickType(TickType::BuyPoint);
				}
			}
		}
		else
		{ // down
			if (!tickdata.empty() && tickdata.size() > 500){
				std::list<TickWrapper>::const_iterator stoper = tickdata.begin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = tickdata.begin(); it != stoper; it++){
					StrategyTech* prePtr = it->GetTechVec();
					if (prePtr == NULL || prePtr->IsTriggerPoint())
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
					m_curOrder->SetOrderType(Order::LimitPriceFOKOrder);
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(THOST_FTDC_D_Sell);
					curPtr->SetTickType(TickType::SellPoint);
				}
			}
		}
	}

	//info.SetTechVec((StrategyTech*)curPtr);
	info.m_techvec = curPtr;
	return orderSingal;
}

bool MACrossTech::IsTableCreated = false;

MACrossTech::MACrossTech(CrossStratgyType type, size_t shortMA, size_t longMA, long long uuid, const std::string& instrumentID, const std::string& time, double lastprice)
: m_type(type)
, m_id(uuid)
, m_ticktype(TickType::Commom)
, m_lastprice(lastprice)
, m_shortMA(shortMA)
, m_longMA(longMA)
, m_ma_tech()
{
	STRCPY(m_time, time.c_str());
	STRCPY(m_instrumentId, instrumentID.c_str());
}

bool MACrossTech::IsTriggerPoint() const {
	return m_ma_tech.IsTriggerPoint();
}

int MACrossTech::CreateTableIfNotExists(const std::string& dbname, const std::string& tableName)
{
	if (MACrossTech::IsTableCreated == true){
		return 0;
	}
	else
	{
		MACrossTech::IsTableCreated = true;
		const char* sqltempl = "CREATE TABLE IF NOT EXISTS `%s`.`%s` (\
		`id` INT NOT NULL AUTO_INCREMENT, \
		`uuid` BIGINT NOT NULL, \
		`LongMA` Double(20,5) NULL, \
		`ShortMA` Double(20,5) NULL, \
		`Ticktype` int NULL, \
		`Time` VARCHAR(64) NULL, \
		`LastPrice` Double NULL, \
		PRIMARY KEY(`id`));";
		char sqlbuf[2046];
		SPRINTF(sqlbuf, sqltempl, dbname.c_str(), tableName.c_str());
		DBWrapper db;
		return db.ExecuteNoResult(sqlbuf);
	}
}

void MACrossTech::serializeToDB(DBWrapper& db, const std::string& mark)
{
	std::stringstream tableName;
	tableName << std::string(m_instrumentId);
	tableName << "_";
	tableName << StratgyType::toString(this->m_type);
	tableName << "_MA" << m_shortMA << "_Cross_MA" << m_longMA << "_";
	tableName << mark;


	MACrossTech::CreateTableIfNotExists(ConfigV2::Instance()->DBName(), tableName.str());

	std::stringstream sql;
	sql.precision(12);
	sql << "INSERT INTO `" << tableName.str() << "` (`";
	sql << "uuid" << "`,`";
	sql << "LongMA" << "`,`";
	sql << "ShortMA" << "`,`";
	sql << "Ticktype" << "`,`";
	sql << "Time" << "`,`";
	sql << "LastPrice" << "`";
	sql << ") VALUES(";
	sql << m_id << ", ";
	sql << m_ma_tech.mLongMAVal << ", ";
	sql << m_ma_tech.mShortMAVal << ", ";
	sql << (int)m_ticktype << ", \"";
	sql << m_time << "\", ";
	sql << m_lastprice << ")";

	//std::cerr << sql.str() << std::endl;
	db.ExecuteNoResult(sql.str());
}