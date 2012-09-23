package org.yegor.reader;

import java.util.concurrent.SynchronousQueue;

import android.util.Log;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;

import android.hardware.Camera;

import android.view.SurfaceHolder;

class PreviewProcessor implements Runnable {
    private static final String TAG= "reader_PreviewProcessor";

    private SurfaceHolder surfaceHolder;
    private Camera.Size previewSize;
    private SynchronousQueue<byte[]> sourceOfPreviewFrames;

    public PreviewProcessor(SurfaceHolder surfaceHolder, Camera.Size previewSize, SynchronousQueue<byte[]> sourceOfPreviewFrames) {
        this.surfaceHolder= surfaceHolder;
        this.previewSize= previewSize;
        this.sourceOfPreviewFrames= sourceOfPreviewFrames;
    }

    private void processPreviewFrames() {
        while (!Thread.currentThread().isInterrupted()) {
            try {
                Log.i(TAG,"Preview Size width = " + previewSize.width);
                byte[] previewFrame= sourceOfPreviewFrames.take();
                //Log.d(TAG,"received preview frame: length = " + previewFrame.length);
                if (previewSize.width % 16 == 0 && previewSize.height % 16 == 0) {
                    for (int y=0; y < previewSize.height; y+=8) {
                        for (int x=0; x < previewSize.width; x++) {
                            previewFrame[y*previewSize.width+x]=-1;
                        }
                    }
                    int[] planeYData= new int[previewSize.width*previewSize.height]; 
                    int maxY= 0;
                    int minY= 255;
                    for (int i=0; i < planeYData.length; i++) {
                        int value= previewFrame[i];
                        int color= value < 0 ? 256 + value : value;
                        if (color > maxY) maxY= color;
                        if (color < minY) minY= color;

                        planeYData[i]= Color.rgb(color,color,color);
                    }
                    Log.i(TAG,"Luma values range in the preview frame: " + minY + " - " + maxY);
                    Bitmap planeY= Bitmap.createBitmap(planeYData,previewSize.width,previewSize.height,Bitmap.Config.ARGB_8888);


                    Canvas canvas= surfaceHolder.lockCanvas();
                    if (canvas != null) {
//                        canvas.scale(1.3f,1.3f);
                        canvas.rotate(90f, 240,320);
                        canvas.drawBitmap(planeY,40f,80f,new Paint());
                        if (canvas != null) {
                            surfaceHolder.unlockCanvasAndPost(canvas);
                        }
                    }
                } else {
                    Log.e(TAG,"Unsupported preview frame size");
                }
                
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
}
