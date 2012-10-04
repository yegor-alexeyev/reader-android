#include <string.h>
#include <jni.h>

#include <unistd.h>
#include <cstdlib>

#include <android/log.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "highlight_letters.h"

#include <memory>

#define  LOG(x...)  __android_log_print(ANDROID_LOG_INFO,"reader",x)

class BitmapBase {
    friend class Pixel;
    public:
        BitmapBase(jbyte* data,size_t width,size_t height) :
            data(data),
            width(width),
            height(height) {
        } 


    private:
        jbyte* data;
    public:
        const size_t width;
        const size_t height;
};


class Pixel {
    friend class Bitmap;
private:
    Pixel(const BitmapBase& bitmap, size_t x, size_t y) :
        bitmap(bitmap),
        x(x),
        y(y) {
    }

    const BitmapBase bitmap;

public:
    uint8_t color() const {
        return bitmap.data[bitmap.width*y+x];
    }

    const size_t x;
    const size_t y;

    Pixel highNeighbor() {
        return Pixel(bitmap,x,y-1);       
    }
        
    Pixel lowNeighbor() {
        return Pixel(bitmap,x,y+1);
    }
    Pixel leftNeighbor() {
        return Pixel(bitmap,x-1,y);
    }

    Pixel rightNeighbor() {
        return Pixel(bitmap,x+1,y);
    }

};

class Bitmap : public BitmapBase {
    public:
        Bitmap(jbyte* data,size_t width,size_t height) :
            BitmapBase(data,width,height) {
        }

        Pixel pixel(size_t x, size_t y) const {
            return Pixel(*this, x, y);
        }
};

static int create_count= 0;
static int merge_count= 0;
static int add_count= 0;
static int recursive_count= 0;

class ManagerOfGroups {
    public:
        ManagerOfGroups(size_t width, size_t height) :
           width(width),
           height(height),
           board(width*height,0),
           groupCounter(0)  {
        }

        uint32_t getGroupNumber(Pixel pixel) const {
            if (!isInGroup(pixel)) {
                LOG("ERROR: getGroupNumber called for a pixel not in group");
            }
            return at(pixel);
        }

        uint32_t isInGroup(Pixel pixel) const {
            return at(pixel) != 0;
        }

        void createGroup(Pixel pixel) {
            groupCounter++;
            at(pixel)= groupCounter;
            create_count++;
        }

        void addToGroup(uint32_t groupNumber, Pixel pixel) {
            at(pixel)= groupNumber;
            add_count++;
        }

        void mergeGroups(Pixel pixel1, Pixel pixel2) {

            Pixel pixelOfGroupToErase=  at(pixel1) > at(pixel2) ? pixel1 : pixel2;
            Pixel pixelOfGroupToExpand= at(pixel1) > at(pixel2) ? pixel2 : pixel1;

            recursivelyMergeGroup(at(pixelOfGroupToErase),at(pixelOfGroupToExpand),pixelOfGroupToErase);

            merge_count++;
        }

    private:
        void recursivelyMergeGroup(uint32_t eraseNumber, uint32_t expandNumber, Pixel position) {
            recursive_count++;
            if (isInGroup(position) && at(position) == eraseNumber) {
                at(position)= expandNumber;
                if (position.y != 0) {
                    recursivelyMergeGroup(eraseNumber, expandNumber, position.highNeighbor());
                }
                if (position.x != width) {
                    recursivelyMergeGroup(eraseNumber, expandNumber, position.rightNeighbor());
                }
                if (position.y != height) {
                    recursivelyMergeGroup(eraseNumber, expandNumber, position.lowNeighbor());
                }
                if (position.x != 0) {
                    recursivelyMergeGroup(eraseNumber, expandNumber, position.leftNeighbor());
                }
            }
        }

        const uint32_t& at(Pixel pixel) const {
            return board[pixel.y*width+pixel.x];
        }

        uint32_t& at(Pixel pixel) {
            return board[pixel.y*width+pixel.x];
        }

        size_t width;  
        size_t height;  
        std::vector<uint32_t> board;

        uint32_t groupCounter;
};





void processNeighbor(ManagerOfGroups& manager, Pixel pixel, Pixel neighbor) {
    if (!manager.isInGroup(neighbor) || manager.getGroupNumber(neighbor) != manager.getGroupNumber(pixel)) {
        if (abs(pixel.color()  - neighbor.color()) < 50) {
            if (manager.isInGroup(neighbor)) {
                manager.mergeGroups(pixel, neighbor);
            } else {
                uint32_t group = manager.getGroupNumber(pixel);
                manager.addToGroup(group,neighbor);
            }
        }
    }
}

void groupPixels(Bitmap bitmap) {
    ManagerOfGroups manager(bitmap.width,bitmap.height);
    for (size_t x= 0; x < bitmap.width; x++) {
        for (size_t y= 0; y < bitmap.height; y++) {
            Pixel pixel= bitmap.pixel(x,y);
            if (!manager.isInGroup(pixel)) {
                manager.createGroup(pixel);
            }
            if (x != bitmap.width - 1) {
                processNeighbor(manager,pixel, pixel.rightNeighbor());
            }
            if (y != bitmap.height - 1) {
                processNeighbor(manager,pixel, pixel.lowNeighbor());
            }
        }
    }
}




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

int get_color(jbyte value) {
    return value < 0 ? value + 256 : value;
}

jbyte get_jbyte_value(int color) {
    return color > 127 ? color - 256 : color;
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

    create_count= 0;
    merge_count= 0;
    add_count= 0;
    recursive_count= 0;

    groupPixels(Bitmap(yuv, width,height));

    LOG("create count = %d",create_count);
    LOG("merge_count = %d",merge_count);
    LOG("add_count = %d",add_count);
    LOG("recursive_count = %d",recursive_count);


    //cv::Mat lumaPlane(height,width,CV_8UC1,(unsigned char*) yuv);

    //for (int y= 0; y < lumaPlane.rows - 1; y++) {
        //for (int x= 0; x < lumaPlane.cols - 1; x++) {
           //int new_value= get_color(lumaPlane.at<jbyte>(y,x)) - get_color(lumaPlane.at<jbyte>(y + 1,x+1));
           //new_value= std::max(new_value,-128);
           //new_value= std::min(new_value,127);
           //new_value+=128;
           //lumaPlane.at<jbyte>(y,x)= get_jbyte_value(new_value); 
        //}
    //}
        
    //const int cn = lumaPlane.channels();
    //cv::Mat dx(lumaPlane.rows, lumaPlane.cols, CV_16SC(cn));
    //cv::Mat dy(src.rows, src.cols, CV_16SC(cn));
//Canny
    //cv::Sobel(lumaPlane, dx, CV_8U, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
//  highlight_letters(lumaPlane,dx,20,60,3,true);
  //dx.copyTo(lumaPlane);
  
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
//  findMarks(yuv,width,height);

  (*env).ReleaseByteArrayElements(jdata,yuv,0);

  return 0;
}

///////////////////////////////////////////////////////////
} // extern "C" {
