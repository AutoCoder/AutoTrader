package com.android.futures;

import com.android.futures.tcp.ClientSessionNew;

import android.app.Application;

public class MyApp extends Application {
	private ClientSessionNew mSession = new ClientSessionNew();
	
	public ClientSessionNew GetSession(){
		return mSession;
	}
}
