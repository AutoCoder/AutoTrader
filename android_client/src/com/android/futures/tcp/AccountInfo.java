package com.android.futures.tcp;

import java.util.ArrayList;

public class AccountInfo {

	private ArrayList<String> mInstrumentList;
	private ArrayList<String> mStrategies;
	private boolean           mIsTrading;
	
	public AccountInfo(ArrayList<String> instrus, ArrayList<String> sts, boolean is_trading){
		setInstrumentList(instrus);
		setStrategies(sts);
		setIsTrading(is_trading);
	}

	public ArrayList<String> getStrategies() {
		return mStrategies;
	}

	public void setStrategies(ArrayList<String> Strategies) {
		this.mStrategies = Strategies;
	}

	public ArrayList<String> getInstrumentList() {
		return mInstrumentList;
	}

	public void setInstrumentList(ArrayList<String> InstrumentList) {
		this.mInstrumentList = InstrumentList;
	}

	public boolean getIsTrading() {
		return mIsTrading;
	}

	public void setIsTrading(boolean mIsTrading) {
		this.mIsTrading = mIsTrading;
	}
}
