#include <string.h>
#include <jni.h>

#include <unistd.h>

jint 
Java_org_yegor_reader_PreviewProcessor_processFrame( JNIEnv* env,
                                                  jobject thiz, jbyteArray jdata, jint width, jint height)
{
  int minimal_value = 25500;
/*  jbyte* data= (*env)->GetByteArrayElements(env,jdata,NULL); 
  if (data == NULL) {
    return -1;
  }

  size_t x,y;
  for (y= 0; y < height; y++) {
    for (x= 0; x < width; x++) {
      jbyte jvalue= data[y*width+x];
      uint8_t value= jvalue < 0 ? jvalue + 256 : jvalue;
      if (value < minimal_value) {
        minimal_value= value;
      }
    }
  }



  (*env)->ReleaseByteArrayElements(env,jdata,data,0);
*/
  const char* result= "JNI_FALSE";

  return minimal_value;

      
}
