package org.yegor.reader.opencv;

import android.app.Activity;
import android.app.AlertDialog;

import android.content.DialogInterface;


import android.util.Log;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;

public class Loader {
    final static String TAG= "reader_opencv_Loader";

    public interface ResultListener {
        void onOpenCVLoaded();
    }

    public static class ImplementationCallback extends BaseLoaderCallback
    {
        final static String TAG= "reader_opencv_Loader_ImplementationCallback";

        private ResultListener listener;

        public ImplementationCallback(Activity activity,ResultListener listener) {
            super(activity);
            this.listener= listener;
        }

        @Override
        public void onManagerConnected(int status) {
            Log.i(TAG,"onManagerConnected");
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    if (listener != null) {
                        listener.onOpenCVLoaded();
                    }
                    Log.i(TAG, "OpenCV library loaded successfully");
                    break;    
                }
                default:
                {
                    super.onManagerConnected(status);
                }
            }
        } 
    }

    public static void startLoad(Activity activity,ResultListener listener) {
        ImplementationCallback callback= new ImplementationCallback(activity,listener);
        if (!OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_2, activity, callback))
        {
            Log.e(TAG,"OpenCVLoader.initAsync failed");
            showOpenCVInitializationErrorDialog(activity);
        }
    }

    private static void showOpenCVInitializationErrorDialog(final Activity activity) {
        AlertDialog initFailedDialog= new AlertDialog.Builder(activity).create();
        initFailedDialog.setTitle("Error");
        initFailedDialog.setMessage("OpenCV library can not be initialized. Application will be shut down");
        initFailedDialog.setCancelable(false);
        initFailedDialog.setButton(AlertDialog.BUTTON_POSITIVE,"OK",new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog,int which) {
                activity.finish();
            }
        });

        initFailedDialog.show();
    }

}
