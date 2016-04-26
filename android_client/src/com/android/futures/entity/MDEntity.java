package com.android.futures.entity;

import org.json.JSONException;
import org.json.JSONObject;

public class MDEntity {
	private int OpenPrice;
	private int ClosePrice;
	private int HighPrice;
	private int LowPrice;
	private int LastPrice;
	private int Vol;
	private long TimeStamp;
	private String Instrument;
	public TechType Techtype;
	public MATechInfo TechMA;
	
	public MDEntity(){
		
	}
	
	public MDEntity(int openPrice, int lastPrice, int highPrice, int lowPrice, int vol, long time, String instru){
		setOpenPrice(openPrice); 
		setLastPrice(lastPrice); 
		setHighPrice(highPrice);
		setLowPrice(lowPrice);
		setVol(vol);
		setTimeStamp(time);
		setInstrument(instru);
	}
	
	// {"Info":"MD","Details":{"OpenPrice":123,"ClosePrice":124,
	// "HighPrice":125, "LowPrice":122,"Vol":500, "TIMESTAMP":
	// 111111.5, "ExtraData" : 	{
//			"Type" : "MA",
//			"Data" : {
//				"Long" : 1000,
//				"Short" : 500,
	//			}
	//		}
//		}
//	}

	public void fromJson(JSONObject jsonobj){
		try {
			OpenPrice = jsonobj.getInt("OpenPrice");
			LastPrice = jsonobj.getInt("LastPrice");
			HighPrice = jsonobj.getInt("HighPrice");
			LowPrice = jsonobj.getInt("LowPrice");
			Vol = jsonobj.getInt("Volume");
			TimeStamp = jsonobj.getLong("TIMESTAMP");
			
			JSONObject extra = jsonobj.getJSONObject("ExtraData");
			if (extra.has("Type")){
				if (extra.getString("Type").equals("MA")){
					TechMA.fromJson(extra.getJSONObject("Data"));
					Techtype = TechType.MA;
				}else{
					Techtype = TechType.Unknown;
				}
			}
			
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public int getOpenPrice() {
		return OpenPrice;
	}

	public void setOpenPrice(int openPrice) {
		OpenPrice = openPrice;
	}

	public int getClosePrice() {
		return ClosePrice;
	}

	public void setClosePrice(int closePrice) {
		ClosePrice = closePrice;
	}

	public int getHighPrice() {
		return HighPrice;
	}

	public void setHighPrice(int highPrice) {
		HighPrice = highPrice;
	}

	public int getLowPrice() {
		return LowPrice;
	}

	public void setLowPrice(int lowPrice) {
		LowPrice = lowPrice;
	}

	public int getLastPrice() {
		return LastPrice;
	}

	public void setLastPrice(int lastPrice) {
		LastPrice = lastPrice;
	}

	public int getVol() {
		return Vol;
	}

	public void setVol(int vol) {
		Vol = vol;
	}

	public long getTimeStamp() {
		return TimeStamp;
	}

	public void setTimeStamp(long timeStamp) {
		TimeStamp = timeStamp;
	}

	public String getInstrument() {
		return Instrument;
	}

	public void setInstrument(String instrument) {
		Instrument = instrument;
	}
}
