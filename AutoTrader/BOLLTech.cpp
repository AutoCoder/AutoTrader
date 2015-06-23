#include "stdafx.h"
#include "BOLLTech.h"


BOLLTech::BOLLTech(double variance, double ma_value, double cur_value)
: m_ma_value(ma_value)
, m_variance(variance)
, m_cur_value(cur_value)
{
}


BOLLTech::~BOLLTech()
{
}


void BOLLTech::serializeToDB(DBWrapper& db, const std::string& mark){
	
}

bool BOLLTech::IsTriggerPoint() const{
	// if the cur_value is in [BollMd, BollUp], regards this point is BreakPoint
	return m_cur_value > (m_ma_value + m_boll_k * m_variance / 2);
}