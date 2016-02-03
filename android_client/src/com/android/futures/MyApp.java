package com.android.futures;

import com.android.futures.tcp.ClientSession;

import android.app.Application;

public class MyApp extends Application {
	private ClientSession mSession = new ClientSession();
	
	public ClientSession GetSession(){
		return mSession;
	}
}
