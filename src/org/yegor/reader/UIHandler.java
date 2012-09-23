package org.yegor.reader;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.SynchronousQueue;

import android.hardware.Camera;
import android.app.AlertDialog;
import android.app.Activity;
import android.view.Display;
import android.view.SurfaceHolder;
import android.os.Bundle;
import android.util.Log;

import org.yegor.reader.PreviewProcessor;
import org.yegor.reader.opencv.Loader;
import org.yegor.reader.hardware.camera.Utility;


public class UIHandler extends Activity implements Loader.ResultListener, SurfaceHolder.Callback2, Camera.PreviewCallback
{
    private static final String TAG = "reader_UIHandler";

    private final CountDownLatch initializationLatch= new CountDownLatch(1);

    private Camera camera;

    private Thread previewProcessorThread;

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
        if (parameters.getSupportedFocusModes().contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
        }

        if (parameters.getSupportedSceneModes().contains(Camera.Parameters.SCENE_MODE_BARCODE)) {
            parameters.setSceneMode(Camera.Parameters.SCENE_MODE_BARCODE);
        }

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
        Log.i(TAG,"onPause() exits");
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
        Camera.Parameters parameters= camera.getParameters();
        Camera.Size previewSize= parameters.getPreviewSize();
        Log.i(TAG,"Camera preview size: " + previewSize.width + "x" + previewSize.height);

        PreviewProcessor previewProcessor= new PreviewProcessor(mainSurfaceHolder,previewSize,previewFramesPipe);
        previewProcessorThread= new Thread(previewProcessor);
        previewProcessorThread.start();

        Log.i(TAG,"surfaceCreated("+holder+")");
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.i(TAG,"surfaceChanged()");
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.i(TAG,"surfaceDestroyed()");
        previewProcessorThread.interrupt();
    } 

    @Override
    public void surfaceRedrawNeeded(SurfaceHolder holder) {
        Log.i(TAG,"surfaceRedrawNeeded()");
    }
}
