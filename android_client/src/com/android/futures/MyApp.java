package com.android.futures;

import com.android.futures.tcp.ClientSession;
import com.squareup.leakcanary.LeakCanaryWrapper;
import com.squareup.leakcanary.RefWatcher;

import android.app.Application;

public class MyApp extends Application {
	private RefWatcher mRefWatcher;
	private ClientSession mSession = new ClientSession();
	
	public ClientSession GetSession(){
		return mSession;
	}
	
    
    @Override
    public void onCreate() {
        super.onCreate();

        mRefWatcher = LeakCanaryWrapper.install(this, BuildConfig.DEBUG);
    }
    
    public RefWatcher getRefWatcher() {
        return mRefWatcher;
    }
}
