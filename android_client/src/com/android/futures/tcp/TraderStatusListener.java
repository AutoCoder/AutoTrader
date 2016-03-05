package com.android.futures.tcp;

import com.android.futures.entity.TradeEntity;

public interface TraderStatusListener {
	public static final int LogOut = 1;
	public static final int Loging = 2;
	public static final int Logined = 3;
	public static final int LoginFailed = 4;
	public static final int AccountInfoUpdated = 5;
	public static final int PositionUpdated = 6;
	public static final int NoTrading = 7;
	public static final int TradeStarting = 8;
	public static final int Trading = 9;
	public static final int TradeStopping = 10;
	
	public static final int TradeNotification = 11;
	
	public void onPositionUpdated(PositionInfo status);
	public void onAccountInfoUpdated(AccountInfo info);
	public void onLoginSuccess();
	public void onLoginFailed(String err_msg);
	public void onLogOutSuccess();
	public void onStartTradeSuccess();
	public void onStartTradeFailed(String err_msg);
	public void onStopTrade();
	public void onCTPCallback(TradeEntity entity);
	public void onTradeNotification(TradeEntity entity);
}