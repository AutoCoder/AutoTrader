package com.android.futures.view;

import java.util.Vector;

import com.android.futures.entity.MDEntity;
import com.android.futures.entity.TechType;
import com.android.futures.entity.TradeEntity;
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

public class QuickTimesView extends SurfaceView implements SurfaceHolder.Callback {
	private final int DATA_MAX_COUNT = 4 * 60;
	private final float Text_Size = VisualizationSetting.TEXT_XLARGE;
	private SurfaceHolder mHolder;
	private DrawThread mThread;
	public Vector<MDEntity> mMDList = null;
	public Vector<TradeEntity> mTradeList = null;
	private String mInstrument = new String("");
	private String mStrategy = new String("");

	private float mMargin = 5;
	private float mTimeRectLeft;
	private float mTimeRectRight;
	private float mTimeRectTop;
	private float mTimeRectBottom;
	private double mRatioRange;
	private float mFontHeight;
	private float mVolumeRectBottom;
	private float mVolumeRectTop;
	private float mTimeSpacing;

	private double mHighPrice;
	private double mLowPrice;
	private double mhighestVolume;
	private double mLowestVolume;
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

	public void setSequenceData(Vector<MDEntity> seqRef, Vector<TradeEntity> tradeseqRef, String instrument, String strategy) {
		mMDList = seqRef;
		mTradeList = tradeseqRef;
		mInstrument = instrument;
		mStrategy = strategy;
	}

