package com.android.futures;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;

import java.util.ArrayList;

//import android.view.Menu;
//import android.view.MenuItem;
import com.android.futures.MyApp;
import com.android.futures.tcp.AccountInfo;
import com.android.futures.tcp.ClientSessionNew;
import com.android.futures.tcp.TraderStatusListener;

public class AccountActivity extends Activity implements Handler.Callback {
	private Handler mHandler = null;
	private ClientSessionNew mSession = null;
	private Spinner mInstrumentList = null;
	private Spinner mStrategyList = null;
	private Button logOutBtn = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		try {
			super.onCreate(savedInstanceState);
			setContentView(R.layout.activity_account);
		} catch (Exception e) {
			Log.e("ERROR", "ERROR IN CODE: " + e.toString());
			e.printStackTrace();
		}
		mHandler = new Handler(this);
		mInstrumentList = (Spinner) this.findViewById(R.id.instrument_List);
		mStrategyList = (Spinner) this.findViewById(R.id.strategy_list);
		logOutBtn = (Button) this.findViewById(R.id.logout);
		
		logOutBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				
				MyApp app = (MyApp) getApplication();
				ClientSessionNew session = app.GetSession();
				session.LogOut();
				
			    Intent intent = new Intent(AccountActivity.this, LoginActivity.class);  
                startActivity(intent);
			}
		});
		
		MyApp app = (MyApp) getApplication();
		mSession = app.GetSession();
		mSession.SetHandler(mHandler);
		mSession.Login();
	}

	@Override
	public boolean handleMessage(Message msg) {
		// TODO Auto-generated method stub
		if (msg.what == TraderStatusListener.AccountInited) {
			System.out.println("AccountInited");
		} else if (msg.what == TraderStatusListener.Logined) {
			AccountInfo info = (AccountInfo) msg.obj;
			ArrayList<String> instrus = info.getInstrumentList();
			ArrayAdapter<String> adapter1 = new ArrayAdapter<String>(AccountActivity.this,
					android.R.layout.simple_spinner_item, instrus);
			mInstrumentList.setAdapter(adapter1);

			ArrayList<String> sts = info.getStrategies();
			ArrayAdapter<String> adapter2 = new ArrayAdapter<String>(AccountActivity.this,
					android.R.layout.simple_spinner_item, sts);
			mStrategyList.setAdapter(adapter2);
		}
		else if (msg.what == TraderStatusListener.TradeStarting){
		    Intent intent = new Intent(AccountActivity.this, MyFragmentActivity.class);  
            startActivity(intent);
		}
		return false;
	}
}
