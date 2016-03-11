package com.android.futures.entity;

import java.text.SimpleDateFormat;
import java.util.Locale;

public class TradeEntity {
	public enum type{
		MD,
		Insert_Order,
		Cancell_Order,
		Trade,
		AccountInit,
	}
	
	private type mType;
	private int mOpenPrice;
	private int mClosePrice;
	private int mHighPrice;
	private int mLowPrice;
	private int   mVol;
	private long mTimeStamp;
	private String mOrderId;
	private int mLastPrice;
	private int mDirection; // 0 Long, 1 short
	private String mInstrument;
	

	public TradeEntity(int openPrice, int lastPrice, int highPrice, int lowPrice, int vol, long time){
		mType = type.MD;
		mOpenPrice = openPrice; 
		mLastPrice = lastPrice; 
		mHighPrice = highPrice;
		mLowPrice = lowPrice;
		mVol = vol;
		mTimeStamp = time;
	}
	
	public TradeEntity(type t, int direction, int lastprice, int vol, String ord_id, long time){
		mType = t;
		mVol = vol;
		mTimeStamp = time;
		mLastPrice = lastprice;
		mOrderId = ord_id;
		mDirection = direction;
	}
	
	public type getType() {
		return mType;
	}
	
	public String getTypeString() {
		String tradeType = "";
		switch (mType){
		case Insert_Order:
			tradeType = "报单";
			break;
		case Cancell_Order:
			tradeType = "撤单";
			break;
		case Trade:
			tradeType = "成交";
			break;
		default:
			tradeType = "未定义";
			break;
		}
		return tradeType;
	}

	public void setType(type mType) {
		this.mType = mType;
	}

	public float getOpenPrice() {
		return mOpenPrice;
	}

	public void setOpenPrice(int openPrice) {
		this.mOpenPrice = openPrice;
	}

	public float getClosePrice() {
		return mClosePrice;
	}

	public void setClosePrice(int closePrice) {
		this.mClosePrice = closePrice;
	}

	public float getHighPrice() {
		return mHighPrice;
	}

	public void setHighPrice(int highPrice) {
		this.mHighPrice = highPrice;
	}

	public float getLowPrice() {
		return mLowPrice;
	}

	public void setLowPrice(int lowPrice) {
		this.mLowPrice = lowPrice;
	}

	public int getVol() {
		return mVol;
	}

	public void setVol(int vol) {
		this.mVol = vol;
	}

	public long getTimeStamp() {
		return mTimeStamp;
	}

	public void setTimeStamp(long time) {
		this.mTimeStamp = time;
	}

	public String getOrderId() {
		return mOrderId;
	}

	public void setOrderId(String orderId) {
		this.mOrderId = orderId;
	}

	public float getLastPrice() {
		return mLastPrice;
	}

	public void setLastPrice(int price) {
		this.mLastPrice = price;
	}

	public int getDirection() {
		return mDirection;
	}

	public void setDirection(int mDirection) {
		this.mDirection = mDirection;
	}
	
	public String getDirectionString(){
		return mDirection == 0 ? "Long" : "Short";
	}
	
	public String getOccurTimeString(){
		java.util.Date curTime =  new java.util.Date(mTimeStamp * 500);
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.CHINA);
		return sdf.format(curTime);
	}

	public String getInstrument() {
		return mInstrument;
	}

	public void setInstrument(String mInstrument) {
		this.mInstrument = mInstrument;
	}
	
}
