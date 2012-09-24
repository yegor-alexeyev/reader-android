#include <string.h>
#include <jni.h>

#include <unistd.h>
#include <cstdlib>

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

extern "C" {
///////////////////////////////////////////////////////////

JNIEXPORT jint JNICALL
Java_org_yegor_reader_PreviewProcessor_processFrame( JNIEnv* env,jobject thiz, jbyteArray jdata, jint width, jint height)
{
  jbyte* data= (*env).GetByteArrayElements(jdata,NULL); 
  if (data == NULL) {
    return -1;
  }

  int counter = 0;
  size_t x,y;
  for (y= height/8; y < 7*height/8; y++) {
    for (x= width/8; x < 7*width/8; x++) {
      uint8_t middle_color= get_color_value(data,x,y,width);
//      uint8_t top_color= get_color_value(data,x,y-1,width);
      uint8_t right_color= get_color_value(data,x+1,y,width);
      uint8_t bottom_color= get_color_value(data,x,y+1,width);
//      uint8_t left_color= get_color_value(data,x-1,y,width);
//      int delta= abs(middle_color*4 - top_color - right_color - bottom_color - left_color);
      int delta= abs(middle_color*2 - right_color - bottom_color);
      if (delta > 16) {
//        counter++;
        set_color_value(data,x,y,width, 255);
      }
      else {
        set_color_value(data,x,y,width, 0);
      }
    }
  }

  (*env).ReleaseByteArrayElements(jdata,data,0);

  return counter;
}

void findMarks() {
}

///////////////////////////////////////////////////////////
} // extern "C" {
