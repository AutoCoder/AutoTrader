#ifndef IPositionControl_H
#define IPositionControl_H

class Order;

class IPositionControl
{
public:
	IPositionControl(){};
	virtual ~IPositionControl(){};

	//return fail or success
	virtual bool completeOrder(Order& ord) = 0;
};

class Pos20Precent : public IPositionControl{
public:
	Pos20Precent(){};
	~Pos20Precent(){};

	//return fail or success
	virtual bool completeOrder(Order& ord);
};

#endif