#include "stdafx.h"
#include "MACrossBOLLTech.h"


MACrossBOLLTech::MACrossBOLLTech()
{
}


MACrossBOLLTech::~MACrossBOLLTech()
{
}


bool MACrossBOLLTech::IsTriggerPoint() const
{
	return m_bollTech.IsTriggerPoint() && m_maCrossTech.IsTriggerPoint();
}

void MACrossBOLLTech::serializeToDB(DBWrapper& db, const std::string& mark)
{

}