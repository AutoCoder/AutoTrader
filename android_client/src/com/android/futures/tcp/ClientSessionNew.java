package com.android.futures.tcp;

import java.io.IOException;
import org.json.JSONException;
import org.json.JSONObject;

import com.android.futures.tcp.AccountStatus;
import android.os.Handler;
import android.os.Message;

public class ClientSessionNew implements TraderStatusListener {

	private String mBrokerId, mAccount, mPassword, mHost;
	private int mPort;
	private Handler mHandler;
	private SocketHandler mSocketHandler = null;
	
	public void SetHandler(Handler handler){
		mHandler = handler;
	}
	
	public void ConnectServer(String brokerId, String account, String pwd, String host, int port)
	{
		if (mSocketHandler != null)//if socket is connected, skip this function.
			return;

		mBrokerId = brokerId;
		mAccount = account;
		mPassword = pwd;
		mHost = host;
		mPort = port;
		try {
			mSocketHandler = new SocketHandler(host, port, this);
			mSocketHandler.listen(true);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
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
			loginJson.put("Action", "LogOut");
			loginJson.put("Arguments", meta);
			String info = loginJson.toString();
			String wrapInfo = String.valueOf(info.length()) + info;
			mSocketHandler.sendMessage(wrapInfo);
			mSocketHandler.shutDown();
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
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
	}

	@Override
	public void onAccountInited(AccountStatus status) {
		// TODO Auto-generated method stub
		Message msg = Message.obtain();
		msg.obj = status;
		msg.what = TraderStatusListener.AccountInited;
		mHandler.sendMessage(msg);
	}

	@Override
	public void onAccountLogined(AccountInfo info) {
		// TODO Auto-generated method stub
		Message msg = Message.obtain();
		msg.obj = info;
		msg.what = TraderStatusListener.Logined;
		mHandler.sendMessage(msg);
	}

	@Override
	public void onStartTradeSuccess() {
		// TODO Auto-generated method stub
		Message msg = Message.obtain();
		msg.what = TraderStatusListener.Trading;
		mHandler.sendMessage(msg);		
	}
	
	@Override
	public void onStartTradeFailed(String err_msg){
		Message msg = Message.obtain();
		msg.what = TraderStatusListener.NoTrading;
		msg.obj = err_msg;
		mHandler.sendMessage(msg);		
	}

	@Override
	public void onStopTrade() {
		// TODO Auto-generated method stub
		Message msg = Message.obtain();
		msg.what = TraderStatusListener.TradeStarting;
		mHandler.sendMessage(msg);
	}

}
  