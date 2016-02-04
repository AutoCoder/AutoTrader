package com.android.futures.tcp;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.Socket;
 
public  class WriterTask extends Thread{
 
    private BufferedWriter bufferedWriter; 
    private String msg = null;
    private Socket socket = null;
    
    public WriterTask(Socket socket) throws IOException {
        this.bufferedWriter = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
        this.socket = socket;
    }

        /**
     * finishTask:(这里用一句话描述这个方法的作用). <br/>
     * TODO(这里描述这个方法适用条件 – 可选).<br/>
     * @throws IOException 
     *
     */
    public void finish() throws IOException {
        if(bufferedWriter!=null && socket!=null)
        {
            if(!socket.isOutputShutdown())
            {
                socket.shutdownOutput();
            }
            bufferedWriter.close();
        }
    }
     
    /* (non-Javadoc)
     * @see java.lang.Runnable#run()
     */
    @Override
    public synchronized void run() {
        try {
            bufferedWriter.write(msg);
            bufferedWriter.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
     
    public void send(String msg){
        this.msg = msg;
        new Thread(this).start();
    }
     
}
