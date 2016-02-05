package com.android.futures.tcp;

public interface TraderStatusListener {
	public static final int LogOut = 1;
	public static final int Loging = 2;
	public static final int Logined = 3;
	public static final int AccountInited = 4;
	public static final int NoTrading = 5;
	public static final int TradeStarting = 6;
	public static final int Trading = 7;
	public static final int TradeStopping = 8;
	
	public void onAccountInited(AccountStatus status);
	public void onAccountLogined(AccountInfo info);
	public void onStartTrade();
	public void onStopTrade();
}