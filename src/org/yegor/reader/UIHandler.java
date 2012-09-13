package org.yegor.reader;

import java.util.List;

import android.hardware.Camera;
import android.app.AlertDialog;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public class UIHandler extends Activity
{
    private static final String TAG = "reader_UIHandler";

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
        setContentView(R.layout.main);
    }

    @Override
    public void onStart() {
        super.onStart();
        Log.i(TAG,"UIHandler::onStart()");
    }

    @Override
    public void onResume() 
    {
        super.onResume();
        Log.i(TAG,"UIHandler::onResume()");
        Camera camera=Camera.open();
        Camera.Parameters parameters=camera.getParameters();


        List<Camera.Size> sizes = parameters.getSupportedPreviewSizes();

        String sizesString = "";
        for (Camera.Size size: sizes) {
            sizesString+=size.width+"x"+size.height+", ";
        }

        AlertDialog infoBox = new AlertDialog.Builder(this).create();
        infoBox.setMessage("The supported preview sizes: " + sizesString);
        infoBox.show();
    }

    @Override
    public void onPause() {
      super.onPause();
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
