package com.android.futures.tcp;


public class PositionInfo {
	private String   mInstrument;
	private double mBalance;
	private int   mPosition;
	private int mPrice;
	
	PositionInfo(double balance, int position, int price, String instru){
		setBalance(balance);
		setPosition(position);
		setPrice(price);
		setInstrument(instru);
	}

	public double getBalance() {
		return mBalance;
	}

	public void setBalance(double mBalance) {
		this.mBalance = mBalance;
	}

	public String getInstrument() {
		return mInstrument;
	}

	public void setInstrument(String mInstrument) {
		this.mInstrument = mInstrument;
	}

	public int getPosition() {
		return mPosition;
	}

	public void setPosition(int mPosition) {
		this.mPosition = mPosition;
	}

	public int getPrice() {
		return mPrice;
	}

	public void setPrice(int mPrice) {
		this.mPrice = mPrice;
	}
}
