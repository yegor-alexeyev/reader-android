package org.yegor.reader.hardware.camera;

import android.hardware.Camera;
import android.view.Display;
import android.view.Surface;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import android.util.Log;

public final class Utility {

    final static String TAG= "reader_hardware_camera_utility";

    public static List<Integer> listCameras(int cameraFacingType) {
        List<Integer> cameras= new ArrayList<Integer>();
        for (int cameraId= 0; cameraId < Camera.getNumberOfCameras(); cameraId++) {
            Camera.CameraInfo cameraInfo= new Camera.CameraInfo();
            Camera.getCameraInfo(cameraId, cameraInfo);
            if (cameraInfo.facing == cameraFacingType) {
                cameras.add(cameraId);
            }
        }
        return cameras;
    }
 
//Camera clockwise rotation angle relative to the phone native orientation
    public static int getCameraOrientationAngle(int cameraId) {
        Camera.CameraInfo cameraInfo= new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId,cameraInfo);
        return 360 - cameraInfo.orientation;
    }
 
//The returned degrees are relative to the display natural orientation
    public static int getDisplayClockwiseRotation(Display display) {
        return 360 - getRenderingClockwiseRotation(display);
    }

//The returned degrees are relative to the rendering in the natural orientation of a display
    public static int getRenderingClockwiseRotation(Display display) {
        int rotationType= display.getRotation();
        switch (rotationType) {
            case Surface.ROTATION_0: return 0;
            case Surface.ROTATION_90: return 90;
            case Surface.ROTATION_180: return 180;
            case Surface.ROTATION_270: return 270;
            default: throw new RuntimeException("Invalid rotation type");
        }
    }

    public static Camera.Size getMaximumPreviewSize(Camera camera) {
        Camera.Parameters parameters= camera.getParameters();
        List<Camera.Size> sizes = parameters.getSupportedPreviewSizes();
        Iterator<Camera.Size> iterator= sizes.iterator();
        Camera.Size candidateSize= iterator.next();
        while (iterator.hasNext()) {
            Camera.Size size= iterator.next();
            Log.i(TAG,"size = " + size.width + "x" + size.height);
            if (size.width*size.height > candidateSize.width*candidateSize.height) {
                candidateSize= size;
            }
        }
        return candidateSize;
    }
}

