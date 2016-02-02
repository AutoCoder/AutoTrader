package com.android.futures;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.widget.Button;
import android.view.View;
import android.widget.EditText; 
import android.app.Application;
import com.android.futures.tcp.ClientSession;;

public class WelcomeActivity extends Activity implements Handler.Callback {

	private final int WHAT = 1987;
	private final long DELAY_TIME = 1500;
	private Button loginBtn;
	private Button cancelBtn;
	private EditText accountEdit;
	private EditText pwdEdit;
	private EditText brokerIdEdit;
	private Handler handler = new Handler(android.os.Message msg){
		
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_welcome);
		handler = new Handler(this);
		loginBtn = (Button) this.findViewById(R.id.login_in);
		cancelBtn = (Button) this.findViewById(R.id.cancel);
		accountEdit = (EditText) this.findViewById(R.id.accountEdittext);
		pwdEdit = (EditText) this.findViewById(R.id.pwdEdittext);
		brokerIdEdit = (EditText) this.findViewById(R.id.brokerIdtext);
		loginBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				//todo: send login info
				ClientSession session = (ClientSession) getApplication();
				session.Login(brokerIdEdit.toString(),accountEdit.toString(),pwdEdit.toString());
			}
		});
	}

	@Override
	protected void onResume() {
		super.onResume();
//		new Thread(new Runnable() {
//
//			public void run() {
//				handler.sendEmptyMessageDelayed(WHAT, DELAY_TIME);
//
//			}
//		}).start();
	}

	@Override
	protected void onPause() {
		super.onPause();
		handler.removeMessages(WHAT);
	}

	public boolean handleMessage(Message msg) {

		if (msg.what == WHAT) {
			Intent intent = new Intent(WelcomeActivity.this, MyFragmentActivity.class);
			startActivity(intent);
			finish();
			return true;
		}
		return false;

	}
}
