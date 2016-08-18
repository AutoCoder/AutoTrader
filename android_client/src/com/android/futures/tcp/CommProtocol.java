package com.android.futures.tcp;

public final class CommProtocol {
	public static final String sAction = "Action";
	public static final String sArguments = "Arguments";

	public static final class Action {
	    public static final String LogIn = "Login";
	    public static final String LogOut = "Logout";
	    public static final String StartTrade = "StartTrade";
	    public static final String StopTrade = "StopTrade";
	    public static final String QueryPosition = "QueryPosition";
	    public static final String TurnOnTickReceiving = "TurnOnTickReceiving";
	    public static final String TurnOffTickReceiving = "TurnOffTickReceiving";
	    public static final String TurnOnSemiAuto = "TurnOnSemiAuto";
	    public static final String TurnOffSemiAuto = "TurnOffSemiAuto";
	}
	    
	public static final class Argument {
	    public static final String BrokerId = "BrokerId";
	    public static final String UserName = "UserName";
	    public static final String Password = "Password";
	    public static final String InstrumentId = "InstrumentId";
	    public static final String StrategyName = "StrategyName";
	}
}