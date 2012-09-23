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
/*
                for (int y=0; y < previewFrame.height; y+=8) {
                    for (int x=0; x < previewFrame.width; x++) {
                        previewFrame.data[y*previewFrame.width+x]=-1;
                    }
                }
 */               
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
