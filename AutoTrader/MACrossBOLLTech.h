#ifndef MACROSS_BOLL_TECH_H
#define MACROSS_BOLL_TECH_H

#include "TechVec.h"
#include "BOLLTech.h"
#include "MACrossStratgy.h"

class MACrossBOLLTech : public StrategyTech
{
public:
	MACrossBOLLTech();
	virtual ~MACrossBOLLTech();

	

	virtual void SetTickType(TickType type){
		m_ticktype = type;
	}

	virtual TickType GetTickType(){
		return m_ticktype;
	}

	virtual size_t ObjSize(){
		return sizeof(*this);
	}

	virtual	void serializeToDB(DBWrapper& db, const std::string& mark);
	virtual bool IsTriggerPoint() const;

private:
	BOLLTech m_bollTech;
	MACrossTech m_maCrossTech;
	TickType m_ticktype;
};

#endif