package com.android.futures.tcp;


public class PositionInfo {
	private String Details;
	private double Balance;
	private double PositionMoney;
	
	PositionInfo(double balance, double positionMoney, String details){
		setBalance(balance);
		setPositionMoney(positionMoney);
		setDetails(details);
	}

	public String getDetails() {
		return Details;
	}

	public void setDetails(String details) {
		Details = details;
	}

	public double getBalance() {
		return Balance;
	}

	public void setBalance(double balance) {
		Balance = balance;
	}

	public double getPositionMoney() {
		return PositionMoney;
	}

	public void setPositionMoney(double positionMoney) {
		PositionMoney = positionMoney;
	}

}
