package com.android.futures.entity;

public class MATechInfo {

	private Integer           Short_MA;
	private Integer           Long_MA;
	private long 			  TimeStamp;
	
	public MATechInfo(Integer short_ma, Integer long_ma, long timestamp){
		Short_MA = short_ma;
		Long_MA = long_ma;
		TimeStamp = timestamp;
	}

	public Integer getShort_MA() {
		return Short_MA;
	}

	public Integer getLong_MA(){
		return Long_MA;
	}
	
	public long getTimeStamp(){
		return TimeStamp;
	}
}
