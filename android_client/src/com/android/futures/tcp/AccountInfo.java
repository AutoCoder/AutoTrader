package com.android.futures.tcp;


public class AccountInfo {
	private String   mInstrument;
	private double mBalance;
	private int   mPosition;
	private int mPrice;
	
	AccountInfo(double balance, int position, int price, String instru){
		mBalance = balance;
		mPosition = position;
		mPrice = price;
		mInstrument = instru;
	}
}
