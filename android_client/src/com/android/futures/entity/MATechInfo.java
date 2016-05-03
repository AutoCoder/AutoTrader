package com.android.futures.entity;

import org.json.JSONObject;
import org.json.JSONException;


public class MATechInfo {

	private Double            Short_MA;
	private Double            Long_MA;
	private boolean           Empty;
	
	public MATechInfo(){
		Empty = true;
	}
	
	public MATechInfo(Double short_ma, Double long_ma){
		Short_MA = short_ma;
		Long_MA = long_ma;
		Empty = false;
	}
	
//	"Data" : {
	//	"Long" : 1000,
	//	"Short" : 500,
//	}
	public void fromJson(JSONObject jsonobj){
		try {
			Long_MA = jsonobj.getDouble("Long");
			Short_MA =  jsonobj.getDouble("Short");
			Empty = false;
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public boolean IsEmpty(){
		return Empty;
	}

	public Double getShort_MA() {
		return Short_MA;
	}

	public Double getLong_MA(){
		return Long_MA;
	}

}
