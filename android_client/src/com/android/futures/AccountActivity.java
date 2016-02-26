package com.android.futures;

import android.annotation.SuppressLint;
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
import android.widget.TextView;

import java.util.ArrayList;

//import android.view.Menu;
//import android.view.MenuItem;
import com.android.futures.MyApp;
import com.android.futures.tcp.AccountInfo;
import com.android.futures.tcp.PositionInfo;
import com.android.futures.tcp.ClientSession;
import com.android.futures.tcp.TraderStatusListener;

public class AccountActivity extends Activity implements Handler.Callback {
	private Handler mHandler = null;
	private ClientSession mSession = null;
	private Spinner mInstrumentList = null;
	private Spinner mStrategyList = null;
	private Button tradeBtn = null;
	private Button monitorBtn = null;
	private Button logOutBtn = null;
	private TextView balanceView = null;
	private TextView positionView = null;
	private boolean mIsTrading = false;

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
		balanceView = (TextView) this.findViewById(R.id.balance_val);
		positionView = (TextView) this.findViewById(R.id.position_val);
		mInstrumentList = (Spinner) this.findViewById(R.id.instrument_List);
		mStrategyList = (Spinner) this.findViewById(R.id.strategy_list);
		tradeBtn = (Button) this.findViewById(R.id.trade_btn);
		monitorBtn = (Button) this.findViewById(R.id.monitor_btn);
		logOutBtn = (Button) this.findViewById(R.id.LogOut);
		
		MyApp app = (MyApp) getApplication();
		mSession = app.GetSession();
		mSession.SetHandler(mHandler);
		mSession.Login();
		
		updateTradeStatus();
		
		logOutBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				mSession.LogOut();
//			    Intent intent = new Intent(AccountActivity.this, LoginActivity.class);  
//                startActivity(intent);
                finish();
			}
		});
		
		monitorBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
			    Intent intent = new Intent(AccountActivity.this, MyFragmentActivity.class); 
			    intent.putExtra("instrument", (String) mInstrumentList.getSelectedItem());
			    intent.putExtra("strategy", (String) mStrategyList.getSelectedItem());
	            startActivity(intent);
			}
		});
		
		tradeBtn.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				if (mIsTrading){
					mSession.StopTrade();
				}else{
					mSession.StartTrade((String) mStrategyList.getSelectedItem(), (String) mInstrumentList.getSelectedItem());
				}
			}
		});
	}

	private void updateTradeStatus(){
		monitorBtn.setEnabled(mIsTrading);
		if (mIsTrading){
			tradeBtn.setText(R.string.StopTrade);
		}else{
			tradeBtn.setText(R.string.Trade);
		}
	}
	
	@Override
	protected void onRestart(){
		super.onRestart();
		mSession.QueryPosition();
	}

	@Override
	public boolean handleMessage(Message msg) {
		if (msg.what == TraderStatusListener.PositionUpdated) {
			PositionInfo status = (PositionInfo) msg.obj;
			balanceView.setText(Double.toString(status.getBalance()));
			String pos_text = String.format("[%s]: (%d * %d)", status.getInstrument(), status.getPrice(), status.getPosition());
			positionView.setText(pos_text);
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
			mIsTrading = info.getIsTrading();
			updateTradeStatus();
		}
		else if (msg.what == TraderStatusListener.Trading){
			mIsTrading = true;
			updateTradeStatus();
		}
		else if (msg.what == TraderStatusListener.NoTrading){
			mIsTrading = false;
			updateTradeStatus();
		}
		return false;
	}
}
