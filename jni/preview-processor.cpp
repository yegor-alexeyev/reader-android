#include <jni.h>

#include <android/log.h>
#define  LOG(x...)  __android_log_print(ANDROID_LOG_INFO,"reader",x)
#include "image-processor.hpp"
#include <unistd.h>
#include <cstdlib>

#include <map>
#include <set>
#include <vector>


#include <memory>



/*
bool processGroup(Bitmap bitmap, ManagerOfGroups& manager,Pixel pixel) {
    std::set<Pixel> processedPixels;
    std::vector<Pixel> toDoStack;

    std::set<uint32_t> neighbors;
    uint32_t groupNumber= manager.getGroupNumber(pixel);

    toDoStack.push_back(pixel);
    while (!toDoStack.empty()) {
        Pixel nextPixel= toDoStack.back();
        toDoStack.pop_back();
        if (processedPixels.count(nextPixel) == 0) {
            if (manager.getGroupNumber(nextPixel) == groupNumber) {
                processedPixels.insert(nextPixel); 
                if (nextPixel.y != 0) {
                    toDoStack.push_back(nextPixel.highNeighbor());
                }
                if (nextPixel.x != manager.width - 1) {
                    toDoStack.push_back(nextPixel.rightNeighbor());
                }
                if (nextPixel.y != manager.height - 1) {
                    toDoStack.push_back(nextPixel.lowNeighbor());
                }
                if (nextPixel.x != 0) {
                    toDoStack.push_back(nextPixel.leftNeighbor());
                }
            } else {
                if (nextPixel.color() == pixel.color()) {
                    LOG("Same-color neighbors: (%d %d) and (%d %d), color = %d",nextPixel.x,nextPixel.y,pixel.x,pixel.y,pixel.color());
                }
               neighbors.insert(manager.getGroupNumber(nextPixel));
            } 
        }
    }



    std::set<uint32_t>::const_iterator item= neighbors.begin();
    if (item == neighbors.end()) {
        return true;
    }
    bool isMaximum= pixel.color() > unpackGroupPixel(bitmap,*item).color();

    item++;
    for (; item != neighbors.end(); ++item) {
        Pixel groupPixel = unpackGroupPixel(bitmap, *item);
        bool isNextMaximum= pixel.color() > groupPixel.color(); 
        if (isMaximum != isNextMaximum) {
            return false;
        }
    }

   for (std::set<Pixel>::const_iterator item= processedPixels.begin(); item != processedPixels.end(); ++item) {
        Pixel pixel= *item;
        pixel.setColor(isMaximum ? 255 : 0);
    }
    return true;
}
*/

extern "C" {

JNIEXPORT jint JNICALL
Java_org_yegor_reader_PreviewProcessor_processFrame( JNIEnv* env,jobject thiz, jbyteArray jdata, jint width, jint height)
{
    jbyte* yuv= (*env).GetByteArrayElements(jdata,NULL); 
    if (yuv == NULL) {
      return -1;
    }

    create_count= 0;
    merge_count= 0;
    add_count= 0;

    Bitmap bitmap(yuv, width,height);
    ManagerOfGroups manager(bitmap.width,bitmap.height);
    groupPixels(manager, bitmap);
/*
    for (size_t y= 0; y< height; y++) {
        for (size_t x= 0; x < width; x++) {
            Pixel pixel= bitmap.pixel(x,y);
            uint32_t number = manager.isInGroup(pixel) ? manager.getGroupNumber(pixel) : 0;
            //uint8_t color= number*255/manager.getLastGroupNumber();
            uint8_t color= number*100 % 256;
            
            pixel.setColor(color);
        }
    }
*/
    LOG("create count = %d",create_count);
    LOG("merge_count = %d",merge_count);
    LOG("add_count = %d",add_count);

    uint32_t countOfAnclaves= 0;

    for (size_t y= 0; y< height; y++) {
        for (size_t x= 0; x < width; x++) {
            Pixel pixel= bitmap.pixel(x,y);
            if (manager.getGroupNumber(pixel) == y*width+x+1) {
                if (processGroupPeriphery(bitmap,manager,pixel)) {
                    countOfAnclaves++;
                }
            }
        }
    }

    LOG("Count of anclaves = %d",countOfAnclaves);

  (*env).ReleaseByteArrayElements(jdata,yuv,0);

  return 0;
}

///////////////////////////////////////////////////////////
} // extern "C" {
