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

public class ListAdapter extends BaseAdapter implements Filterable {
	private Vector<TradeEntity> list;
	private Vector<TradeEntity> filteredData;
	private Context c;
	private TradeTypeFilter filter = null;
	private boolean include_trade = true;
	private boolean include_insertOrder = true;
	private boolean include_cancelOrder = true;

	@SuppressWarnings("unchecked")
	public ListAdapter(Vector<TradeEntity> list, Context c) {
		super();
		this.list = list;
		this.filteredData = (Vector<TradeEntity>) list.clone();
		this.c = c;
	}

	public void setFilter(boolean trade, boolean insertOrder, boolean cancelOrder) {
		include_trade = trade;
		include_insertOrder = insertOrder;
		include_cancelOrder = cancelOrder;
	}

	@Override
	public int getCount() {
		// TODO Auto-generated method stub
		if (filteredData == null) {
			return 0;
		} else {
			return (filteredData.size());
		}
	}

	@Override
	public Object getItem(int position) {
		if (filteredData == null) {
			return null;
		} else {
			return filteredData.get(position);
		}
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
			convertView = LayoutInflater.from(c).inflate(R.layout.trade_listview_item, null);
			holder.TradeType = (TextView) convertView.findViewById(R.id.entity_type);
			holder.InstrumentIdLabel = (TextView) convertView.findViewById(R.id.instrument_id);
			holder.directionLabel = (TextView) convertView.findViewById(R.id.direction);
			holder.offsetLabel = (TextView) convertView.findViewById(R.id.offsetFlag);
			holder.priceLabel = (TextView) convertView.findViewById(R.id.last_price);
			holder.volumeLabel = (TextView) convertView.findViewById(R.id.volume);
			holder.orderRefLabel = (TextView) convertView.findViewById(R.id.order_ref);
			holder.occur_time = (TextView) convertView.findViewById(R.id.occur_time);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}

		holder.TradeType.setText(filteredData.get(position).getTypeString());
		holder.InstrumentIdLabel.setText(filteredData.get(position).getInstrument());
		holder.directionLabel.setText(filteredData.get(position).getDirectionString());
		holder.offsetLabel.setText(filteredData.get(position).getOffsetFlagString());
		holder.priceLabel.setText(String.valueOf(filteredData.get(position).getLastPrice()));
		holder.volumeLabel.setText(String.valueOf(filteredData.get(position).getVol()));
		holder.orderRefLabel.setText(filteredData.get(position).getOrderId());
		holder.occur_time.setText(filteredData.get(position).getOccurTimeString());

		return convertView;
	}

	static class ViewHolder {
		TextView TradeType;
		TextView InstrumentIdLabel;
		TextView directionLabel;
		TextView offsetLabel;
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
			filteredData.clear();
			for (int i = 0; i < list.size(); i++) {
				if (include_trade && list.get(i).getType() == TradeEntity.type.Trade) {
					filteredData.add(list.get(i));
				}
				if (include_insertOrder && list.get(i).getType() == TradeEntity.type.Insert_Order) {
					filteredData.add(list.get(i));
				}
				if (include_cancelOrder && list.get(i).getType() == TradeEntity.type.Cancell_Order) {
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
			if (results.count > 0) {
				notifyDataSetChanged();
			} else {
				notifyDataSetInvalidated();
			}
		}

	}
}
