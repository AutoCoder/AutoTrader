package com.android.futures;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.view.Gravity;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import java.io.IOException;
import com.android.futures.MyApp;
import com.android.futures.tcp.ClientSession;
import com.android.futures.util.SubThreadException;;

public class LoginActivity extends Activity{
	
	private Button loginBtn;
	private Button cancelBtn;
	private EditText accountEdit;
	private EditText pwdEdit;
	private EditText brokerIdEdit;
	private EditText hostEdit;
	private EditText portEdit;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_login);
		loginBtn = (Button) this.findViewById(R.id.login_in);
		cancelBtn = (Button) this.findViewById(R.id.cancel);
		accountEdit = (EditText) this.findViewById(R.id.accountEdittext);
		pwdEdit = (EditText) this.findViewById(R.id.pwdEdittext);
		brokerIdEdit = (EditText) this.findViewById(R.id.brokerIdtext);
		hostEdit = (EditText) this.findViewById(R.id.host_text);
		portEdit = (EditText) this.findViewById(R.id.port_text);
		loginBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				
				MyApp app = (MyApp) getApplication();
				ClientSession session = app.GetSession();
				int port = Integer.parseInt(portEdit.getText().toString());
				try{
					session.ConnectServer(brokerIdEdit.getText().toString(),accountEdit.getText().toString(),pwdEdit.getText().toString(), hostEdit.getText().toString(), port);
				    Intent intent = new Intent(LoginActivity.this,  
				    		AccountActivity.class);  
	                startActivity(intent); 	
				}
				catch (SubThreadException e){
					Toast toast = Toast.makeText(LoginActivity.this, "Can't connect to AutoTrader server... Reason:" + e.getMessage(),
							Toast.LENGTH_LONG);
					toast.setGravity(Gravity.CENTER, 0, 0);
					toast.show();
				}				
				catch (IOException e){
					e.printStackTrace();
				}
			}
		});
		
		cancelBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				finish(); 
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
	}

}
