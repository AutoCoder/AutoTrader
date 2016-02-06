package com.android.futures;

import com.android.futures.tcp.ClientSessionNew;
import com.android.futures.tcp.TraderStatusListener;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTabHost;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public class MyFragmentActivity extends FragmentActivity implements OnClickListener, Handler.Callback {
	private static final Integer[] TABS = new Integer[] { R.layout.tab_times, R.layout.tab_kcharts};
	private static final int WHAT = 1987;
	private Button mBack;
	private Button mLeft;
	private Button mRight;
	private Button mRefresh;
	private ProgressDialog mProgressDialog;
	private long mExitTime;
	private Handler mHandler = null;
	private ClientSessionNew mSession = null;
	private String mTradingInstrument = null;
	private String mTradingStrategy = null;


	@Override
	protected void onCreate(Bundle bundle) {
		super.onCreate(bundle);
		setContentView(R.layout.activity4fragment_my);
		initViews();

		FragmentTabHost tabHost = (FragmentTabHost) findViewById(android.R.id.tabhost);
		tabHost.setup(this, getSupportFragmentManager(), R.id.frame_content);

		tabHost.addTab(
				tabHost.newTabSpec(String.valueOf(TABS[0])).setIndicator(getLayoutInflater().inflate(TABS[0], null)),
				TimesFragment.class, null);
		tabHost.addTab(
				tabHost.newTabSpec(String.valueOf(TABS[1])).setIndicator(getLayoutInflater().inflate(TABS[1], null)),
				KChartsFragment.class, null);
		
		mHandler = new Handler(this);
		MyApp app = (MyApp) getApplication();
		mSession = app.GetSession();
		mSession.SetHandler(mHandler);
		
		Intent intent=getIntent(); 
	    Bundle trade_data = intent.getExtras();
	    mTradingInstrument = trade_data.getString("instrument");
	    mTradingStrategy = trade_data.getString("strategy");
		mSession.StartTrade(mTradingInstrument, mTradingStrategy);
	}

	private void initViews() {
		mBack = (Button) findViewById(R.id.title_back_btn);
		mBack.setOnClickListener(this);
		mLeft = (Button) findViewById(R.id.title_left_btn);
		mLeft.setOnClickListener(this);
		mRight = (Button) findViewById(R.id.title_right_btn);
		mRight.setOnClickListener(this);
		mRefresh = (Button) findViewById(R.id.title_refresh_btn);
		mRefresh.setOnClickListener(this);

	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		switch (keyCode) {
		case KeyEvent.KEYCODE_BACK:
			if ((System.currentTimeMillis() - mExitTime) > 2000) {
				Toast.makeText(getApplicationContext(), "More once back key to exit", Toast.LENGTH_SHORT).show();
				mExitTime = System.currentTimeMillis();
			} else {
				finish();
			}
			break;

		default:
			return super.onKeyDown(keyCode, event);
		}
		return true;
	}

	public void onClick(View view) {
		switch (view.getId()) {
		case R.id.title_back_btn:
			new AlertDialog.Builder(this).setTitle("退出").setMessage("确定要退出？")
					.setPositiveButton("退出", new DialogInterface.OnClickListener() {

						public void onClick(DialogInterface dialog, int which) {
							finish();

						}
					}).setNegativeButton("取消", new DialogInterface.OnClickListener() {

						public void onClick(DialogInterface dialog, int which) {

						}
					}).create().show();
			break;

		case R.id.title_left_btn:
			Toast.makeText(this, "Left", Toast.LENGTH_SHORT).show();
			break;

		case R.id.title_right_btn:
			Toast.makeText(this, "Right", Toast.LENGTH_SHORT).show();
			break;

		case R.id.title_refresh_btn:
			mProgressDialog = new ProgressDialog(this);
			mProgressDialog.setTitle("刷新");
			mProgressDialog.setMessage("正在刷新，请稍候…");
			mProgressDialog.show();
			Handler handler = new Handler(this);
			handler.sendEmptyMessageDelayed(WHAT, 3 * 1000);
			break;

		default:
			break;
		}

	}

	public boolean handleMessage(Message msg) {
		if (msg.what == WHAT) {
			if (mProgressDialog != null) {
				mProgressDialog.dismiss();
				return true;
			}
		}
		else if (msg.what == TraderStatusListener.Trading){
			Toast toast = Toast.makeText(MyFragmentActivity.this, "开始交易。。",
					Toast.LENGTH_LONG);
			toast.setGravity(Gravity.CENTER, 0, 0);
			toast.show();
		}
		else if (msg.what == TraderStatusListener.NoTrading){
			String err_msg = (String)msg.obj;
			Toast toast = Toast.makeText(MyFragmentActivity.this, "开始交失败：" + err_msg,
					Toast.LENGTH_LONG);
			toast.setGravity(Gravity.CENTER, 0, 0);
			toast.show();	
			
		    Intent intent = new Intent(MyFragmentActivity.this, AccountActivity.class); 
            startActivity(intent);
		}
		return false;
	}
}
