#ifndef IPositionControl_H
#define IPositionControl_H


class Order;
namespace AP{
	class AccountDetailMgr;
};

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
	explicit Pos20Precent();
	~Pos20Precent(){};

	void BindAccount(AP::AccountDetailMgr* mgr) { m_detailMgr = mgr; }

	//return fail or success
	virtual bool completeOrder(Order& ord);

private:
	AP::AccountDetailMgr* m_detailMgr;
};

#endif