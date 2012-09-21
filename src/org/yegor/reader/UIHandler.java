package org.yegor.reader;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.SynchronousQueue;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.ImageFormat;
import android.graphics.Paint;
import android.hardware.Camera;
import android.app.AlertDialog;
import android.app.Activity;
import android.view.Display;
import android.view.SurfaceHolder;
import android.os.Bundle;
import android.util.Log;

import org.yegor.reader.opencv.Loader;
import org.yegor.reader.hardware.camera.Utility;


public class UIHandler extends Activity implements Loader.ResultListener, SurfaceHolder.Callback2, Camera.PreviewCallback,Runnable
{
    private static final String TAG = "reader_UIHandler";

    private final CountDownLatch initializationLatch= new CountDownLatch(1);

    private Camera camera;
    private Camera.Size previewSize;

    private Thread previewProcessor;

    public UIHandler() {
      Log.i(TAG,"UIHandler()");
    }
    @Override
    public void onSaveInstanceState(Bundle bundle) {
        super.onSaveInstanceState(bundle);
        Log.i(TAG,"onSaveInstanceState(Bundle)");
    }

    @Override
    public void onRestoreInstanceState(Bundle bundle) {
        super.onRestoreInstanceState(bundle);
        Log.i(TAG,"onRestoreInstanceState(Bundle)");
    }

    @Override
    public void onOpenCVLoaded() {
        Log.i(TAG,"onOpenCVLoaded()");
       initializationLatch.countDown(); 
    }

    private void openCamera() {
        List<Integer> cameras= Utility.listCameras(Camera.CameraInfo.CAMERA_FACING_BACK);
        if (cameras.isEmpty()) {
            throw new RuntimeException("This device does not have a back-facing camera");
        }
        int cameraId= cameras.get(0);
        camera= Camera.open(cameraId);
        int previewRotationAngle= 360 - Utility.getCameraOrientationAngle(cameraId);
        camera.setDisplayOrientation(previewRotationAngle);
        Camera.Parameters parameters= camera.getParameters();
        previewSize= parameters.getPreviewSize();
        Log.i(TAG,"Camera preview size: " + previewSize.width + "x" + previewSize.height);
        if (parameters.getSupportedFocusModes().contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
        }

        if (parameters.getSupportedSceneModes().contains(Camera.Parameters.SCENE_MODE_BARCODE)) {
            parameters.setSceneMode(Camera.Parameters.SCENE_MODE_BARCODE);
        }

        if (!parameters.getSupportedPreviewFormats().contains(ImageFormat.YV12)) {
           throw new RuntimeException("YV12 camera preview image format is not supported");
        }
        parameters.setPreviewFormat(ImageFormat.YV12);
        camera.setParameters(parameters);
        Log.i(TAG,"Scene mode: " + parameters.getSceneMode());
        Log.i(TAG,"Focus mode: " + parameters.getFocusMode());
    }

    private void releaseCamera() {
        camera.stopPreview();
        camera.setPreviewCallback(null);
        camera.release();
        camera= null;
    } 

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Log.i(TAG,"onCreate()");
        Loader.startLoad(this,this);
        openCamera();
        getWindow().takeSurface(this);
    }

    @Override
    public void onStart()
    {
        super.onStart();
        Log.i(TAG,"onStart()");
    }

    @Override
    public void onResume() 
    {
        super.onResume();
        Log.i(TAG,"onResume()");
        if (camera == null) {
            openCamera();
        }
/*
        try {
            initializationLatch.await();
        } catch (InterruptedException exception) {
            throw new RuntimeException(exception);
        }
*/

    }

    @Override
    public void onPause() {
        super.onPause();
        Log.i(TAG,"onPause()");
        releaseCamera();
        previewProcessor.interrupt();
    }

    @Override
    public void onStop() {
        super.onStop();
        Log.i(TAG,"onStop()");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG,"onDestroy()");
    }

    private SynchronousQueue<byte[]> previewFramesPipe= new SynchronousQueue<byte[]>();

    @Override
    public void onPreviewFrame(byte[] data,Camera camera) {
        if (!previewFramesPipe.offer(data)) {
            Log.d(TAG,"Preview frame was skipped");
        }
    }

    @Override
    public void run() {
        while (!Thread.currentThread().isInterrupted()) {
            try {
                byte[] previewFrame= previewFramesPipe.take();
                Log.d(TAG,"received preview frame: length = " + previewFrame.length);
                if (previewSize.width % 16 == 0 && previewSize.height % 16 == 0) {
                    int[] planeYdata= new int[previewSize.width*previewSize.height]; 
                    for (int i=0; i < planeYdata.length; i++) {
                        planeYdata[i]= previewFrame[i];
                    }
                    Bitmap planeY= Bitmap.createBitmap(planeYdata,previewSize.width,previewSize.height,Bitmap.Config.RGB_565);
                    Canvas canvas= mainSurfaceHolder.lockCanvas();
                    canvas.drawBitmap(planeY,0f,0f,new Paint());
                    if (canvas != null) {
                        mainSurfaceHolder.unlockCanvasAndPost(canvas);
                    }
                } else {
                    Log.e(TAG,"Unsupported preview frame size");
                }
                
            } catch (InterruptedException exception) {
                return;
            }
        }
    }

    private SurfaceHolder mainSurfaceHolder;

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mainSurfaceHolder= holder;
/*
        try {
            camera.setPreviewDisplay(holder);
        } catch (IOException exception) {
            Log.e(TAG,"Error executing Camera::setPreviewDisplay("+holder+")");
            throw new RuntimeException(exception);
        }
*/
        camera.setPreviewCallback(this);
        camera.startPreview();
        previewProcessor= new Thread(this);
        previewProcessor.start();

        Log.i(TAG,"surfaceCreated("+holder+")");
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.i(TAG,"surfaceChanged()");
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.i(TAG,"surfaceDestroyed()");
    } 

    @Override
    public void surfaceRedrawNeeded(SurfaceHolder holder) {
        Log.i(TAG,"surfaceRedrawNeeded()");
    }
}
