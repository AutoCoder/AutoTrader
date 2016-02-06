package com.android.futures.tcp;

import java.io.IOException;
import java.net.Socket;

public  class SocketHandler implements SocketStatusListener {
 
        private Socket socket=null;
        private ReaderTask reader;
        private WriterTask writer;
        private String address;
        private int port;
        
         
        public SocketHandler(String add, int p, TraderStatusListener delegate) throws IOException {
        	address = add;
        	port = p;

            Thread init_thread = new Thread(  
                    new Runnable(){  
                        @Override  
                        public void run() {  
                			try {
                				socket = new Socket(address, port);
    						} catch (Exception e) {
    							// TODO Auto-generated catch block
    							e.printStackTrace();
    						}                       
                        }  
                    }  
            );  
            init_thread.start();
            try {
				init_thread.join();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
            reader = new ReaderTask(socket, delegate);
            writer = new WriterTask(socket);
            onSocketStatusChanged(socket, STATUS_OPEN, null);
        }
         
         
        /**
         * sendMessage:(这里用一句话描述这个方法的作用). <br/>
         * TODO(这里描述这个方法适用条件 – 可选).<br/>
         */
        public  void sendMessage(String msg) {
            writer.send(msg);
        }
         
        public void listen(boolean isListen)
        {
            reader.startListener(this);
             
        }
         
        public void shutDown() {
             
            if(!socket.isClosed() &&socket.isConnected())
            {
                try {
                    writer.finish();
                    reader.finish();
                    socket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                    onSocketStatusChanged(socket, STATUS_CLOSE, e);
                }finally{
                    reader = null;
                    writer = null;
                    System.out.println("Socket连接已关闭！！");
                }
            }
             
        }
 
        @Override
        public void onSocketStatusChanged(Socket socket,int status, IOException e) {
             
            switch (status) {
             
                case SocketStatusListener.STATUS_CLOSE:
                case SocketStatusListener.STATUS_RESET:
                case SocketStatusListener.STATUS_PIP_BROKEN:
                    shutDown();
                break;
 
            default:
                break;
            }
        }
         
    }
