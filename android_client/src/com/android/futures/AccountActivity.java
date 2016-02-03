package com.android.futures;

import com.android.futures.tcp.ClientSession;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.Menu;
import android.view.MenuItem;
import com.android.futures.MyApp;
import com.android.futures.tcp.ClientSession;;

public class AccountActivity extends Activity implements Handler.Callback  {
	private Handler mHandler = null;
	private ClientSession mSession = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_account);
		mHandler = new Handler(this);
		MyApp app = (MyApp) getApplication();
		mSession = app.GetSession();
		mSession.SetHandler(mHandler);
		mSession.Login();
	}

	@Override
	public boolean handleMessage(Message msg) {
		// TODO Auto-generated method stub
		if(msg.what == mSession.AccountInited) {
			System.out.println("hello");
		}
		return false;
	}
}
