#include "stdafx.h"
#include "IPositionControl.h"
#include "Order.h"
#include "crossplatform.h"
#include "AP_Mgr.h"


bool Pos20Precent::completeOrder(Order& ord){
	while (!AP::GetManager().isReady()){
		sleep(20);
	}

	double posMoney = 0.0;
	double available = 0.0;
	int volume = 0;
	AP::Direction posDirection = AP::Buy;
	AP::GetManager().GetPosition(posMoney, posDirection, volume, available);

	//�����������������ֲ���������һ��,���߲�λΪ0��
	if (ord.GetExchangeDirection() == posDirection || posMoney < std::numeric_limits<double>::epsilon()){
		if (posMoney > available * 0.2){
			//��λ�Ѿ�����2�ɣ� ������ö���
			return false;
		}
		else{
			double purchaseMoney = available*0.2 - posMoney;
			int vol = purchaseMoney / ord.GetRefExchangePrice();

			ord.SetCombOffsetFlagType(THOST_FTDC_OF_Open);
			ord.SetVolume(vol);
			return true;
		}
	}
	else{
		ord.SetCombOffsetFlagType(THOST_FTDC_OF_CloseToday);
		ord.SetVolume(volume);
		return true;
	}
}
