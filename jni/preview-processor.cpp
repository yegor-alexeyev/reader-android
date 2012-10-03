#include <string.h>
#include <jni.h>

#include <unistd.h>
#include <cstdlib>

#include <android/log.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "highlight_letters.h"

inline uint8_t get_color_value(jbyte* data,size_t x,size_t y, jint width) {
  jbyte jvalue= data[y*width+x];
  return jvalue < 0 ? jvalue + 256 : jvalue;
} 

inline void set_color_value(jbyte* data,size_t x,size_t y, jint width, int color) {
  if (color > 255) {
    color= 255;
  }
  data[y*width+x]= color > 127 ? color - 256 : color;
//  data[y*width+x]= color;
} 

typedef uint16_t Count;

void colorMark(jbyte* data, Count width, Count height, Count x, Count y, int fillColor) {
  set_color_value(data,x,y,width,fillColor);
  fillColor= 80;
  for (int delta_x= -1; delta_x <= 1; delta_x++) {
    for (int delta_y= -1; delta_y <= 1; delta_y++) {
      if ((delta_x != 0 || delta_y != 0)) {
        if (get_color_value(data,x+delta_x,y+delta_y,width) == 255) {
          colorMark(data,width,height,x+delta_x,y+delta_y,fillColor);
        }
      }
    }
  }
}
          


void findMarks(jbyte* data, Count width,Count height) {
  int fillColor= 254;
  for ( Count y= height/8; y < 7*height/8; y++) {
    for ( Count x= width/8; x < 7*width/8; x++) { 
      if ( get_color_value(data, x,y,width)== 255 ) {
//        __android_log_print(ANDROID_LOG_INFO,"reader_native","Canny color: %d",get_color_value(data,x,y,width));        
        //fillColor+=10;
        colorMark(data, width, height, x, y, fillColor); 
      }
    }
  }
}



extern "C" {
///////////////////////////////////////////////////////////


JNIEXPORT jint JNICALL
Java_org_yegor_reader_PreviewProcessor_processFrame( JNIEnv* env,jobject thiz, jbyteArray jdata, jint width, jint height)
{
  jbyte* yuv= (*env).GetByteArrayElements(jdata,NULL); 
  if (yuv == NULL) {
    return -1;
  }

  cv::Mat lumaPlane(height,width,CV_8UC1,(unsigned char*) yuv);
  highlight_letters(lumaPlane,lumaPlane,20,60,3);
  
/*
  int counter = 0;
  size_t x,y;
  for (y= height/8; y < 7*height/8; y++) {
    for (x= width/8; x < 7*width/8; x++) {
      uint8_t middle_color= get_color_value(yuv,x,y,width);
//      uint8_t top_color= get_color_value(yuv,x,y-1,width);
      uint8_t right_color= get_color_value(yuv,x+1,y,width);
      uint8_t bottom_color= get_color_value(yuv,x,y+1,width);
//      uint8_t left_color= get_color_value(yuv,x-1,y,width);
//      int delta= abs(middle_color*4 - top_color - right_color - bottom_color - left_color);
      int delta= abs(middle_color*2 - right_color - bottom_color);
      if (delta > 16) {
//        counter++;
        set_color_value(yuv,x,y,width, 1);
      }
      else {
        set_color_value(yuv,x,y,width, 0);
      }
    }
  }
*/
  findMarks(yuv,width,height);

  (*env).ReleaseByteArrayElements(jdata,yuv,0);

  return 0;
}

///////////////////////////////////////////////////////////
} // extern "C" {
