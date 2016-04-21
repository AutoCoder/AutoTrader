package com.android.futures.tcp;

import java.util.ArrayList;

public class AccountInfo {

	private ArrayList<String> mInstrumentList;
	private ArrayList<String> mStrategies;
	private boolean           mIsTrading;
	private String            mRunningInstru;
	private String            mRunningStrategy;
	
	public AccountInfo(ArrayList<String> instrus, ArrayList<String> sts, boolean is_trading, String running_instu, String running_st){
		setInstrumentList(instrus);
		setStrategies(sts);
		setIsTrading(is_trading);
		setRunningInstrument(running_instu);
		setRunningStrategy(running_st);
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

	public void setIsTrading(boolean isTrading) {
		this.mIsTrading = isTrading;
	}

	public void setRunningInstrument(String running_instu) {
		this.mRunningInstru = running_instu;
	}

	public String getRunningInstrument(){
		return mRunningInstru;
	}

	public void setRunningStrategy(String running_st) {
		this.mRunningStrategy = running_st;
	}

	public String getRunningStrategy(){
		return mRunningStrategy;
	}
}
