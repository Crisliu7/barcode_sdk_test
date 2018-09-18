
// #define STR(s) #s


#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
// #include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
// #include <opencv2/highgui/highgui.hpp>
#include <dirent.h>
#include <sys/time.h>
#include <zbar.h>



// valid image extension
static char const * const ENABLED_FILE_EXTENSIONS[] = {
  "png",
  "jpg",
  "jpeg",
  "tif",
  "bmp",
  NULL
};

// image container
typedef struct InputImage {
    char const *file_name;
    struct InputImage const *next;
} InputImage;

/**
 * check if image extension is valid
 */
static int has_valid_extension(char const *file_name)
{
  char const * const *extension = ENABLED_FILE_EXTENSIONS;
  for (; *extension != NULL; ++extension) 
  {
    const size_t file_name_length = strlen(file_name);
    const size_t extension_length = strlen(*extension);

    char const * const file_extension =
            file_name + (file_name_length - extension_length);
    if (file_name_length >= extension_length && strcmp(file_extension, *extension) == 0) 
    {
      return true;
    }
  }

  return false;
}

/**
 * push images to image container
 */
static InputImage *push_if_valid_image(char const * filename, InputImage *container)
{
  if (has_valid_extension(filename) == true) 
  {
    InputImage *new_image = (InputImage*)malloc(sizeof(InputImage));
    const size_t target_size = strlen(filename) + 1;
    new_image->file_name = (char*)malloc(target_size);
    strncpy((char*)new_image->file_name, filename, target_size);

    new_image->next = container;

    container = new_image;
  }

  return container;
}

/**
 * load images from file or folder
 */
InputImage const *get_input_files(int argc, char const *argv[])
{
  InputImage *ret = NULL;

  // We skip the first argument
  for (int arg_idx = 1; arg_idx < argc; ++arg_idx) 
  {
    char const * const current_arg = argv[arg_idx];
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(current_arg)) != NULL) 
    {
      // We have a directory
      while ((ent = readdir (dir)) != NULL) 
      {
        char *combined_name = (char*)malloc(sizeof(current_arg) + sizeof(ent->d_name) + 2);
        combined_name[0] = '\0';
        strcat(combined_name, current_arg);
        strcat(combined_name, "/");
        strcat(combined_name, ent->d_name);

        ret = push_if_valid_image(combined_name, ret);
      }
      closedir (dir);
    } else {
      // We have a file
      ret = push_if_valid_image(current_arg, ret);
    }
  }

  return ret;
}

/**
 * opencv read single image
 */
// static bool read_image(const char* image_name, uint8_t **data, uint32_t *width, uint32_t *height, uint32_t *row_stride)
// {

//   cv::Mat image;
//   const std::string image_name_str(image_name);
//   image = cv::imread(image_name_str, cv::IMREAD_GRAYSCALE);
//   if(image.empty())               
//   {
//     std::cout <<  "OpenCV could not open or find the image" << std::endl ;
//     return false;
//   }
//   // *data = image.data;
//   *width = image.cols;
//   *height = image.rows;
//   *row_stride = image.step[0];

//   const int blob_size = *row_stride * *height;

//   *data = (uint8_t *)malloc(blob_size);
//   memcpy(*data, image.data, blob_size);

//   std::cout<< "Image: "<< image_name << ", size: " << *width << " * " << *height << ", stride " << *row_stride << "("<< blob_size << " bytes)" << std::endl;


//   return true;
// }
void writeToFile(std::vector<float> time_vec, std::vector<std::string> result_vec, std::vector<std::string> type_vec)
{
  int result_size = result_vec.size();
  int time_size = time_vec.size();
  int type_size = type_vec.size();
  if(result_size != time_size)
  {
    std::cout << "The result vector size is not equal to time vector size !!! cannot write to csv file" << std::endl; 
    return;
  }
  std::ofstream ofs ("zbar_result.csv", std::ofstream::out);
  for(int i = 0; i < result_size; i++)
  {
    ofs << result_vec[i] << '\t';
  }
  ofs << '\n';
  for(int i = 0; i < time_size; i++)
  {
    ofs << time_vec[i] << '\t';
  }
  ofs << '\n';
  for(int i = 0; i < type_size; i++)
  {
    ofs << type_vec[i] << '\t';
  }
  ofs.close();
}



int main(int argc, const char* argv[])
{
	if(argc <= 1)
	{
		printf("Usage: ReadBarcode [ImageFilePath]\n");
		return 0; 
	}

  float fCostTime = 0;
  std::vector<std::string> result_vector;
  std::vector<float> time_vector;
  std::vector<std::string> type_vector;

  // create a reader
  zbar::ImageScanner scanner;

  // configure the reader
  scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

  // load image file/folder
  InputImage const * const images = get_input_files(argc, argv);
  // uint8_t *image_data = NULL;

  // traverse loaded image
  for (InputImage const *current_image = images; current_image != NULL; current_image = current_image->next)
  {
    // opencv read image
    uint32_t image_width, image_height, row_stride;
    cv::Mat cv_image;
    const std::string image_name_str(current_image->file_name);
    cv_image = cv::imread(image_name_str, cv::IMREAD_GRAYSCALE);
    if(cv_image.empty())               
    {
      std::cout <<  "OpenCV could not open or find the image " << image_name_str << std::endl ;
      return 0;
    }
    image_width = cv_image.cols;
    image_height = cv_image.rows;
    row_stride = cv_image.step[0];
    const int blob_size = row_stride * image_height;
    std::cout<< "Image: "<< image_name_str << ", size: " << image_width << " * " << image_height << ", stride " << row_stride << "("<< blob_size << " bytes)" << std::endl;

    
    // decode and calculate time cost
	  struct timeval ullTimeBegin, ullTimeEnd;
    gettimeofday(&ullTimeBegin, NULL);    
    // wrap image data
    zbar::Image zbar_image(image_width, image_height, "GREY", cv_image.data, row_stride);
    // scan the image for barcodes
    int n = scanner.scan(zbar_image);
    gettimeofday(&ullTimeEnd, NULL);
    fCostTime = (float)((ullTimeEnd.tv_sec * 1000 * 1000 +  ullTimeEnd.tv_usec) - (ullTimeBegin.tv_sec * 1000 * 1000 + ullTimeBegin.tv_usec))/(1000 * 1000);
    time_vector.push_back(fCostTime);



    if (zbar_image.symbol_begin() == zbar_image.symbol_end()) 
    {
      std::cout << "    No barcode found. Total time spent: " << fCostTime << " seconds."<< std::endl;
      result_vector.push_back("");
      type_vector.push_back("");
    }
    else
    {
      std::cout << "    Total barcode(s) found: " << n << ". Total time spent: " << fCostTime << std::endl;
      // output decoding results
      int iIndex = 0;
      for(zbar::Image::SymbolIterator symbol = zbar_image.symbol_begin(); symbol != zbar_image.symbol_end(); ++symbol) 
      {
        std::cout << "        Barcode " << iIndex + 1 << ":" << std::endl;
        std::cout << "            Type: " << symbol->get_type_name() << std::endl;
        std::cout << "            Value: " << symbol->get_data() << std::endl;
        result_vector.push_back(symbol->get_data());
        type_vector.push_back(symbol->get_type_name());
        iIndex++;
      }
    }
    // clean up
    zbar_image.set_data(NULL, 0);

  }
  // write to csv file
  writeToFile(time_vector, result_vector, type_vector);

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