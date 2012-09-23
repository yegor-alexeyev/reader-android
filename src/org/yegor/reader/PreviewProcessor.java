package org.yegor.reader;

import java.util.concurrent.SynchronousQueue;

import android.util.Log;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;

import android.view.SurfaceHolder;

import org.yegor.reader.Image;

class PreviewProcessor implements Runnable {
    private static final String TAG= "reader_PreviewProcessor";

    private SurfaceHolder surfaceHolder;
    private SynchronousQueue<Image> sourceOfPreviewFrames;

    public PreviewProcessor(SurfaceHolder surfaceHolder, SynchronousQueue<Image> sourceOfPreviewFrames) {
        this.surfaceHolder= surfaceHolder;
        this.sourceOfPreviewFrames= sourceOfPreviewFrames;
    }

    private void processPreviewFrames() {
        while (!Thread.currentThread().isInterrupted()) {
            try {
                Image previewFrame= sourceOfPreviewFrames.take();
                for (int y=0; y < previewFrame.height; y+=8) {
                    for (int x=0; x < previewFrame.width; x++) {
                        previewFrame.data[y*previewFrame.width+x]=-1;
                    }
                }
                int[] planeYData= new int[previewFrame.width*previewFrame.height]; 
                int maxY= 0;
                int minY= 255;
                for (int i=0; i < planeYData.length; i++) {
                    int value= previewFrame.data[i];
                    int color= value < 0 ? 256 + value : value;
                    if (color > maxY) maxY= color;
                    if (color < minY) minY= color;

                    planeYData[i]= Color.rgb(color,color,color);
                }
                Log.i(TAG,"Luma values range in the preview frame: " + minY + " - " + maxY);
                Bitmap planeY= Bitmap.createBitmap(planeYData,previewFrame.width,previewFrame.height,Bitmap.Config.ARGB_8888);


                Canvas canvas= surfaceHolder.lockCanvas();
                if (canvas != null) {
//                        canvas.scale(1.3f,1.3f);
                    canvas.rotate(90f, 240,320);
                    canvas.drawBitmap(planeY,40f,80f,new Paint());
                    if (canvas != null) {
                        surfaceHolder.unlockCanvasAndPost(canvas);
                    }
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
