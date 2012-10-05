package org.yegor.reader;

import java.util.Arrays;
import java.util.concurrent.SynchronousQueue;

import android.util.Log;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;

import android.view.SurfaceHolder;

import android.hardware.Camera;

import org.yegor.reader.Image;

public class PreviewProcessor implements Runnable {
    private static final String TAG= "reader_PreviewProcessor";

    private SurfaceHolder surfaceHolder;
    private SynchronousQueue<Image> sourceOfPreviewFrames;
    private Camera camera;

    static {
        System.loadLibrary("preview-processor");
    }

    public PreviewProcessor(Camera camera, SurfaceHolder surfaceHolder, SynchronousQueue<Image> sourceOfPreviewFrames) {
        this.surfaceHolder= surfaceHolder;
        this.sourceOfPreviewFrames= sourceOfPreviewFrames;
        this.camera= camera;
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
                int intData[]= new int[(frame.width - 2*70)*frame.height];
                
                for (int i= 0; i < (frame.width - 2*70)*frame.height; i++) {
                }

                for (int y= 0; y < frame.height; y++) {
                    for (int x= 70; x < frame.width - 70; x++) {
                        int intDataPosition= y*(frame.width - 70*2) + x - 70;
                        int frameDataPosition= y*frame.width + x;
                        intData[intDataPosition]= frame.data[frameDataPosition];

                        intData[intDataPosition]= frame.data[frameDataPosition] < 0 ? 256 + frame.data[frameDataPosition] : frame.data[frameDataPosition];
                        intData[intDataPosition]= intData[intDataPosition] + intData[intDataPosition]*256 + intData[intDataPosition]*256*256;
                        intData[intDataPosition]|= 0xFF000000;
                    }
                }

                camera.addCallbackBuffer(frame.data);
                Bitmap bitmapData= Bitmap.createBitmap(intData, frame.width - 70*2, frame.height, Bitmap.Config.ARGB_8888);
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
