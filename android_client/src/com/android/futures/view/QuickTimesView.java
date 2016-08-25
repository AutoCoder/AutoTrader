package com.android.futures.view;

import java.util.ArrayList;
import java.util.Locale;
import java.util.Vector;

import com.android.futures.entity.MDEntity;
import com.android.futures.entity.TechType;
import com.android.futures.entity.TradeEntity;
import com.android.futures.util.CircularMDQueue;
import com.android.futures.util.VisualizationSetting;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.PorterDuff.Mode;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.util.Log;

public class QuickTimesView extends SurfaceView implements SurfaceHolder.Callback {
	private final int DATA_MAX_COUNT = VisualizationSetting.TICKVIEW_DURATION * 60 * 2; // tick_max_count
	private final float Text_Size = VisualizationSetting.TEXT_XLARGE;
	private SurfaceHolder mHolder;
	private DrawThread mThread;
	CircularMDQueue mMDList = null;
	public Vector<TradeEntity> mTradeList = null;
	private String mInstrument = new String("");
	private String mStrategy = new String("");

	private float mMargin = 5;
	private float mTimeRectLeft;
	private float mTimeRectRight;
	private float mTimeRectTop;
	private float mTimeRectBottom;
	private float mTimeAxis;
	private double mRatioRange;
	private float mFontHeight;
	private float mVolumeRectBottom;
	private float mVolumeRectTop;
	private float mTimeSpacing;

	private double mHighPrice;
	private double mLowPrice;
	private double mhighestVolume;
	private double mLowestVolume;
	private double mPreSettlementPrice;
	private int m_beginIdx;
	private long m_dtimestamp;//unit: 0.5s

	public QuickTimesView(Context context) {
		super(context);
		// TODO Auto-generated constructor stub
		init();
	}

	public QuickTimesView(Context context, AttributeSet attrs) {
		super(context, attrs);
		init();
	}

