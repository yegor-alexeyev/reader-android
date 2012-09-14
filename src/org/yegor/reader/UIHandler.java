package org.yegor.reader;

import java.util.List;

import org.opencv.android.OpenCVLoader;

import android.hardware.Camera;
import android.app.AlertDialog;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import org.yegor.reader.OpenCVLoaderCallback;

public class UIHandler extends Activity
{
    private static final String TAG = "reader_UIHandler";
    private Camera camera;

    public UIHandler() {
      Log.i(TAG,"UIHandler::UIHandler()");
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

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Log.i(TAG,"UIHandler::onCreate()");
        if (!OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_2, this, new OpenCVLoaderCallback(this)))
        {
            String errorMessage= "Cannot connect to OpenCV Manager";
            Log.e(TAG,errorMessage);
            throw new RuntimeException(errorMessage);
        }
        setContentView(R.layout.main);
    }

    @Override
    public void onStart()
    {
        super.onStart();
        Log.i(TAG,"UIHandler::onStart()");
    }

    @Override
    public void onResume() 
    {
        super.onResume();
        Log.i(TAG,"UIHandler::onResume()");
        camera=Camera.open();
        Camera.Parameters parameters=camera.getParameters();

    }

    @Override
    public void onPause() {
        super.onPause();
        camera.release();
        Log.i(TAG,"UIHandler::onPause()");
    }

    @Override
    public void onStop() {
        super.onStop();
        Log.i(TAG,"UIHandler::onStop()");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG,"UIHandler::onDestroy()");
    }
}
