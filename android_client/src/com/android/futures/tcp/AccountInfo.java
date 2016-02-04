package com.android.futures.tcp;

import java.util.ArrayList;

public class AccountInfo {

	private ArrayList<String> mInstrumentList;
	private ArrayList<String> mStrategies;
	
	public AccountInfo(ArrayList<String> instrus, ArrayList<String> sts){
		setInstrumentList(instrus);
		setStrategies(sts);
	}

	public ArrayList<String> getStrategies() {
		return mStrategies;
	}

	public void setStrategies(ArrayList<String> mStrategies) {
		this.mStrategies = mStrategies;
	}

	public ArrayList<String> getInstrumentList() {
		return mInstrumentList;
	}

	public void setInstrumentList(ArrayList<String> mInstrumentList) {
		this.mInstrumentList = mInstrumentList;
	}
}
