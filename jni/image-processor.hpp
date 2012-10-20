
#include <unistd.h>
#include <cstdlib>

#include <vector>


class BitmapBase {
    friend class Pixel;
    public:
        BitmapBase(jbyte* data,size_t width,size_t height) :
            data(data),
            width(width),
            height(height) {
        } 


        bool operator !=(const BitmapBase& bitmapBase) {
            return data != bitmapBase.data || width != bitmapBase.width || height != bitmapBase.height;
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

enum Axis {
    axis_X = -1
    ,axis_Y = 1
};

Axis change_axis(Axis axis) {
    return axis == axis_X ? axis_Y: axis_X;
}

const int negative_direction = -1;
const int positive_direction = 1;
        


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

    bool operator != (const Pixel& pixel) {
        return bitmap != pixel.bitmap || x != pixel.x || y != pixel.y;
    }

    uint8_t color() const {
        //!!! return get_color(bitmap.data[bitmap.width*y+x]);
        return bitmap.data[bitmap.width*y+x];
    }

    void setColor(uint8_t color) {
        bitmap.data[bitmap.width*y+x]= color;
        //!!! set_color_value(bitmap.data,x,y,bitmap.width,color);
    }
    size_t x;
    size_t y;

    bool hasNeighbor(Axis axis, int direction) const {
        if (axis == axis_X) {
            if (direction == positive_direction) {
                return x+1 < bitmap.width;
            } else {
                return x > 0;
            }
        } else {
            if (direction == positive_direction) {
                return y+1 < bitmap.height;
            } else {
                return y > 0;
            }
        }
    }

    Pixel neighbor(Axis axis, int direction) const {
        if (axis == axis_X) {
            if (direction == positive_direction) {
                return rightNeighbor();
            } else {
                return leftNeighbor();
            }
        } else {
            if ( direction == positive_direction) {
                return lowNeighbor();
            } else {
                return highNeighbor();
            }
        }
    }

    Pixel highNeighbor() const{
        return Pixel(bitmap,x,y-1);       
    }
        
    Pixel lowNeighbor() const {
        return Pixel(bitmap,x,y+1);
    }
    Pixel leftNeighbor() const {
        return Pixel(bitmap,x-1,y);
    }

    Pixel rightNeighbor() const {
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

        bool isInSameGroup(Pixel pixel1, Pixel pixel2) const {
            return getGroupNumber(pixel1) == getGroupNumber(pixel2);
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

            Pixel eraseGroupPixel=  pixel1.y > pixel2.y ? pixel1 : pixel2;
            Pixel pixelOfGroupToExpand= pixel1.y > pixel2.y ? pixel2 : pixel1;

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
    if (!manager.isInGroup(neighbor) || !manager.isInSameGroup(pixel,neighbor)) {
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




inline uint8_t get_color_value(jbyte* data,size_t x,size_t y, size_t width) {
  jbyte jvalue= data[y*width+x];
  return jvalue < 0 ? jvalue + 256 : jvalue;
} 

inline void set_color_value(jbyte* data,size_t x,size_t y, size_t width, int color) {
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

class PixelEdge {
private:
    Pixel pixel;
    Axis axis;
    int direction;

public:
    int sideNumber() const {
        return (static_cast<int>(axis)+1) + (direction+1)/2;
    }

    bool operator !=(PixelEdge pixelEdge) {
        return pixel != pixelEdge.pixel
            || axis != pixelEdge.axis
            || direction != pixelEdge.direction;
    }

    Pixel pixelInside() {
        return pixel;
    }

    PixelEdge(Pixel pixel,Axis axis, int direction) :
        pixel(pixel),
        axis(axis),
        direction(direction){
    }

    int nextLeftDirection() const {
        if (axis == axis_X) {
            return -direction;
        } else {
            return direction;
        }
    }

    int nextRightDirection() const {
        if (axis == axis_X) {
            return direction;
        } else {
            return -direction;
        }
    }

    PixelEdge nextLeft() {
        Pixel straightNeighbor= pixel.neighbor(axis,direction);
        return PixelEdge(straightNeighbor.neighbor(change_axis(axis),nextLeftDirection()),change_axis(axis),nextLeftDirection());
    }

    
    PixelEdge nextStraight() {
        return PixelEdge(pixel.neighbor(axis,direction),axis,direction);
    }
    PixelEdge nextRight() {
        return PixelEdge(pixel,change_axis(axis),nextRightDirection());
    }

    bool hasPixelOutside() const {
        return pixel.hasNeighbor(change_axis(axis), nextLeftDirection());
    }

    bool isBorder() const {
        return !hasPixelOutside() || abs(pixelOutside().color() - pixel.color()) > 0;
    }

    //PixelEdge nextBorder() {
    PixelEdge nextBorder(uint8_t minimumColor, uint8_t maximumColor) {
        
        if (nextRight().isBorder()) {
            LOG("RIGHT\n");
            return nextRight();
        }
       
        if (!nextStraight().isBorder()) {
            LOG("LEFT\n");
            return nextLeft(); 
        }
 
        LOG("STRAIGHT\n");
        return nextStraight();
    }

    Pixel pixelOutside() const {
        return pixel.neighbor(change_axis(axis),nextLeftDirection());
    }
};



bool processGroupPeriphery(Bitmap bitmap, Pixel topPixel, Bitmap resultBitmap, Bitmap trailMap) {
    uint32_t length= 0;
    const PixelEdge startPixelEdge(topPixel,axis_X,positive_direction);
    {
    uint8_t minimumColor= 255;
    uint8_t maximumColor= 0;
    if (!startPixelEdge.isBorder()) {
        return false;
    }
    
    PixelEdge currentPixelEdge= startPixelEdge;
    LOG("START\n");
    do {
        length++;
        Pixel pixel= currentPixelEdge.pixelInside();
        if (pixel.color() > maximumColor) maximumColor= pixel.color();
        if (pixel.color() < minimumColor) minimumColor= pixel.color();
        Pixel trailPixel= trailMap.pixel(pixel.x,pixel.y);
        if ((trailPixel.color() & (1 << currentPixelEdge.sideNumber())) != 0) {
            LOG("RETURN FALSE");
            return false;
        }
        LOG("AT %d %d\n", pixel.x, pixel.y);
        currentPixelEdge= currentPixelEdge.nextBorder(minimumColor, maximumColor);
    }    
    while (currentPixelEdge != startPixelEdge);
    }
    if (length < 30) return false;
    {
    PixelEdge currentPixelEdge= startPixelEdge;
    uint8_t minimumColor= 255;
    uint8_t maximumColor= 0;
    do {
        Pixel pixel= currentPixelEdge.pixelInside();
        Pixel trailPixel= trailMap.pixel(pixel.x,pixel.y);
        trailPixel.setColor(trailPixel.color() | (1 << currentPixelEdge.sideNumber()));
        resultBitmap.pixel(pixel.x,pixel.y).setColor(255);
        if (currentPixelEdge.hasPixelOutside()) {
            resultBitmap.pixel(currentPixelEdge.pixelOutside().x, currentPixelEdge.pixelOutside().y).setColor(0);
        }
        currentPixelEdge= currentPixelEdge.nextBorder(minimumColor, maximumColor);
    }
    while (currentPixelEdge != startPixelEdge);
    }
    return true;
}
