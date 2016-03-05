package com.android.futures.tcp;

import java.io.IOException;
import java.util.Vector;

import org.json.JSONException;
import org.json.JSONObject;

import com.android.futures.entity.TradeEntity;
import com.android.futures.tcp.PositionInfo;
import android.os.Handler;
import android.os.Message;

public class ClientSession implements TraderStatusListener {

	private String mBrokerId, mAccount, mPassword;
	private String mCurrentInstrument = new String("");
	private String mStrategyName = new String("");
	private Handler mHandler;
	private SocketHandler mSocketHandler = null;
	public int State = LogOut;
	public Vector<TradeEntity> mMdSequence = new Vector<TradeEntity>();
	public Vector<TradeEntity> mTradeSequence = new Vector<TradeEntity>();
	
	public void SetHandler(Handler handler){
		mHandler = handler;
	}
	
	public void ConnectServer(String brokerId, String account, String pwd, String host, int port) throws IOException
	{
		if (mSocketHandler != null)//if socket is connected, skip this function.
			return;

		mBrokerId = brokerId;
		mAccount = account;
		mPassword = pwd;

		mSocketHandler = new SocketHandler(host, port, this);
		mSocketHandler.listen(true);
	}
	
	public void Login(){
		JSONObject meta = new JSONObject();
		
		try {
			meta.put("BrokerId", mBrokerId);
			meta.put("UserName", mAccount);
			meta.put("Password", mPassword);
			JSONObject loginJson = new JSONObject(); 
			loginJson.put("Action", "Login");
			loginJson.put("Arguments", meta);
			String info = loginJson.toString();
			String wrapInfo = String.valueOf(info.length()) + info;
			mSocketHandler.sendMessage(wrapInfo);
			State = AccountInfoUpdated;
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public void LogOut(){
		JSONObject meta = new JSONObject();
		
		try {
			meta.put("BrokerId", mBrokerId);
			meta.put("UserName", mAccount);
			meta.put("Password", mPassword);
			JSONObject loginJson = new JSONObject(); 
			loginJson.put("Action", "Logout");
			loginJson.put("Arguments", meta);
			String info = loginJson.toString();
			String wrapInfo = String.valueOf(info.length()) + info;
			mSocketHandler.sendMessage(wrapInfo);

		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}		
	}
	
	public void StartTrade(String strategyName, String instrument){
		//{"ActionType":"StartTrade","Arguments":{"InstrumentId":"rb1604","StrategyName":"Pos20Precent_3_5_MACrossStratgy"}}
		try {
			JSONObject meta = new JSONObject();
			meta.put("InstrumentId", instrument);
			meta.put("StrategyName", strategyName);
			
			JSONObject loginJson = new JSONObject(); 
			loginJson.put("Action", "StartTrade");
			loginJson.put("Arguments", meta);
			
			String info = loginJson.toString();
			String wrapInfo = String.valueOf(info.length()) + info;
			mSocketHandler.sendMessage(wrapInfo);
			setCurrentInstrument(instrument);
			setStrategyName(strategyName);
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
	}
	public void StopTrade(){
		//{"ActionType":"StopTrade"}
		try {
			
			JSONObject json = new JSONObject(); 
			json.put("Action", "StopTrade");
			String info = json.toString();
			String wrapInfo = String.valueOf(info.length()) + info;
			mSocketHandler.sendMessage(wrapInfo);
			setCurrentInstrument("");
			setStrategyName("");
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
	}
	
	public void QueryPosition(){
		try {
			JSONObject json = new JSONObject(); 
			json.put("Action", "QueryPosition");
			String info = json.toString();
			String wrapInfo = String.valueOf(info.length()) + info;
			mSocketHandler.sendMessage(wrapInfo);
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 		
	}
	
	@Override
	public void onPositionUpdated(PositionInfo status) {
		// TODO Auto-generated method stub
		Message msg = Message.obtain();
		msg.obj = status;
		msg.what = TraderStatusListener.PositionUpdated;
		mHandler.sendMessage(msg);
	}

	@Override
	public void onAccountInfoUpdated(AccountInfo info) {
		// TODO Auto-generated method stub
		Message msg = Message.obtain();
		msg.obj = info;
		msg.what = TraderStatusListener.AccountInfoUpdated;
		mHandler.sendMessage(msg);
	}

	@Override
	public void onStartTradeSuccess() {
		// TODO Auto-generated method stub
		Message msg = Message.obtain();
		msg.what = TraderStatusListener.Trading;
		mHandler.sendMessage(msg);		
		State = Trading;
	}
	
	@Override
	public void onStartTradeFailed(String err_msg){
		Message msg = Message.obtain();
		msg.what = TraderStatusListener.NoTrading;
		msg.obj = err_msg;
		mHandler.sendMessage(msg);	
		State = NoTrading;
	}

	@Override
	public void onStopTrade() {
		//if stop trade action is success replied from socket server
		// send message to update Activity
		Message msg = Message.obtain();
		msg.what = TraderStatusListener.NoTrading;
		mHandler.sendMessage(msg);
		
		// update state, clear current tick queue.
		State = NoTrading;
		mMdSequence.clear();
	}

	@Override
	public void onCTPCallback(TradeEntity entity) {
		// TODO Auto-generated method stub
		mMdSequence.add(entity);
		if (entity.getType() != TradeEntity.type.MD){
			onTradeNotification(entity);
		}
	}

	public String getCurrentInstrument() {
		return mCurrentInstrument;
	}

	public void setCurrentInstrument(String mCurrentInstrument) {
		this.mCurrentInstrument = mCurrentInstrument;
	}

	public String getStrategyName() {
		return mStrategyName;
	}

	public void setStrategyName(String mStrategyName) {
		this.mStrategyName = mStrategyName;
	}

	@Override
	public void onLoginSuccess() {
		// TODO Auto-generated method stub
		Message msg = Message.obtain();
		msg.what = TraderStatusListener.Logined;
		mHandler.sendMessage(msg);		
		State = Logined;		
	}

	@Override
	public void onLoginFailed(String err_msg) {
		// TODO Auto-generated method stub
		Message msg = Message.obtain();
		msg.what = TraderStatusListener.LoginFailed;
		msg.obj = err_msg;
		mHandler.sendMessage(msg);		
		State = LoginFailed;			
	}
	
	@Override
	public void onLogOutSuccess(){
		mSocketHandler.shutDown();
		mSocketHandler = null;
		State = LogOut;
		Message msg = Message.obtain();
		msg.what = TraderStatusListener.LogOut;
		mHandler.sendMessage(msg);
	}

	@Override
	public void onTradeNotification(TradeEntity entity) {
		// TODO Auto-generated method stub
		mTradeSequence.add(entity);
		Message msg = Message.obtain();
		msg.what = TraderStatusListener.TradeNotification;
		msg.obj = entity;
		mHandler.sendMessage(msg);
	}

}
  