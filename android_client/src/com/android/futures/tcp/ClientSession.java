package com.android.futures.tcp;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.Reader;
import java.net.Socket;
import java.nio.CharBuffer;

import org.json.JSONException;
import org.json.JSONObject;

import com.android.futures.entity.TradeEntity;


public class ClientSession {
	private Socket mSocket;
	enum state{
		Logined,
		Loging,
		LogOut,
		NoTrading,
		StartTrading,
		Trading
	}
	private state mState;
	
	private void Send(String data){
		try {
			PrintWriter writer = new PrintWriter(new OutputStreamWriter(mSocket.getOutputStream()));
	        writer.write(data);  
	        writer.flush();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}  
	}
	
	public void Login(String brokerId, String name, String pw){
		//96{"ActionType":"Login","Arguments":{"BrokerId":"9999","UserName":"021510","Password":"wodemima"}}
		try {
			JSONObject meta = new JSONObject();
			meta.put("BrokerId", brokerId);
			meta.put("UserName", name);
			meta.put("Password", pw);
			
			JSONObject loginJson = new JSONObject(); 
			loginJson.put("ActionType", "Login");
			loginJson.put("Arguments", meta);
			
			String info = loginJson.toString();
			String wrapInfo = String.valueOf(info.length()) + info;
			Send(wrapInfo);
			mState = state.Loging;
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
			loginJson.put("ActionType", "StartTrade");
			loginJson.put("Arguments", meta);
			
			String info = loginJson.toString();
			String wrapInfo = String.valueOf(info.length()) + info;
			Send(wrapInfo);
			mState = state.StartTrading;
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
	}
	
	
	class ReceiveThread extends Thread{  
        private Socket socket;  
          
        public ReceiveThread(Socket socket) {  
            this.socket = socket;  
        }  
  
        @Override  
        public void run() {  
            while(true){  
                try {                     
                    Reader reader = new InputStreamReader(socket.getInputStream());  
                    CharBuffer charBuffer = CharBuffer.allocate(8192); 
                    int index = -1;  
                    while((index=reader.read(charBuffer))!=-1){  
                        charBuffer.flip();
                        ParseCharBuffer(charBuffer);
                    }  
                } catch (Exception e) {  
                    e.printStackTrace();  
                }  
            }  
        }  
    }  
	
	private void ParseCharBuffer(CharBuffer charBuffer){
		int charCount = 0;
		for (Integer i =0; i < charBuffer.length(); i++){
			if (Character.isDigit(charBuffer.get(i))){
				charCount = charCount * 10 + Integer.valueOf(charBuffer.get(i));
			}else{
				char jsonstr[] = new char[charBuffer.length()-i];
				charBuffer.get(jsonstr, i, charBuffer.length()-i);
				try {
					JSONObject obj = new JSONObject(String.valueOf(jsonstr));
					if (obj.has("ActionType")){
						if (obj.getString("ActionType")=="Login" && mState == state.Loging){
							mState = obj.getInt("ErrorCode") == 0 ? state.Logined : state.LogOut;
						}else if(obj.getString("ActionType")=="StartTrade" && mState == state.StartTrading){
							mState = obj.getInt("ErrorCode") == 0 ? state.Trading : state.NoTrading;
						}
					}else{
						//return md & trade
						//{"Type":"MD","Details":{"OpenPrice":123,"ClosePrice":124, "HighPrice":125, "LowPrice":122,"Vol":500, "TIMESTAMP": 111111.5}}
						//{"Type":"INSERT_ORDER","Details":{"Direction" : 1, Price":123,"Vol":124, "ORDER_ID":11156, "TIMESTAMP": 111111.5}}
						//{"Type":"CANCELL_ORDER","Details":{"Direction" : 1, "Price":123,"Vol":124, "ORDER_ID":11156, "TIMESTAMP": 111111.5}}
						//{"Type":"TRADE","Details":{"Direction" : 1, "Price":124, "Vol":125,"ORDER_ID":11156, "TIMESTAMP": 111111.5}}
						if (obj.has("Type") && obj.getString("Type") == "MD"){
							TradeEntity temp;
							TradeEntity.type t = TradeEntity.type.MD;
							JSONObject details = obj.getJSONObject("Details");
							if (obj.getString("Type") == "MD"){
								temp = new TradeEntity(details.getInt("OpenPrice"), details.getInt("ClosePrice"), details.getInt("HighPrice"), details.getInt("LowPrice"), details.getInt("Vol"), details.getDouble("TIMESTAMP"));
							}else {
								if (obj.getString("Type") == "ORDER"){
									t = TradeEntity.type.Order;
								}else if (obj.getString("Type") == "CANCELL_ORDER"){
									t = TradeEntity.type.Cancell_Order;
								}else if (obj.getString("Type") == "TRADE"){
									t = TradeEntity.type.Trade;
								}else{}
								temp = new TradeEntity(t, details.getInt("Price"), details.getInt("Vol"), details.getInt("ORDER_ID"), details.getDouble("TIMESTAMP"));
							}
							
						}
					}
				} catch (JSONException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
	}
	
}
  