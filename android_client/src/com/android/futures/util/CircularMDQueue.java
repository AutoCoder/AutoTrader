package com.android.futures.util;

import java.util.ArrayList;
import java.util.Collections;
import java.util.RandomAccess;
import com.android.futures.entity.MDEntity;


public class CircularMDQueue implements RandomAccess {
	  
    private final int n; // buffer length
    private final ArrayList<MDEntity> buf; // a List implementing RandomAccess
    private int head = 0;
    private int size = 0;

    public CircularMDQueue(int capacity) {
        n = capacity;
        buf = new ArrayList<MDEntity>(Collections.nCopies(n, (MDEntity) null));
    }
  
    public int limit() {
        return n;
    }
    
    public void clear(){
    	buf.clear();
    }
    
    public synchronized Boolean IsEmpty(){
    	return buf.isEmpty();
    }   
    
    public synchronized MDEntity lastElement(){
    	return get(size-1);
    }
    
    public synchronized int size(){
    	return size;
    }
  
    public synchronized MDEntity get(int i) {
    	 //i is the logical idx for Elements
        if (i < 0 || i >= size()) {
            throw new IndexOutOfBoundsException();
        }
       
        int idx = i % n;//calulate the real idx for memory.
        return buf.get(idx);
    }
    
    public synchronized ArrayList<MDEntity> getLastElements(int count){
    	assert(count <= n);
    	int beginIdx = size > count ? (size - count) : 0;
		System.out.print("m_beginIdx is " + beginIdx + "...");
		System.out.print("currentSize is " + size + "...  ");

        ArrayList<MDEntity> array = new ArrayList<MDEntity>();
        for (int i =0 ; i < size - beginIdx; i++){
        	array.add(new MDEntity(get(beginIdx+i)));
        }
        
        return array;    	
    }
    
    //old ----------->new
    public synchronized void Append(MDEntity e) {
    	if (size < n){
    		buf.set(head+size, e);
    	}
    	else{
    		buf.set(head, e);
    		head++;
    		head = head % n;
    	}
    	size++;
    }
}