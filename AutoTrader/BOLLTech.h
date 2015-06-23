#ifndef BOLL_TECH_H
#define BOLL_TECH_H

#include "TechVec.h"


class BOLLTech : public StrategyTech
{
public:
	BOLLTech(){};
	BOLLTech(double variance, double ma_value, double cur_value);
	virtual ~BOLLTech();

	virtual void SetTickType(TickType type){
		m_ticktype = type;
	}

	virtual TickType GetTickType(){
		return m_ticktype;
	}

	virtual size_t ObjSize(){
		return sizeof(*this);
	}

	double BollWidth() const {
		return m_variance * m_boll_k * 2;
	}

	double BollUp() const {
		return m_ma_value + m_boll_k * m_variance;
	}

	double BollDown() const{
		return m_ma_value - m_boll_k * m_variance;
	}

	virtual	void serializeToDB(DBWrapper& db, const std::string& mark);
	virtual bool IsTriggerPoint() const;

private:
	static const size_t m_boll_k = 2;
	TickType m_ticktype;
	double m_ma_value;;
	double m_variance;
	double m_cur_value;
};

#endif