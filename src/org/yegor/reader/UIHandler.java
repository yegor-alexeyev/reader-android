package org.yegor.reader;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.SynchronousQueue;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.app.AlertDialog;
import android.app.Activity;
import android.view.Display;
import android.view.SurfaceHolder;
import android.os.Bundle;
import android.util.Log;

import org.yegor.reader.Image;
import org.yegor.reader.PreviewProcessor;
import org.yegor.reader.hardware.camera.Utility;


public class UIHandler extends Activity implements SurfaceHolder.Callback2, Camera.PreviewCallback
{
    private static final String TAG = "reader_UIHandler";

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
        Camera.Size previewSize= parameters.getPreviewSize();
        Log.i(TAG,"Camera preview size: " + previewSize.width + "x" + previewSize.height);
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

    private SynchronousQueue<Image> previewFramesPipe= new SynchronousQueue<Image>();

    @Override
    public void onPreviewFrame(byte[] data,Camera camera) {
        Camera.Size previewSize= camera.getParameters().getPreviewSize();
        Image image= new Image(data,previewSize.width,previewSize.height);
        if (!previewFramesPipe.offer(image)) {
            camera.addCallbackBuffer(data);
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
        camera.setPreviewCallbackWithBuffer(this);
        Camera.Size previewSize= camera.getParameters().getPreviewSize();
        int bufferSize= previewSize.width*previewSize.height*ImageFormat.getBitsPerPixel(ImageFormat.NV21)/8;

        camera.addCallbackBuffer(new byte[bufferSize]);
        camera.addCallbackBuffer(new byte[bufferSize]);
        camera.addCallbackBuffer(new byte[bufferSize]);
        camera.addCallbackBuffer(new byte[bufferSize]);
        camera.addCallbackBuffer(new byte[bufferSize]);
        camera.addCallbackBuffer(new byte[bufferSize]);
        camera.startPreview();

        PreviewProcessor previewProcessor= new PreviewProcessor(camera,mainSurfaceHolder,previewFramesPipe);
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
