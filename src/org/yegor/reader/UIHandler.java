package org.yegor.reader;

import java.util.List;
import java.util.concurrent.CountDownLatch;

import android.hardware.Camera;
import android.app.AlertDialog;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import org.yegor.reader.opencv.Loader;

public class UIHandler extends Activity implements Loader.ResultListener
{
    private static final String TAG = "reader_UIHandler";

    private final CountDownLatch initializationLatch= new CountDownLatch(1);

    private Camera camera;

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

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Log.i(TAG,"onCreate()");
        Loader.startLoad(this,this);
        setContentView(R.layout.main);
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
        try {
            initializationLatch.await();
        } catch (InterruptedException exception) {
            throw new RuntimeException(exception);
        }
        camera=Camera.open();
        Camera.Parameters parameters=camera.getParameters();

    }

    @Override
    public void onPause() {
        super.onPause();
        camera.release();
        Log.i(TAG,"onPause()");
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
}
