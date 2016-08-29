package com.android.futures.tcp;


public class PositionInfo {
	private String Details;
	private double Balance;
	private double Available;
	private double Margin;
	private double ForzenMargin;
	private double Commission;
	private double ForzenCommission;
	private double PositionProfit;

	
	PositionInfo(double balance, double available, double margin, double forzenMargin, double commission, double forzenCommission, double positionProfit, String details){
		setBalance(balance);
		setAvailable(available);
		setMargin(margin);
		setForzenMargin(forzenMargin);
		setCommission(commission);
		setForzenCommission(forzenCommission);
		setPositionProfit(positionProfit);
		setDetails(details);
	}

	public String getDetails() {
		return Details;
	}

	public void setDetails(String details) {
		Details = details;
	}

	public double getBalance() {
		return Balance;
	}

	public void setBalance(double balance) {
		Balance = balance;
	}

	public double getAvailable() {
		return Available;
	}

	public void setAvailable(double available) {
		Available = available;
	}

	public double getMargin(){
		return Margin;
	}

	public void setMargin(double margin){
		Margin = margin;
	}

	public double getForzenMargin() {
		return ForzenMargin;
	}

	public void setForzenMargin(double forzenMargin) {
		ForzenMargin = forzenMargin;
	}

	public double getCommission(){
		return Commission;
	}

	public void setCommission(double commission){
		Commission = commission;
	}

	public double getForzenCommission(){
		return ForzenCommission;
	}

	public void setForzenCommission(double forzenCommission){
		ForzenCommission = forzenCommission;
	}

	public double getPositionProfit() {
		return PositionProfit;
	}

	public void setPositionProfit(double positionProfit) {
		PositionProfit = positionProfit;
	}


}
