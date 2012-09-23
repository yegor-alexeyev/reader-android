#include <string.h>
#include <jni.h>

#include <unistd.h>
#include <math.h>

inline uint8_t get_color_value(jbyte* data,size_t x,size_t y, jint width) {
  jbyte jvalue= data[y*width+x];
  return jvalue < 0 ? jvalue + 256 : jvalue;
} 

jint 
Java_org_yegor_reader_PreviewProcessor_processFrame( JNIEnv* env,
                                                  jobject thiz, jbyteArray jdata, jint width, jint height)
{
  jbyte* data= (*env)->GetByteArrayElements(env,jdata,NULL); 
  if (data == NULL) {
    return -1;
  }

  int counter = 0;
  size_t x,y;
  for (y= height/8; y < 7*height/8; y++) {
    for (x= width/8; x < 7*width/8; x++) {
      uint8_t middle_color= get_color_value(data,x,y,width);
      uint8_t top_color= get_color_value(data,x,y-1,width);
      uint8_t right_color= get_color_value(data,x+1,y,width);
      uint8_t bottom_color= get_color_value(data,x,y+1,width);
      uint8_t left_color= get_color_value(data,x-1,y,width);
      int delta= abs(middle_color*4 - top_color - right_color - bottom_color - left_color);
      if (delta > 64) {
        counter++;
      }
    }
  }

  (*env)->ReleaseByteArrayElements(env,jdata,data,0);

  return counter;
}
