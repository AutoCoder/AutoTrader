package com.android.futures;

import com.squareup.leakcanary.RefWatcher;

import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTabHost;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;


public class MyFragmentActivity extends FragmentActivity implements OnClickListener{
	private static final Integer[] TABS = new Integer[] { R.layout.tab_times, R.layout.tab_kcharts};

	@Override
	protected void onCreate(Bundle bundle) {
		try {
			super.onCreate(bundle);
			setContentView(R.layout.activity4fragment_my);
			
			/*
			 * monitor memory leak
			*/
			MyApp app = (MyApp) getApplication();
			RefWatcher refWatcher = app.getRefWatcher();
		    refWatcher.watch(this);
		    
		} catch (Exception e) {
			Log.e("ERROR", "ERROR IN CODE: " + e.toString());
			e.printStackTrace();
		}
		FragmentTabHost tabHost = (FragmentTabHost) findViewById(android.R.id.tabhost);
		tabHost.setup(this, getSupportFragmentManager(), R.id.frame_content);

		tabHost.addTab(
				tabHost.newTabSpec(String.valueOf(TABS[0])).setIndicator(getLayoutInflater().inflate(TABS[0], null)),
				TimesFragment.class, null);
		tabHost.addTab(
				tabHost.newTabSpec(String.valueOf(TABS[1])).setIndicator(getLayoutInflater().inflate(TABS[1], null)),
				KChartsFragment.class, null);
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		switch (keyCode) {
		case KeyEvent.KEYCODE_BACK:
//			if ((System.currentTimeMillis() - mExitTime) > 2000) {
//				Toast.makeText(getApplicationContext(), "More once back key to exit", Toast.LENGTH_SHORT).show();
//				mExitTime = System.currentTimeMillis();
//			} else {
//				finish();
//			}
			finish();
			break;

		default:
			return super.onKeyDown(keyCode, event);
		}
		return true;
	}

	public void onClick(View view) {
		switch (view.getId()) {

		default:
			break;
		}

	}

//	@Override
//	public void onBackPressed(){
//		finish();
//	}
}
