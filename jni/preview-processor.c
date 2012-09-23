#include <string.h>
#include <jni.h>

jint 
Java_org_yegor_reader_PreviewProcessor_processFrame( JNIEnv* env,
                                                  jobject thiz, jbyteArray jdata, jint width, jint height)
{
  jbyte* data= (*env)->GetByteArrayElements(env,jdata,NULL); 

  (*env)->ReleaseByteArrayElements(env,jdata,data,0);

  const char* result= "JNI_FALSE";

  return 777;

  /*for (size_t y= 0; y < height; y++) {*/
    /*for (size_t x= 0; x < width; x++) {*/
      
}
