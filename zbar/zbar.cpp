
// #define STR(s) #s


// #include <cstdio>
#include <cstdlib>
// #include <cstdint>
// #include <string>
// #include <iostream>
// #include <fstream>
// #include <opencv2/core/core.hpp>
// #include <opencv2/imgcodecs.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <dirent.h>
#include <sys/time.h>
#include <zbar.h>
#include "../Tools.h"

int main(int argc, const char* argv[])
{
	if(argc < 2)
	{
		printf("Usage: ReadBarcode [ImageFilePath]\n");
		return 0; 
	}

  // create tools object
  Tools zbarTools;
  // generate file name for final writing csv
  std::string file_name = "zbar";
  zbarTools.generate_filename(argv, file_name);
  // load all image file's name
  zbarTools.get_input_files(argc, argv);


  // create a reader
  zbar::ImageScanner scanner;

  // configure the reader
  scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);



  // traverse loaded image
  for (std::string image_name : zbarTools.file_name_vec)
  {
    // opencv read the image from disc.
    if( zbarTools.read_image(image_name) == false)
    {
      std::cout << "Failed to load image " << image_name << std::endl;
      return 0;
    }

    std::cout << "Image: "<< image_name<< ", size: " << zbarTools.image_width << " * " << zbarTools.image_height
               << ", stride " << zbarTools.row_stride << "("<< zbarTools.blob_size << " bytes)" << std::endl;

    
    // decode and calculate time cost
	  struct timeval ullTimeBegin, ullTimeEnd;
    gettimeofday(&ullTimeBegin, NULL);    
    // wrap image data
    zbar::Image zbar_image(zbarTools.image_width, zbarTools.image_height, "GREY", zbarTools.data, zbarTools.row_stride);
    // scan the image for barcodes
    int n = scanner.scan(zbar_image);
    gettimeofday(&ullTimeEnd, NULL);
    zbarTools.fCostTime = (float)((ullTimeEnd.tv_sec * 1000 * 1000 +  ullTimeEnd.tv_usec) - (ullTimeBegin.tv_sec * 1000 * 1000 + ullTimeBegin.tv_usec))/(1000 * 1000);
    zbarTools.time_vec.push_back(zbarTools.fCostTime);


  


    if (zbar_image.symbol_begin() == zbar_image.symbol_end()) 
    {
      std::cout << "    No barcode found. Total time spent: " << zbarTools.fCostTime << " seconds."<< std::endl;
      zbarTools.result_vec.push_back("");
      zbarTools.type_vec.push_back("");
    }
    else
    {
      std::cout << "    Total barcode(s) found: " << n << ". Total time spent: " << zbarTools.fCostTime << std::endl;
      // output decoding results
      int iIndex = 0;
      for(zbar::Image::SymbolIterator symbol = zbar_image.symbol_begin(); symbol != zbar_image.symbol_end(); ++symbol) 
      {
        std::cout << "        Barcode " << iIndex + 1 << ":" << std::endl;
        std::cout << "            Type: " << symbol->get_type_name() << std::endl;
        std::cout << "            Value: " << symbol->get_data() << std::endl;
        zbarTools.result_vec.push_back(symbol->get_data());
        zbarTools.type_vec.push_back(symbol->get_type_name());
        iIndex++;
      }
    }
    // clean up
    zbar_image.set_data(NULL, 0);
    free(zbarTools.data);


  }
  // write to csv file
  zbarTools.writeToFile(file_name);

  return 0;
}




/**
 * Helper function to load image from disk using SDL2
 */
// static bool read_image(const char* image_name, uint8_t** data,
//                          uint32_t* width, uint32_t *height,
//                          uint32_t* row_stride)
// {
//   SDL_Surface *image = IMG_Load(image_name);
//   if (image == NULL) {
//       printf("IMG_Load '%s' failed: %s\n", image_name, IMG_GetError());
//       return false;
//   }

//   SDL_Surface* image_rgb = SDL_ConvertSurfaceFormat(image,
//                                                     SDL_PIXELFORMAT_RGB24,
//                                                     0);
//   SDL_FreeSurface(image);
//   if (image_rgb == NULL) {
//       printf("Image '%s' convertion failed: %s\n", image_name, IMG_GetError());
//       return false;
//   }

//   *width = image_rgb->w;
//   *height = image_rgb->h;
//   *row_stride = image_rgb->pitch;
//   const int blob_size = *row_stride * *height;

//   printf("Image '%s' size: %ux%u, stride %u (%u bytes)\n", image_name, *width, *height,
//           *row_stride, blob_size);
//   *data = (uint8_t *)malloc(blob_size);
//   memcpy(*data, image_rgb->pixels, blob_size);

//   SDL_FreeSurface(image_rgb);

//   return true;
// }