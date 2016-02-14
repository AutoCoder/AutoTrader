package com.android.futures.entity;

public class TradeEntity {
	public enum type{
		MD,
		Order,
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
	private int mPrice;
	private int mDirection; // 0 sell, 1 buy

	public TradeEntity(int openPrice, int closePrice, int highPrice, int lowPrice, int vol, long time){
		mType = type.MD;
		mOpenPrice = openPrice; 
		mClosePrice = closePrice; 
		mHighPrice = highPrice;
		mLowPrice = lowPrice;
		mVol = vol;
		mTimeStamp = time;
	}
	
	public TradeEntity(type t, int direction, int price, int vol, String ord_id, long time){
		mType = t;
		mVol = vol;
		mTimeStamp = time;
		mPrice = price;
		mOrderId = ord_id;
		mDirection = direction;
	}
	
	public type getType() {
		return mType;
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

	public float getPrice() {
		return mPrice;
	}

	public void setPrice(int price) {
		this.mPrice = price;
	}

	public int getDirection() {
		return mDirection;
	}

	public void setDirection(int mDirection) {
		this.mDirection = mDirection;
	}
	
}
