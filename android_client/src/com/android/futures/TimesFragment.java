package com.android.futures;


import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.android.futures.tcp.ClientSession;
import com.android.futures.view.QuickTimesView;

public class TimesFragment extends Fragment {
	//private TimesView mTimesView;
	private QuickTimesView mTimesView;
	private ClientSession mSession;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View view = inflater.inflate(R.layout.fragment_times, null);
		mTimesView = (QuickTimesView) view.findViewById(R.id.my_fenshi_view);
		mTimesView.setTimeSequence(mSession.mMdSequence, mSession.getCurrentInstrument(), mSession.getStrategyName());
		return view;
	}
	
	@Override
	public void onAttach(Activity activity){
        super.onAttach(activity);
        MyFragmentActivity my_activity = (MyFragmentActivity)activity;
        MyApp app = (MyApp) my_activity.getApplication();
        mSession = app.GetSession();
	}
}
