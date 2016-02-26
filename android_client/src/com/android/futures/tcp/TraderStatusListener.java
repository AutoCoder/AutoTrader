package com.android.futures.tcp;

import com.android.futures.entity.TradeEntity;

public interface TraderStatusListener {
	public static final int LogOut = 1;
	public static final int Loging = 2;
	public static final int Logined = 3;
	public static final int PositionUpdated = 4;
	public static final int NoTrading = 5;
	public static final int TradeStarting = 6;
	public static final int Trading = 7;
	public static final int TradeStopping = 8;
	
	public void onPositionUpdated(PositionInfo status);
	public void onAccountLogined(AccountInfo info);
	public void onStartTradeSuccess();
	public void onStartTradeFailed(String err_msg);
	public void onStopTrade();
	public void onCTPCallback(TradeEntity entity);
}