	public QuickTimesView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		init();
	}

	public void setSequenceData(CircularMDQueue seqRef, Vector<TradeEntity> tradeseqRef, String instrument, String strategy) {
		mMDList = seqRef;
		mTradeList = tradeseqRef;
		mInstrument = instrument;
		mStrategy = strategy;
	}
	
	/**
	 * Return Ticks of current duration
	 * */
	private ArrayList<MDEntity> UpdateBoundary() {
		assert(mMDList.size() > 0);
		float viewWith = getWidth();
		float viewHeight = getHeight();

		ArrayList<MDEntity> CurrentTicks = mMDList.getLastElements(DATA_MAX_COUNT);
		if (CurrentTicks.isEmpty())
			return CurrentTicks;
		
		MDEntity first = CurrentTicks.get(0);
		mPreSettlementPrice = first.getPreSettlementPrice();
		m_dtimestamp = first.getTimeStamp();
		mHighPrice =  mPreSettlementPrice;
		mLowPrice = mPreSettlementPrice;
		mhighestVolume = first.getVol();
		mLowestVolume = first.getVol();
		
		for (int i = 0; i < CurrentTicks.size(); ++i) {
			
			MDEntity fenshiData = CurrentTicks.get(i);

			if (mHighPrice < fenshiData.getLastPrice())
				mHighPrice = fenshiData.getLastPrice();
			if (mLowPrice > fenshiData.getLastPrice()) 
				mLowPrice = fenshiData.getLastPrice();
			
			if (fenshiData.Techtype == TechType.MA){
				if (mHighPrice < fenshiData.TechMA.getLong_MA())
					mHighPrice = fenshiData.TechMA.getLong_MA();
				if (mHighPrice < fenshiData.TechMA.getShort_MA())
					mHighPrice = fenshiData.TechMA.getShort_MA();
				if (mLowPrice > fenshiData.TechMA.getLong_MA()) 
					mLowPrice = fenshiData.TechMA.getLong_MA();
				if (mLowPrice > fenshiData.TechMA.getShort_MA()) 
					mLowPrice = fenshiData.TechMA.getShort_MA();
			}
			
			if (mhighestVolume < fenshiData.getVol())
				mhighestVolume = fenshiData.getVol();
			else if (mLowestVolume > fenshiData.getVol())
				mLowestVolume = fenshiData.getVol();
		}
		
		//let the mTimeAxis centre vertically
		double absoffup = mHighPrice - mPreSettlementPrice;
		double absoffdown = mPreSettlementPrice - mLowPrice;
		if (absoffup > absoffdown){
			mLowPrice = mPreSettlementPrice - absoffup;
		}else{
			mHighPrice = mPreSettlementPrice + absoffdown;
		}
		
		Paint paint = new Paint();
		paint.setTextSize(Text_Size);
		String high = String.format(Locale.US, "%.1f", mHighPrice);//String.valueOf(mHighPrice);
		Rect textRect = new Rect();
		paint.getTextBounds(high, 0, 1, textRect);
		mFontHeight = textRect.height();
		float strWidth = paint.measureText(high);
		mTimeRectLeft = mMargin + strWidth;
		mTimeRectRight = viewWith - mTimeRectLeft;
		
		mTimeRectTop = 2* mMargin + mFontHeight;
		mTimeRectBottom = mTimeRectTop + viewHeight * 2 / 3;
		float ratio_axis = (float) ((mPreSettlementPrice - mLowPrice) / (float) (mHighPrice - mLowPrice));
		mTimeAxis = mTimeRectBottom - (mTimeRectBottom - mTimeRectTop) * ratio_axis;
		MDEntity last = CurrentTicks.get(CurrentTicks.size() - 1);
		mTimeSpacing = (mTimeRectRight - mTimeRectLeft) / DATA_MAX_COUNT; //(last.getTimeStamp() - m_dtimestamp);
		mVolumeRectBottom = viewHeight - mMargin;
		
		return CurrentTicks;
	}

	private void init() {
		mHolder = this.getHolder();
		mHolder.addCallback(this);
	}

	private class DrawThread extends Thread {
		public boolean isRunning = false;
		private static final int DRAW_INTERVAL = 500;

		public DrawThread() {
			isRunning = true;
		}

		public void stopThread() {
			isRunning = false;
			boolean workIsNotFinish = true;
			while (workIsNotFinish) {
				try {
					this.join();// 保证run方法执行完毕
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				workIsNotFinish = false;
			}
		}

		public void run() {
			long deltaTime = 0;
			long tickTime = 0;
			tickTime = System.currentTimeMillis();
			while (isRunning) {
				if (mMDList.size() == 0)
					continue;
				
				Canvas canvas = null;
				try {
					synchronized (mHolder) {
						canvas = mHolder.lockCanvas();
						if (canvas != null) {
							// clear
							Paint paint = new Paint();
							paint.setXfermode(new PorterDuffXfermode(Mode.CLEAR));
							canvas.drawPaint(paint);
							paint.setXfermode(new PorterDuffXfermode(Mode.SRC));

							
							ArrayList<MDEntity> CurrentTicks = UpdateBoundary();
							if (!CurrentTicks.isEmpty())
							{
								drawMDFrame(canvas, CurrentTicks);
								drawTicks(canvas, CurrentTicks);
								drawVolumes(canvas, CurrentTicks);
							}
						}
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					if (null != mHolder && canvas != null) {
						mHolder.unlockCanvasAndPost(canvas);
					}
				}

				deltaTime = System.currentTimeMillis() - tickTime;
				if (deltaTime < DRAW_INTERVAL) {
					try {
						Thread.sleep(DRAW_INTERVAL - deltaTime);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
				tickTime = System.currentTimeMillis();
			}
		}
	}

	private void drawMDFrame(Canvas canvas, ArrayList<MDEntity> CurrentTicks) {
		Paint paint = new Paint();
		paint.setAntiAlias(true);
		paint.setStyle(Style.STROKE);
		paint.setColor(Color.DKGRAY);
		paint.setTextSize(Text_Size);
		String high = String.format(Locale.US, "%.1f", mHighPrice); //String.(mHighPrice);
		String low = String.format(Locale.US, "%.1f", mLowPrice); //String.valueOf(mLowPrice);
		String preSettlement = String.format(Locale.US, "%.1f", mPreSettlementPrice); //String.valueOf(mPreSettlementPrice);
		
		MDEntity lastItem = CurrentTicks.get(CurrentTicks.size() - 1);
		paint.setColor(Color.WHITE);
		canvas.drawText(String.format(Locale.US, "%s %s Price:%5.0f Volume:%d", mInstrument, mStrategy, (float)lastItem.getLastPrice(), lastItem.getVol()), mTimeRectLeft, mMargin + mFontHeight , paint);
		paint.setColor(Color.DKGRAY);
		canvas.drawRect(mTimeRectLeft, mTimeRectTop, mTimeRectRight, mTimeRectBottom, paint);
		canvas.drawLine(mTimeRectLeft, mTimeAxis, mTimeRectRight, mTimeAxis, paint);
		mVolumeRectTop = mTimeRectBottom + 2 * mMargin + mFontHeight;
		canvas.drawRect(mTimeRectLeft, mVolumeRectTop, mTimeRectRight, mVolumeRectBottom, paint);

		mRatioRange = Math.max(Math.abs(mPreSettlementPrice - mHighPrice), Math.abs(mPreSettlementPrice - mLowPrice)) / mPreSettlementPrice * 100;
		String ratio = String.format(Locale.US, "%.1f", mRatioRange) + "%";
		float ratioWidth = paint.measureText(high);
		paint.setColor(Color.RED);
		canvas.drawText(high, 0, mTimeRectTop + mFontHeight + 1, paint);
		canvas.drawText(ratio, mTimeRectRight - ratioWidth, mTimeRectTop + mFontHeight + 1, paint);

		paint.setColor(Color.GREEN);
		canvas.drawText(low, 0, mTimeRectBottom - 1, paint);
		canvas.drawText(ratio, mTimeRectRight - ratioWidth, mTimeRectBottom - 1, paint);

		paint.setColor(Color.WHITE);
		//draw axis price
		canvas.drawText(preSettlement, 0, mTimeAxis + + mFontHeight/2, paint);
		//draw volume/TurnOver number
		canvas.drawText(String.format(Locale.US, "Volume:%d TurnOver:%9.0f", lastItem.getTotalVol(), lastItem.getTurnOver()), mTimeRectLeft, mVolumeRectTop - mMargin, paint);
		
		String highVol_str = String.valueOf(mhighestVolume);
		String lowVol_str = String.valueOf(mLowestVolume);
		canvas.drawText(highVol_str, 0, mVolumeRectTop + mFontHeight + 1, paint);
		canvas.drawText(lowVol_str, 0, mVolumeRectBottom - 1, paint);
	}

	private void drawVolumes(Canvas canvas, ArrayList<MDEntity> CurrentTicks) {
		float ratio;
		Paint paint = new Paint();

		paint.setAntiAlias(true);
		for (int i = 1; i < CurrentTicks.size(); i++) {
			MDEntity preData = CurrentTicks.get(i-1);
			MDEntity fenshiData = CurrentTicks.get(i);
			int timestamp_offset = (int) (fenshiData.getTimeStamp() - m_dtimestamp);
			
			ratio = (float) ((fenshiData.getVol() - mLowestVolume) / (mhighestVolume - mLowestVolume));
			float curY = mVolumeRectBottom - (mVolumeRectBottom - mVolumeRectTop) * ratio;
			float curX = mTimeRectLeft + mTimeSpacing * timestamp_offset;//(i - m_beginIdx);

			if (fenshiData.getLastPrice() > preData.getLastPrice()) {
				paint.setColor(Color.RED);
			} 
			else if(fenshiData.getLastPrice() < preData.getLastPrice()){
				paint.setColor(Color.GREEN);
			}
			else {
				paint.setColor(Color.GRAY);
			}
			canvas.drawLine(curX, mVolumeRectBottom, curX, curY, paint);
		}
	}

	private void drawTicks(Canvas canvas, ArrayList<MDEntity> CurrentTicks) {
		Paint paint = new Paint();
		paint.setAntiAlias(true);
		
		MDEntity first = CurrentTicks.get(0);
		boolean bDrawMA = !first.TechMA.IsEmpty() && first.Techtype == TechType.MA;
		double curY_short = mLowPrice;
		double curY_long = mLowPrice;
		
		float ratio = (float) ((first.getLastPrice() - mLowPrice) / (float) (mHighPrice - mLowPrice));
		float curY = mTimeRectBottom - (mTimeRectBottom - mTimeRectTop) * ratio;
		float curX = mTimeRectLeft;
		
		if (bDrawMA){
			double ratio_short = ((first.TechMA.getShort_MA() - mLowPrice) /  (mHighPrice - mLowPrice));
			double ratio_long = ((first.TechMA.getLong_MA() - mLowPrice) / (mHighPrice - mLowPrice));
			curY_short = mTimeRectBottom - (mTimeRectBottom - mTimeRectTop) * ratio_short;
			curY_long = mTimeRectBottom - (mTimeRectBottom - mTimeRectTop) * ratio_long;			
		}
		
		//draw tick
		for (int i = 1; i < CurrentTicks.size(); i++) {
			MDEntity fenshiData = CurrentTicks.get(i);
			int timestamp_offset = (int) (fenshiData.getTimeStamp() - m_dtimestamp);
			
			ratio = (float) (((float) (fenshiData.getLastPrice() - mLowPrice)) / (mHighPrice - mLowPrice));
			float nextY = mTimeRectBottom - (mTimeRectBottom - mTimeRectTop) * ratio;
			float nextX = mTimeRectLeft + mTimeSpacing * timestamp_offset;//(i - m_beginIdx);
			paint.setColor(Color.WHITE);
			canvas.drawLine(curX, curY, nextX, nextY, paint);
			curY = nextY;
			
			//draw tech (MA-Short-Long)
			if (bDrawMA){
				float ratio_short = (float) (((float) (fenshiData.TechMA.getShort_MA() - mLowPrice)) / (mHighPrice - mLowPrice));
				float ratio_long = (float) (((float) (fenshiData.TechMA.getLong_MA() - mLowPrice)) / (mHighPrice - mLowPrice));
				float nextY_short = mTimeRectBottom - (mTimeRectBottom - mTimeRectTop) * ratio_short;
				float nextY_long = mTimeRectBottom - (mTimeRectBottom - mTimeRectTop) * ratio_long;
				paint.setColor(Color.YELLOW);
				canvas.drawLine(curX, (float) curY_short, nextX, nextY_short, paint);
				paint.setColor(Color.GREEN);
				canvas.drawLine(curX, (float) curY_long, nextX, nextY_long, paint);	
				curY_short = nextY_short;
				curY_long = nextY_long;
			}
			
			curX = nextX;
		}
		
		//draw trade signal
		if (mTradeList.size() != 0){
			final int size = mTradeList.size();
			int idx = 0;
			while (idx < size){
				TradeEntity item = mTradeList.get(idx);
				long timestamp_offset = item.getTimeStamp() - m_dtimestamp;
				if (timestamp_offset < 0){
//					StringBuilder msg = new StringBuilder("m_dtimestamp:");
//					msg.append(m_dtimestamp).append(" item timestamp: ").append(item.getTimeStamp());
//					Log.d("Exception", msg.toString());
					break;
				}
//				else{
//					StringBuilder msg = new StringBuilder("m_dtimestamp:");
//					msg.append(m_dtimestamp).append(" item timestamp: ").append(item.getTimeStamp());
//					Log.d("Find", msg.toString());
//				}

					
				float x_pos = mTimeRectLeft + mTimeSpacing * timestamp_offset;
				ratio = (float) ((item.getLastPrice() - mLowPrice)/(mHighPrice - mLowPrice));
				float y_pos = mTimeRectBottom - (mTimeRectBottom - mTimeRectTop) * ratio;
				
				if (item.getDirection() == 0)//Buy:0 Sell:1
					paint.setColor(Color.RED);
				else
					paint.setColor(Color.GREEN);
				
				float radius = mFontHeight / 3;
				
				if (item.getType() == TradeEntity.type.Insert_Order){
					paint.setStyle(Paint.Style.STROKE);
					canvas.drawCircle(x_pos, y_pos, radius, paint);
				}
				else if (item.getType() == TradeEntity.type.Cancell_Order){
					paint.setStyle(Paint.Style.FILL);
					paint.setColor(Color.GRAY);
					canvas.drawCircle(x_pos, y_pos, radius, paint);
				}else if (item.getType() == TradeEntity.type.Trade){
					paint.setStyle(Paint.Style.FILL);
					canvas.drawCircle(x_pos, y_pos, radius, paint);
				}	
				else if (item.getType() == TradeEntity.type.Order_Triggered){
					paint.setStyle(Paint.Style.STROKE);
					canvas.drawCircle(x_pos, y_pos, radius/2, paint);
					canvas.drawCircle(x_pos, y_pos, radius, paint);
				}else
				{
					assert(false);
				}
				idx++;
			}
		}
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		if (mMDList != null) {
			mThread = new DrawThread();
			mThread.start();
		}
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		// TODO Auto-generated method stub

	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		mThread.stopThread();
		mMDList = null;
		mTradeList = null;
	}
}