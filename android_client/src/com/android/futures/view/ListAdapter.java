package com.android.futures.view;

import java.util.Vector;

import com.android.futures.entity.TradeEntity;
import com.android.futures.R;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.TextView;

public class ListAdapter extends BaseAdapter implements Filterable{
	private Vector<TradeEntity> list = null;
	private Vector<TradeEntity> filteredData = null;
	private Context c;
	private TradeTypeFilter filter = null;
	private boolean include_trade = true;
	private boolean include_insertOrder = true;
	private boolean include_cancelOrder = true;
	
	public ListAdapter(Vector<TradeEntity> list, Context c) {
		super();
		this.list = list;
		this.filteredData = list;
		this.c = c;
	}
	
	public void setFilter(boolean trade, boolean insertOrder, boolean cancelOrder){
		include_trade = trade;
		include_insertOrder = insertOrder;
		include_cancelOrder = cancelOrder;
	}
	
	@Override
	public int getCount() {
		// TODO Auto-generated method stub
		return filteredData.size();
	}

	@Override
	public Object getItem(int position) {
		return filteredData.get(position);
	}

	@Override
	public long getItemId(int position) {
		// TODO Auto-generated method stub
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		ViewHolder holder = null;
		if (convertView == null) {
			holder = new ViewHolder();
			convertView = LayoutInflater.from(c).inflate(R.layout.trade_listview_item,
					null);
			holder.InstrumentIdLabel = (TextView) convertView.findViewById(R.id.instrument_id);
			holder.directionLabel = (TextView) convertView.findViewById(R.id.direction);
			holder.priceLabel = (TextView) convertView.findViewById(R.id.last_price);
			holder.volumeLabel = (TextView) convertView.findViewById(R.id.volume);
			holder.orderRefLabel = (TextView) convertView.findViewById(R.id.order_ref);
			holder.occur_time = (TextView) convertView.findViewById(R.id.occur_time);
			convertView.setTag(holder);
		}else{
			holder=(ViewHolder) convertView.getTag();
		}
		
		holder.InstrumentIdLabel.setText("rb1605");//list.get(position).
		holder.directionLabel.setText(list.get(position).getDirection());
		holder.priceLabel.setText(String.valueOf(list.get(position).getLastPrice()));
		holder.volumeLabel.setText(list.get(position).getVol());
		holder.orderRefLabel.setText(list.get(position).getOrderId());
		holder.occur_time.setText(String.valueOf(list.get(position).getTimeStamp()));
		
		return convertView;
	}
	
	static class ViewHolder {
		TextView InstrumentIdLabel;
		TextView directionLabel;
		TextView priceLabel;
		TextView volumeLabel;
		TextView orderRefLabel;
		TextView occur_time;
	}

	@Override
	public Filter getFilter() {
		// TODO Auto-generated method stub
		if (filter == null)
			filter = new TradeTypeFilter();
		return filter;
	}
	
	private class TradeTypeFilter extends Filter {
        @Override
        protected FilterResults performFiltering(CharSequence constraint) {

            FilterResults results = new FilterResults();
            
            for (int i =0; i < list.size(); i++){
            	if (include_trade && list.get(i).getType() == TradeEntity.type.Trade){
            		filteredData.add(list.get(i));
            	}
            	if (include_insertOrder && list.get(i).getType() == TradeEntity.type.Insert_Order){
            		filteredData.add(list.get(i));
            	}
            	if(include_cancelOrder && list.get(i).getType() == TradeEntity.type.Cancell_Order){
            		filteredData.add(list.get(i));
            	}
            }
            
            results.values = filteredData;
            results.count = filteredData.size();

            return results;
        }

        @SuppressWarnings("unchecked")
        @Override
        protected void publishResults(CharSequence constraint, FilterResults results) {
            filteredData = (Vector<TradeEntity>) results.values;
            notifyDataSetChanged();
        }

    }
}
