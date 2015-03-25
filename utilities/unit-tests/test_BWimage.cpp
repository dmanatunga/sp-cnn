#include "BWImage.h"
#include <string>
#include <assert.h>

int main(int argc, char* argv[]) 
{
  BWImage bw_img;
  
  assert(argc == 4);
  string filename(argv[1]);
  string delimiter(argv[2]);  
  string filename2(argv[3]);

  bw_img.dlmread(filename, delimiter);
  bw_img.dlmwrite(filename2, delimiter);
}
   

