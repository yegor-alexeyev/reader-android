package org.yegor.reader;

import java.util.Arrays;
import java.util.concurrent.SynchronousQueue;

import android.util.Log;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;

import android.view.SurfaceHolder;

import org.yegor.reader.Image;

public class PreviewProcessor implements Runnable {
    private static final String TAG= "reader_PreviewProcessor";

    private SurfaceHolder surfaceHolder;
    private SynchronousQueue<Image> sourceOfPreviewFrames;

    static {
        System.loadLibrary("preview-processor");
    }

    public PreviewProcessor(SurfaceHolder surfaceHolder, SynchronousQueue<Image> sourceOfPreviewFrames) {
        this.surfaceHolder= surfaceHolder;
        this.sourceOfPreviewFrames= sourceOfPreviewFrames;
    }

    private void processPreviewFrames() {
        while (!Thread.currentThread().isInterrupted()) {
            try {
                Image frame= sourceOfPreviewFrames.take();
                int counter= processFrame(frame.data, frame.width,frame.height);
                boolean is_image_sharp= counter > 100;
                if (is_image_sharp) {
                    Log.i(TAG,"sharp frame");
                } 
                int intData[]= new int[frame.width*frame.height];
                
                for (int i= 0; i < frame.width*frame.height; i++) {
                    intData[i]= frame.data[i] < 0 ? 256 + frame.data[i] : frame.data[i];
                    intData[i]= intData[i] + intData[i]*256 + intData[i]*256*256;
                    intData[i]|= 0xFF000000;
                }
                Bitmap bitmapData= Bitmap.createBitmap(intData, frame.width, frame.height, Bitmap.Config.ARGB_8888);
                Canvas canvas= surfaceHolder.lockCanvas();
                if (canvas!=null) {
                    canvas.rotate(90);
                    canvas.drawBitmap(bitmapData,0f,-frame.height, new Paint());
                    surfaceHolder.unlockCanvasAndPost(canvas);
                }
                    
                    
                //Log.i(TAG,"counters: " + counters[0] + ", " + counters[1] + ", " + counters[2]);
            } catch (InterruptedException exception) {
                return;
            }
        }
    }

    @Override
    public void run() {
        try {
            processPreviewFrames();
        }
        catch (Throwable error) {
            Log.e(TAG, "Error: " + error.toString() + ":" + error.getMessage());
            for (StackTraceElement location: error.getStackTrace()) {
                Log.e(TAG,"At: " +location.toString());
            }
        }
    }

    public static native int processFrame(byte[] data, int width, int height);
}
