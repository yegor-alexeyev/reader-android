#include <string.h>
#include <jni.h>

#include <unistd.h>
#include <cstdlib>

#include <map>
#include <set>
#include <vector>

#include <android/log.h>

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


        bool operator <(const BitmapBase& rightArgument) const {
            if (data != rightArgument.data) {
                return data < rightArgument.data;
            }
            if (height != rightArgument.height) {
                return height < rightArgument.height;
            }
            return width < rightArgument.width;
        } 

    private:
        jbyte* data;
    public:
        size_t width;
        size_t height;
};

uint8_t get_color(jbyte value) {
    return value < 0 ? value + 256 : value;
}

jbyte get_jbyte_value(uint8_t color) {
    return color > 127 ? color - 256 : color;
}

inline void set_color_value(jbyte* data,size_t x,size_t y, size_t width, uint8_t color) {
    
    jbyte value;
    if (color > 127) {
        value= color - 256;
    } else {
        value= color;
    }
   
  data[y*width+x]= value;
} 

class Pixel {
    friend class Bitmap;
private:
    Pixel(const BitmapBase& bitmap, size_t x, size_t y) :
        bitmap(bitmap),
        x(x),
        y(y) {
    }

     BitmapBase bitmap;

public:

    bool operator < (const Pixel& rightArgument) const {
        if (bitmap < rightArgument.bitmap || rightArgument.bitmap < bitmap) {
            return bitmap < rightArgument.bitmap;
        }
        if (y != rightArgument.y) {
            return y < rightArgument.y;
        }
        return x < rightArgument.x;
    }

    uint8_t color() const {
        return get_color(bitmap.data[bitmap.width*y+x]);
    }

    void setColor(uint8_t color) {
        set_color_value(bitmap.data,x,y,bitmap.width,color);
    }
    size_t x;
    size_t y;

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

class group_range_hasher {
public:

    size_t operator ()(uint32_t value) const {
        return value;
    }
};

class ManagerOfGroups {
    public:
        ManagerOfGroups(size_t width, size_t height) :
           width(width),
           height(height),
           board(width*height,0),
           groupCounter(0)
        {
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
            at(pixel)= pixel.y*width+pixel.x+1;
            create_count++;
        }

        void addToGroup(uint32_t groupNumber, Pixel pixel) {
            at(pixel)= groupNumber;
            add_count++;
        }

        void mergeGroups(Pixel pixel1, Pixel pixel2) {

            Pixel eraseGroupPixel=  at(pixel1) > at(pixel2) ? pixel1 : pixel2;
            Pixel pixelOfGroupToExpand= at(pixel1) > at(pixel2) ? pixel2 : pixel1;

            uint32_t eraseNumber= at(eraseGroupPixel); 
            uint32_t expandNumber= at(pixelOfGroupToExpand); 

            std::vector<Pixel> toDoStack;

            toDoStack.push_back(eraseGroupPixel);
            while (!toDoStack.empty()) {
                Pixel pixel= toDoStack.back();
                toDoStack.pop_back();
                
                if (isInGroup(pixel) && at(pixel) == eraseNumber) {
                    at(pixel)= expandNumber;
                    if (pixel.y != 0) {
                        toDoStack.push_back(pixel.highNeighbor());
                    }
                    if (pixel.x != width - 1) {
                        toDoStack.push_back(pixel.rightNeighbor());
                    }
                    if (pixel.y != height - 1) {
                        toDoStack.push_back(pixel.lowNeighbor());
                    }
                    if (pixel.x != 0) {
                        toDoStack.push_back(pixel.leftNeighbor());
                    }
                }
            }
            merge_count++;
        }
        uint32_t getLastGroupNumber() const {
            return groupCounter;
        }

    private:


        std::vector<uint32_t> board;


        uint32_t groupCounter;
    public:

        size_t width;  
        size_t height;  

        const uint32_t& at(Pixel pixel) const {
            return board.at(pixel.y*width+pixel.x);
        }

        uint32_t& at(Pixel pixel) {
            return board.at(pixel.y*width+pixel.x);
        }
};





void processNeighbor(ManagerOfGroups& manager, Pixel pixel, Pixel neighbor) {
    if (!manager.isInGroup(neighbor) || manager.getGroupNumber(neighbor) != manager.getGroupNumber(pixel)) {
        if (pixel.color() == neighbor.color()) {
            if (manager.isInGroup(neighbor)) {
                manager.mergeGroups(pixel, neighbor);
            } else {
                uint32_t group = manager.getGroupNumber(pixel);
                manager.addToGroup(group,neighbor);
            }
        }
    }
}

void groupPixels(ManagerOfGroups& manager, Bitmap bitmap) {
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

        colorMark(data, width, height, x, y, fillColor); 
      }
    }
  }
}

Pixel unpackGroupPixel(Bitmap& bitmap, uint32_t groupNumber) {
    uint32_t index= groupNumber - 1;
    return bitmap.pixel(index%bitmap.width, index/bitmap.width);
}

///////////////////////////////////////////////////////////

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
    recursive_count= 0;

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
    LOG("recursive_count = %d",recursive_count);

    uint32_t countOfAnclaves= 0;

    for (size_t y= 0; y< height; y++) {
        for (size_t x= 0; x < width; x++) {
            Pixel pixel= bitmap.pixel(x,y);
            if (manager.getGroupNumber(pixel) == y*width+x+1) {
                if (processGroup(bitmap,manager,pixel)) {
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
