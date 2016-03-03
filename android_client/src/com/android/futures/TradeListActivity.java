package com.android.futures;

import com.android.futures.tcp.ClientSession;
import com.android.futures.view.ListAdapter;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.ListView;

public class TradeListActivity extends Activity {
	private ListView listview;
	private ListAdapter adapter;
	private ClientSession mSession = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		try {
			super.onCreate(savedInstanceState);
			setContentView(R.layout.activity_listview);
		} catch (Exception e) {
			Log.e("ERROR", "ERROR IN CODE: " + e.toString());
			e.printStackTrace();
		}
		
		MyApp app = (MyApp) getApplication();
		mSession = app.GetSession();
		listview = (ListView) findViewById(R.id.listview);
		adapter = new ListAdapter(mSession.mTradeSequence, TradeListActivity.this);
		listview.setAdapter(adapter);
	}
}
