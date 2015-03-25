#ifndef BW_IMAGE_H
#define BW_IMAGE_H

#include <string>
using namespace std;

class BWImage {
  private:
    float* _image;
    size_t _R;
    size_t _C;

  public:
    BWImage();
    BWImage(size_t R, size_t C);
    ~BWImage();


    float* getImage();
    void dlmread(string filename, string delimiter);
    void dlmwrite(string filename, string delimiter);

#define THRESHOLD 0.0001f
    bool compare(BWImage* img);
    unsigned numPixelsDiff(BWImage* img);
    
    size_t R() { return _R; }
    size_t C() { return _C; }
};

#endif // BW_IMAGE_H
