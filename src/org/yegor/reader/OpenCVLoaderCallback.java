package org.yegor.reader;

import android.app.Activity;
import android.util.Log;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;

class OpenCVLoaderCallback extends BaseLoaderCallback
{
    final static String TAG= "reader_OpenCVLoaderCallback";

    public OpenCVLoaderCallback(Activity activity) {
        super(activity);
    }

    @Override
    public void onManagerConnected(int status) {
        switch (status) 
        {
            case LoaderCallbackInterface.SUCCESS:
            {
                Log.i(TAG, "OpenCV library loaded successfully");
                break;    
            }
            default:
            {
                String errorMessage= "Can not load OpenCV library";
                Log.e(TAG, errorMessage);
                super.onManagerConnected(status);
                throw new RuntimeException(errorMessage);
            }
        }
    } 
}
