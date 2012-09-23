package org.yegor.reader;

import android.util.Log;

class Image {
    private static final String TAG= "reader_Image";
    public final byte[] data;
    public final int width;
    public final int height;

    public Image(byte[] data, int width, int height) {
        this.data= data;
        this.width= width;
        this.height= height;

        if (width % 16 != 0 && height % 16 != 0) {
            String message= "Unsupported preview frame size";
            Log.e(TAG,message);
            throw new RuntimeException(message);
        }
    }

    int getColor(int x,int y) {
        byte value = data[y*width + x];
        return value < 0 ? 256 + value : value;
    }

}
