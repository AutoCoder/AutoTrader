package com.android.futures.entity;

import org.json.JSONObject;
import org.json.JSONException;


public class MATechInfo {

	private Integer           Short_MA;
	private Integer           Long_MA;
	private boolean           Empty;
	
	public MATechInfo(){
		Empty = true;
	}
	
	public MATechInfo(Integer short_ma, Integer long_ma){
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
			Long_MA = jsonobj.getInt("Long");
			Short_MA =  jsonobj.getInt("Short");
			Empty = false;
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public boolean IsEmpty(){
		return Empty;
	}

	public Integer getShort_MA() {
		return Short_MA;
	}

	public Integer getLong_MA(){
		return Long_MA;
	}

}
