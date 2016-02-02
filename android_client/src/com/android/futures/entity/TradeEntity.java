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
	private double mTime;
	private int   mOrderId;
	private int mPrice;

	public TradeEntity(int openPrice, int closePrice, int highPrice, int lowPrice, int vol, double time){
		mType = type.MD;
		mOpenPrice = openPrice; 
		mClosePrice = closePrice; 
		mHighPrice = highPrice;
		mLowPrice = lowPrice;
		mVol = vol;
		mTime = time;
	}
	
	public TradeEntity(type t, int price, int vol, int ord_id, double time){
		mType = t;
		mVol = vol;
		mTime = time;
		mPrice = price;
		mOrderId = ord_id;
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

	public double getTime() {
		return mTime;
	}

	public void setTime(double time) {
		this.mTime = time;
	}

	public int getOrderId() {
		return mOrderId;
	}

	public void setOrderId(int orderId) {
		this.mOrderId = orderId;
	}

	public float getPrice() {
		return mPrice;
	}

	public void setPrice(int price) {
		this.mPrice = price;
	}
	
}
