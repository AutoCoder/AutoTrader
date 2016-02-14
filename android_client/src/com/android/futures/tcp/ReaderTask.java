package com.android.futures.tcp;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.net.SocketException;
import java.nio.CharBuffer;
import java.util.ArrayList;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import com.android.futures.entity.TradeEntity;

public class ReaderTask extends Thread {

	private SocketStatusListener socketStatusListener;
	private TraderStatusListener statusChangeHandler;
	private BufferedReader bufferedReader;
	private Socket socket;
	private boolean listening;

	public ReaderTask(Socket socket, TraderStatusListener delegate) throws IOException {
		bufferedReader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		statusChangeHandler = delegate;
		this.socket = socket;
	}

	/**
	 * finish:(这里用一句话描述这个方法的作用). <br/>
	 * TODO(这里描述这个方法适用条件 – 可选).<br/>
	 * 
	 * @throws IOException
	 *
	 */
	public void finish() throws IOException {
		listening = false;
		interrupt();
		if (bufferedReader != null && socket != null) {
			if (socket.isInputShutdown()) {
				socket.shutdownInput();
			}
			bufferedReader.close();
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Runnable#run()
	 */
	@Override
	public synchronized void run() {
		while (listening) {
			try {
				String readStr = null;
				while ((readStr = bufferedReader.readLine()) != null) {
					ParseCharBuffer(readStr);
				}
			} catch (IOException e) {
				listening = false;
				if (socketStatusListener != null) {
					int status = parseSocketStatus(e);
					socketStatusListener.onSocketStatusChanged(socket, status, e);
				}
				e.printStackTrace();
				continue;// 终止线程继续运行,这里也可以使用continue
			}
		}
	}

	private int parseSocketStatus(IOException e) {
		if (SocketException.class.isInstance(e)) {
			String msg = e.getLocalizedMessage().trim();
			if ("Connection reset".equalsIgnoreCase(msg)) {
				return SocketStatusListener.STATUS_RESET;
			} else if ("Socket is closed".equalsIgnoreCase(msg)) {
				return SocketStatusListener.STATUS_CLOSE;
			} else if ("Broken pipe".equalsIgnoreCase(msg)) {
				return SocketStatusListener.STATUS_PIP_BROKEN;
			}

		}
		return SocketStatusListener.STATUS_UNKOWN;
	}

	/**
	 * listen:(这里用一句话描述这个方法的作用). <br/>
	 * TODO(这里描述这个方法适用条件 – 可选).<br/>
	 *
	 */
	public void startListener(SocketStatusListener ssl) {
		listening = true;
		this.socketStatusListener = ssl;
		start();
	}

	private void ParseCharBuffer(String readstr) {
		int charCount = 0;
		int jsonstart = 0;
		while (true) {
			char input = readstr.charAt(jsonstart);
			if (Character.isDigit(input)) {
				int num = Integer.valueOf(input + "");
				charCount = charCount * 10 + num;
				jsonstart++;
			} else {
				break;
			}
		}
		if (charCount != 0) {
			try {
				// int temp2 = charBuffer.remaining();
				// char jsonstr[] = new char[temp2];
				// charBuffer.get(jsonstr);
				String jsonStr = readstr.substring(jsonstart, jsonstart + charCount - 1);
				JSONObject obj = new JSONObject(jsonStr);
				if (obj.has("Action")) {
					String actionType = obj.getString("Action");
					if (actionType.equals("Login")) {

					} else if (actionType.equals("StartTrade")) {
						boolean sucess = (obj.getInt("ErrorCode") == 0);
						if (statusChangeHandler != null){
							if (sucess)
								statusChangeHandler.onStartTradeSuccess();
							else
							{
								String err_str = obj.getString("ErrorMsg");
								statusChangeHandler.onStartTradeFailed(err_str);
							}
						}
					} else if (actionType.equals("StopTrade")) {
						if (statusChangeHandler != null){
							statusChangeHandler.onStopTrade();
						}
					}
					else{
						boolean elsebranch = true;
					}
				} else if (obj.has("Info")) {
					String infoType = obj.getString("Info");
					TradeEntity temp;
					TradeEntity.type t = TradeEntity.type.MD;
					JSONObject details = obj.getJSONObject("Details");
					// return md & trade
					// {"Info":"MD","Details":{"OpenPrice":123,"ClosePrice":124,
					// "HighPrice":125, "LowPrice":122,"Vol":500, "TIMESTAMP":
					// 111111.5}}
					// {"Info":"INSERT_ORDER","Details":{"Direction" : 1,
					// Price":123,"Vol":124, "ORDER_ID":11156, "TIMESTAMP":
					// 111111.5}}
					// {"Info":"CANCELL_ORDER","Details":{"Direction" : 1,
					// "Price":123,"Vol":124, "ORDER_ID":11156, "TIMESTAMP":
					// 111111.5}}
					// {"Info":"TRADE","Details":{"Direction" : 1, "Price":124,
					// "Vol":125,"ORDER_ID":11156, "TIMESTAMP": 111111.5}}
					// {"Info":"ACCOUNT_INFO","Details":{"Blance":122313,"Position":20,
					// "Instrument":"rb1605", "Price":2555}}
					if (infoType.equals("MD")) {
						temp = new TradeEntity(details.getInt("OpenPrice"), details.getInt("ClosePrice"),
								details.getInt("HighPrice"), details.getInt("LowPrice"), details.getInt("Volume"),
								details.getDouble("TIMESTAMP"));
						statusChangeHandler.onMDReturn(temp);
					} else if (infoType.equals("ORDER")) {
						t = TradeEntity.type.Order;
					} else if (infoType.equals("CANCELL_ORDER")) {
						t = TradeEntity.type.Cancell_Order;
					} else if (infoType.equals("TRADE")) {
						t = TradeEntity.type.Trade;
					} else if (infoType.equals("ACCOUNT_STATUS")) {
						AccountStatus status = new AccountStatus(details.getDouble("Balance"),
								details.getInt("Position"), details.getInt("Price"), details.getString("Instrument"));
						if (statusChangeHandler != null)
							statusChangeHandler.onAccountInited(status);
					} else if (infoType.equals("ACCOUNT_INFO")) {
						JSONArray instrus = details.getJSONArray("Instruments");
						JSONArray sts = details.getJSONArray("Strategies");
						ArrayList<String> instru_list = new ArrayList<String>();
						ArrayList<String> st_list = new ArrayList<String>();
						for (int i = 0; i < instrus.length(); ++i) {
							instru_list.add(instrus.getString(i));
						}
						for (int i = 0; i < sts.length(); ++i) {
							st_list.add(sts.getString(i));
						}
						AccountInfo info = new AccountInfo(instru_list, st_list);
						if (statusChangeHandler != null)
							statusChangeHandler.onAccountLogined(info);
					} else {
			
					}
				}
			} catch (JSONException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		} else {
			return;
		}
	}
}