package com.android.futures;

import java.util.Vector;

import com.android.futures.entity.TradeEntity;
import com.android.futures.tcp.ClientSession;
import com.android.futures.view.ListAdapter;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.ListView;

public class TradeListActivity extends Activity {
	private ListView listview;
	private ListAdapter adapter;
	private ClientSession mSession = null;
	private CheckBox includeTrade = null;
	private CheckBox includeInsertOrder = null;
	private CheckBox includeCancelOrder = null;
	
//	private Vector<TradeEntity> AddFakeData(){
//		Vector<TradeEntity> tradeSequence = new Vector<TradeEntity>();
//		TradeEntity ent = new TradeEntity(TradeEntity.type.Insert_Order, 0, 2155, 10, "1", 1457670173);
//		TradeEntity ent1 = new TradeEntity(TradeEntity.type.Cancell_Order, 0, 2155, 10, "1", 1457670173);
//		TradeEntity ent2 = new TradeEntity(TradeEntity.type.Trade, 0, 2155, 10, "1", 1457670173);
//		tradeSequence.add(ent);
//		tradeSequence.add(ent1);
//		tradeSequence.add(ent2);
//		return tradeSequence;
//	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		try {
			super.onCreate(savedInstanceState);
			setContentView(R.layout.activity_listview);
			includeTrade = (CheckBox)this.findViewById(R.id.trade_check);
			includeInsertOrder = (CheckBox)this.findViewById(R.id.insertOrder_check);
			includeCancelOrder = (CheckBox)this.findViewById(R.id.cancelOrder_check);
			
			MyApp app = (MyApp) getApplication();
			mSession = app.GetSession();
			listview = (ListView) findViewById(R.id.listview);
			adapter = new ListAdapter(mSession.mTradeSequence, TradeListActivity.this);
			//adapter = new ListAdapter(AddFakeData(), TradeListActivity.this);
			
		} catch (Exception e) {
			Log.e("ERROR", "ERROR IN CODE: " + e.toString());
			e.printStackTrace();
		}
		
		includeTrade.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener(){ 
            @Override
            public void onCheckedChanged(CompoundButton buttonView, 
                    boolean isChecked) { 
            	validateFilter();
            } 
        }); 
		
		includeInsertOrder.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener(){ 
            @Override
            public void onCheckedChanged(CompoundButton buttonView, 
                    boolean isChecked) { 
            	validateFilter();
            } 
        }); 
		
		includeCancelOrder.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener(){ 
            @Override
            public void onCheckedChanged(CompoundButton buttonView, 
                    boolean isChecked) { 
            	validateFilter();
            	
            } 
        }); 
		
		listview.setAdapter(adapter);
		listview.setTextFilterEnabled(true);  
	}
	
	private void validateFilter(){
		adapter.setFilter(includeTrade.isChecked(), includeInsertOrder.isChecked(), includeCancelOrder.isChecked());
		if (adapter instanceof Filterable) {  
			Filter filter = ((Filterable) adapter).getFilter();
			filter.filter(null);  
		}
		//adapter.notifyDataSetChanged();
	}
}
