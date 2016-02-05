package com.android.futures.tcp;

import java.io.IOException;
import org.json.JSONException;
import org.json.JSONObject;

import com.android.futures.tcp.AccountStatus;
import android.os.Handler;
import android.os.Message;

public class ClientSessionNew implements TraderStatusListener {
	public int mState;
	private String mBrokerId, mAccount, mPassword, mHost;
	private int mPort;
	private Handler mHandler;
	private SocketHandler mSocketHandler = null;
	
	public void SetHandler(Handler handler){
		mHandler = handler;
	}
	
	public void ConnectServer(String brokerId, String account, String pwd, String host, int port)
	{
		mBrokerId = brokerId;
		mAccount = account;
		mPassword = pwd;
//		mHost = host;
//		mPort = port;
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
			mState = Loging;
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
			mState = LogOut;
			
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
			mState = StartTrading;
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

}
  