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
                //Log.i(TAG,"JNI test: " + processFrame(frame.data, frame.width,frame.height));

                int counters[] = new int[256];
                for (int y=frame.height/8; y < 7*frame.height/8; y++) {
                    for (int x=frame.width/8; x < 7*frame.width/8; x++) {
                        int color= frame.getColor(x,y);
                        int neighbourColor= frame.getColor(x+1,y);
                        int difference= Math.abs(color - neighbourColor);
                        counters[difference]++; 
                    }
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
