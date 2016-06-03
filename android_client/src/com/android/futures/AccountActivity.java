package com.android.futures;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.Context;
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

import com.android.futures.MyApp;
import com.android.futures.entity.TradeEntity;
import com.android.futures.tcp.AccountInfo;
import com.android.futures.tcp.PositionInfo;
import com.squareup.leakcanary.RefWatcher;
import com.android.futures.tcp.ClientSession;
import com.android.futures.tcp.ClientStatusListener;

public class AccountActivity extends Activity implements Handler.Callback {
	private Handler mHandler = null;
	private ClientSession mSession = null;
	private Spinner mInstrumentList = null;
	private Spinner mStrategyList = null;
	private Button tradeBtn = null;
	private Button monitorBtn = null;
	private Button logOutBtn = null;
	private Button closeCtpBtn = null;
	private Button checkMsgBtn = null;
	private TextView accountView = null;
	private TextView balanceView = null;
	private TextView positionView = null;
	private boolean IsTrading = false;
	private boolean IsPositionUpdated = false;
	private ProgressDialog progressDlg;
	private int notificationID = 0;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		try {
			super.onCreate(savedInstanceState);
			setContentView(R.layout.activity_account);
			
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
		mHandler = new Handler(this);
		accountView = (TextView) this.findViewById(R.id.account_val);
		balanceView = (TextView) this.findViewById(R.id.balance_val);
		positionView = (TextView) this.findViewById(R.id.position_val);
		mInstrumentList = (Spinner) this.findViewById(R.id.instrument_List);
		mStrategyList = (Spinner) this.findViewById(R.id.strategy_list);
		tradeBtn = (Button) this.findViewById(R.id.trade_btn);
		monitorBtn = (Button) this.findViewById(R.id.monitor_btn);
		logOutBtn = (Button) this.findViewById(R.id.LogOut);
		closeCtpBtn = (Button) this.findViewById(R.id.ReleaseCtp);
		checkMsgBtn = (Button) this.findViewById(R.id.CheckMessage);
		
		Intent intent =getIntent();
		if (intent.hasExtra("AccountId")){
			String account_str = intent.getStringExtra("AccountId");
			accountView.setText(account_str.toCharArray(), 0, account_str.length());
		}
		
		MyApp app = (MyApp) getApplication();		
		mSession = app.GetSession();
		mSession.SetHandler(mHandler);
		mSession.Login();
		updateButtonStatus();
		
		logOutBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				finish();
			}
		});
		
		closeCtpBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				mSession.LogOut();
			}
		});
		
		checkMsgBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				Intent intent = new Intent(AccountActivity.this, TradeListActivity.class);
				startActivity(intent);
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
				if (IsTrading){
					mSession.StopTrade();
				}else{
					mSession.StartTrade((String) mStrategyList.getSelectedItem(), (String) mInstrumentList.getSelectedItem());
				}
			}
		});
		
		progressDlg = new ProgressDialog(this);
		progressDlg.setTitle("提示");
		progressDlg.setMessage("登陆中。。。");
		//progressDlg.setCancelable(false);
		progressDlg.show();
	}

	private void updateButtonStatus(){
		monitorBtn.setEnabled(IsTrading);
		tradeBtn.setEnabled(IsPositionUpdated);
		if (IsTrading){
			tradeBtn.setText(R.string.StopTrade);
		}else{
			tradeBtn.setText(R.string.Trade);
		}
	}
	
	@Override
	protected void onRestart(){
		super.onRestart();
		mSession.SetHandler(mHandler);
		mSession.QueryPosition();
	}

	@Override
	public boolean handleMessage(Message msg) {
		if (msg.what == ClientStatusListener.Logined){
			progressDlg.setMessage("登陆成功，账户初始化中。。。");
			
		} else if (msg.what == ClientStatusListener.LoginFailed){
			String err_str = (String) msg.obj;
			progressDlg.setMessage("登陆失败，Reason:" + err_str);
			try {
				Thread.sleep(1000, 0);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			progressDlg.dismiss();
			finish();
			
		} else if (msg.what == ClientStatusListener.LogOut){
			finish();
		} else if (msg.what == ClientStatusListener.PositionUpdated) {
			PositionInfo status = (PositionInfo) msg.obj;
			balanceView.setText(Double.toString(status.getBalance()));
			//String pos_text = String.format("[%s]: (%d * %d)", status.getInstrument(), status.getPrice(), status.getPosition());
			positionView.setText(status.getDetails());
			IsPositionUpdated = true;
			progressDlg.setMessage("账户初始化完毕。");
			try {
				Thread.sleep(1000, 0);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			progressDlg.dismiss();
			updateButtonStatus();
			
		} else if (msg.what == ClientStatusListener.AccountInfoUpdated) {
			AccountInfo info = (AccountInfo) msg.obj;
			ArrayList<String> instrus = info.getInstrumentList();
			ArrayAdapter<String> adapter1 = new ArrayAdapter<String>(AccountActivity.this,
					android.R.layout.simple_spinner_item, instrus);
			mInstrumentList.setAdapter(adapter1);

			ArrayList<String> sts = info.getStrategies();
			ArrayAdapter<String> adapter2 = new ArrayAdapter<String>(AccountActivity.this,
					android.R.layout.simple_spinner_item, sts);
			mStrategyList.setAdapter(adapter2);
			IsTrading = info.getIsTrading();
			mSession.setInstrument(info.getRunningInstrument());
			mSession.setStrategyName(info.getRunningStrategy());
			updateButtonStatus();
			progressDlg.setMessage("登陆成功，账户初始化中。。。");
		}
		else if (msg.what == ClientStatusListener.Trading){
			IsTrading = true;
			updateButtonStatus();
		}
		else if (msg.what == ClientStatusListener.NoTrading){
			IsTrading = false;
			updateButtonStatus();
		}
		else if (msg.what == ClientStatusListener.TradeNotification){
			sendTradeNotification(msg);  
		}
		return false;
	}

	private void sendTradeNotification(Message msg) {
		TradeEntity tradeEntity = (TradeEntity)msg.obj;

		String title = String.format("%s Price:%5.0f Volume:%d", tradeEntity.getDirectionString(), tradeEntity.getLastPrice(), tradeEntity.getVol());
		String content = String.format("Order_Ref:%s Trade_Time:%s", tradeEntity.getOrderId(), tradeEntity.getOccurTimeString());
		PendingIntent pendingIntent = PendingIntent.getActivity(this, notificationID++,  
		        new Intent(this, TradeListActivity.class), 0);
		
		NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE); 
		final int NOTIFICATION_FLAG = 1;  
		Notification notify = new Notification.Builder(this)  
		    .setSmallIcon(R.drawable.messages) 
		    .setTicker(tradeEntity.getTypeString() + "提醒！")                
		    .setContentTitle(title)                               
		    .setContentText(content)
		    .setContentIntent(pendingIntent)
		    .setNumber(1)
		    .getNotification();
		
		notify.defaults |= Notification.DEFAULT_SOUND;  
		notify.defaults |= Notification.DEFAULT_VIBRATE;  
		notify.flags |= Notification.FLAG_AUTO_CANCEL;  
		manager.notify(NOTIFICATION_FLAG, notify);
	}
}