	private void UpdateBoundary() {
		float viewWith = getWidth();
		float viewHeight = getHeight();

		m_beginIdx = mMDList.size() > DATA_MAX_COUNT ? (mMDList.size() - DATA_MAX_COUNT) : 0;
		m_dtimestamp = mMDList.get(m_beginIdx).getTimeStamp();
		mHighPrice = mMDList.size() > 0 ? mMDList.get(m_beginIdx).getLastPrice() : 0.0;
		mLowPrice = mMDList.size() > 0 ? mMDList.get(m_beginIdx).getLastPrice() : 1000000000.0;
		mhighestVolume = mMDList.size() > 0 ? mMDList.get(m_beginIdx).getVol() : 0.0;
		mLowestVolume = mMDList.size() > 0 ? mMDList.get(m_beginIdx).getVol() : 0.0;

		for (int i = m_beginIdx; i < m_beginIdx + DATA_MAX_COUNT && i < mMDList.size(); ++i) {
			
			MDEntity fenshiData = mMDList.get(i);

			if (mHighPrice < fenshiData.getLastPrice())
				mHighPrice = fenshiData.getLastPrice();
			if (mLowPrice > fenshiData.getLastPrice()) 
				mLowPrice = fenshiData.getLastPrice();

			if (mhighestVolume < fenshiData.getVol())
				mhighestVolume = fenshiData.getVol();
			else if (mLowestVolume > fenshiData.getVol())
				mLowestVolume = fenshiData.getVol();
		}
		
		//Enlarge Price Range a little bit
		mHighPrice += 3;
		mLowPrice -= 3;
		
		Paint paint = new Paint();
		paint.setTextSize(Text_Size);
		String high = String.valueOf(mHighPrice);
		Rect textRect = new Rect();
		paint.getTextBounds(high, 0, 1, textRect);
		mFontHeight = textRect.height();
		float strWidth = paint.measureText(high);
		mTimeRectLeft = mMargin + strWidth;
		mTimeRectRight = viewWith - mTimeRectLeft - mMargin;
		
		mTimeRectTop = 2* mMargin + mFontHeight;
		mTimeRectBottom = mTimeRectTop + viewHeight * 2 / 3;
		mTimeSpacing = (mTimeRectRight - mTimeRectLeft) / DATA_MAX_COUNT;
		mVolumeRectBottom = viewHeight - mMargin;
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
				if (mMDList.isEmpty())
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

							UpdateBoundary();
							drawMDFrame(canvas);
							drawTicks(canvas);
							drawVolumes(canvas);
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

	private void drawMDFrame(Canvas canvas) {
		Paint paint = new Paint();
		paint.setAntiAlias(true);
		paint.setStyle(Style.STROKE);
		paint.setColor(Color.DKGRAY);
		paint.setTextSize(Text_Size);
		String high = String.valueOf(mHighPrice);
		String low = String.valueOf(mLowPrice);
		
		MDEntity lastItem = mMDList.lastElement();
		paint.setColor(Color.WHITE);
		canvas.drawText(String.format("%s %s Price:%5.0f Volume:%d", mInstrument, mStrategy, (float)lastItem.getLastPrice(), lastItem.getVol()), mTimeRectLeft, mMargin + mFontHeight , paint);
		paint.setColor(Color.DKGRAY);
		canvas.drawRect(mTimeRectLeft, mTimeRectTop, mTimeRectRight, mTimeRectBottom, paint);
		mVolumeRectTop = mTimeRectBottom + 2 * mMargin + mFontHeight;
		canvas.drawRect(mTimeRectLeft, mVolumeRectTop, mTimeRectRight, mVolumeRectBottom, paint);

		mRatioRange = (mHighPrice - mLowPrice) / 200.0;
		String ratio = mRatioRange + "%";
		float ratioWidth = paint.measureText(high);
		paint.setColor(Color.RED);
		canvas.drawText(high, 0, mTimeRectTop + mFontHeight + 1, paint);
		canvas.drawText(ratio, mTimeRectRight - ratioWidth, mTimeRectTop + mFontHeight + 1, paint);

		paint.setColor(Color.GREEN);
		canvas.drawText(low, 0, mTimeRectBottom - 1, paint);
		canvas.drawText(ratio, mTimeRectRight - ratioWidth, mTimeRectBottom - 1, paint);

		paint.setColor(Color.WHITE);
		String volumeTitle = "量:1215  现手:1215  额:163.4万";
		canvas.drawText(volumeTitle, mTimeRectLeft, mVolumeRectTop - mMargin, paint);
		
		String highVol_str = String.valueOf(mhighestVolume);
		String lowVol_str = String.valueOf(mLowestVolume);
		canvas.drawText(highVol_str, 0, mVolumeRectTop + mFontHeight + 1, paint);
		canvas.drawText(lowVol_str, 0, mVolumeRectBottom - 1, paint);
	}

	private void drawVolumes(Canvas canvas) {
		float ratio;
		Paint paint = new Paint();

		paint.setAntiAlias(true);
		for (int i = m_beginIdx; i < m_beginIdx + DATA_MAX_COUNT && i < mMDList.size(); i++) {
			MDEntity preData = i > 1 ? mMDList.get(i-1) : mMDList.get(0);
			MDEntity fenshiData = mMDList.get(i);
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

	private void drawTicks(Canvas canvas) {
		Paint paint = new Paint();
		paint.setAntiAlias(true);
		
		MDEntity first = mMDList.get(m_beginIdx);
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
		for (int i = m_beginIdx + 1; i < m_beginIdx + DATA_MAX_COUNT && i < mMDList.size(); i++) {
			MDEntity fenshiData = mMDList.get(i);
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
			int lastIdx = mTradeList.size() - 1;
			while (lastIdx >= 0){
				TradeEntity item = mTradeList.get(lastIdx);
				long timestamp_offset = item.getTimeStamp() - m_dtimestamp;
				if (timestamp_offset > 0){
					float x_pos = mTimeRectLeft + mTimeSpacing * timestamp_offset;
					ratio = (float) ((item.getLastPrice() - mLowPrice)/(mHighPrice - mLowPrice));
					float y_pos = mTimeRectBottom - (mTimeRectBottom - mTimeRectTop) * ratio;
					
					if (item.getDirection() == 0)
						paint.setColor(Color.GREEN);
					else
						paint.setColor(Color.RED);
					
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
					lastIdx--;
				}else{
					break;
				}
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
	}
}