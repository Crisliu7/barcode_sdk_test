/**
 * \file CommandLineBarcodeScannerImageProcessingSample.c
 *
 * \brief ScanditSDK demo application
 *
 * Takes a list of input images and directories (containing images) as argument.
 * The resulting input images are processed in reverse order by the barcode
 * scanner.
 *
 * This example is configured to achieve a good scan performance on a single image
 * (not a video stream). We assume that we have infinite processing power and no
 * real time requirements.
 *
 * \copyright Copyright (c) 2018 Scandit AG. All rights reserved.
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <dirent.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
// #include <SDL2/SDL_image.h>
#include <opencv2/imgcodecs.hpp>
#include <Scandit/ScRecognitionContext.h>
#include <Scandit/ScBarcodeScanner.h>

// Please insert your app key here:
#define SCANDIT_SDK_LICENSE_KEY "AeBshlxbQDdMJkyqEw7/QsYutjLMA6ygjFuVMVdYRBOHUH/IhXrBVZcXfLKadV8192jUw5d4WfVVBqDGt1Jy34VNejW+VMUo/GobAhMU2kOIIKjwohIiKb2lxMKXuYqYnIDWqtNyAq3zIwlXLZW8Wi/tvqZAanVAFS4hj1ZJVJQsz47DsOkHT+vw/+sUMUGIJpEVX2fO9rFlYsiO7QCNpOFLoiGdT33AtjXh924lAWaYjrV+navm+rl4V5LGRuJDbN3jXVsyDt25tTHPqNvsFkc6q4BV0Wq/lyNQ8IRmIySrcVLrcwaveS5KEwFzsHbQJFYFRPBNXzvTDIOmJR5Y1Q0T+0YjoaIAdorwd84DeCoFhJe8T/CWNDrC00Aj/pyc9330BpX12F1E+Rk/ARjpePYL3okWPQL+7fBKq1ZasnIDuZ8goa06A7zwI+hWt5vSeEAJHDL3BoBQWkPtbN+jwB/gRw4eOlFTYYE9leAvXfK5sHgBd1gpb1Mxat7Or+QUy0wcqXUEHQ5EsbDRSRirlbzxd2eUtDe6FVKLt6uj2QC5WCGm2jTEuC57ok7q4U5cZ/G0/d82e/+QjJ58oioH1NLfqcRbjuWxBUYsj75xpAY/nw7Ya+6A2PHnO+fNEw7USoiFpvQwf9rDeegGnSKNz/JVjJyUyhFoKC4J3OVA4IxqJu9kJVj6hM6+zRdj3QPq3pS7ldyMAYyZlbKSvN2wefCByKVt1ejz5WSE8LdF3PZEgJ8yztxiJ2ff0kxvaXOmgGrXhKMTDJWb3tJl2Zo15b5x4s/LNgI="

static char const * const ENABLED_FILE_EXTENSIONS[] = {
    "png",
    "jpg",
    "jpeg",
    "tif",
    "bmp",
    NULL
};

typedef struct InputImage {
    char const *file_name;
    struct InputImage const *next;
} InputImage;

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
        return SC_TRUE;
    }
  }

  return SC_FALSE;
}

static InputImage *push_if_valid_image(char const * filename, InputImage *container)
{
  if (has_valid_extension(filename) == SC_TRUE) 
  {
    InputImage *new_image = (InputImage *)malloc(sizeof(InputImage));
    const size_t target_size = strlen(filename) + 1;
    new_image->file_name = (char *)malloc(target_size);
    strncpy((char*)new_image->file_name, filename, target_size);

    new_image->next = container;

    container = new_image;
  }
  return container;
}

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
          char *combined_name = (char *)malloc(sizeof(current_arg) + sizeof(ent->d_name) + 2);
          combined_name[0] = '\0';
          strcat(combined_name, current_arg);
          strcat(combined_name, "/");
          strcat(combined_name, ent->d_name);
          // std::cout << "wen jian ming" << combined_name << std::endl;
          ret = push_if_valid_image(combined_name, ret);
        }
        closedir (dir);
      } 
      else 
      {
        // We have a file
        ret = push_if_valid_image(current_arg, ret);
      }
    }

    return ret;
}

/**
 * opencv read single image
 */
static ScBool read_image(const char* image_name, uint8_t **data, uint32_t *width, uint32_t *height, uint32_t *row_stride)
{
  cv::Mat image;
  const std::string image_name_str(image_name);
  image = cv::imread(image_name_str, cv::IMREAD_GRAYSCALE);
  if(image.empty())               
  {
    std::cout <<  "OpenCV could not open or find the image" << std::endl ;
    return SC_FALSE;
  }
  *data = image.data;
  *width = image.cols;
  *height = image.rows;
  *row_stride = image.step[0];

  const int blob_size = *row_stride * *height;

  // *data = (uint8_t *)malloc(blob_size);
  // memcpy(*data, image.data, blob_size);

  std::cout<< "Image: "<< image_name << ", size: " << *width << " * " << *height << ", stride " << *row_stride << "("<< blob_size << " bytes)" << std::endl;

  return SC_TRUE;
}

// /**
//  * Helper function to load image from disk using SDL2
//  */
// static ScBool load_image(const char* image_name, uint8_t** data,
//                          uint32_t* width, uint32_t *height,
//                          uint32_t* row_stride)
// {
//     SDL_Surface *image = IMG_Load(image_name);
//     if (image == NULL) {
//         printf("IMG_Load '%s' failed: %s\n", image_name, IMG_GetError());
//         return SC_FALSE;
//     }

//     SDL_Surface* image_rgb = SDL_ConvertSurfaceFormat(image,
//                                                       SDL_PIXELFORMAT_RGB24,
//                                                       0);
//     SDL_FreeSurface(image);
//     if (image_rgb == NULL) {
//         printf("Image '%s' convertion failed: %s\n", image_name, IMG_GetError());
//         return SC_FALSE;
//     }

//     *width = image_rgb->w;
//     *height = image_rgb->h;
//     *row_stride = image_rgb->pitch;
//     const int blob_size = *row_stride * *height;

//     printf("Image '%s' size: %ux%u, stride %u (%u bytes)\n", image_name, *width, *height,
//            *row_stride, blob_size);
//     *data = malloc(blob_size);
//     memcpy(*data, image_rgb->pixels, blob_size);

//     SDL_FreeSurface(image_rgb);

//     return SC_TRUE;
// }
void writeToFile(std::vector<float> time_vec, std::vector<std::string> result_vec, std::vector<std::string> type_vec, std::string file_name)
{
  int result_size = result_vec.size();
  int time_size = time_vec.size();
  int type_size = type_vec.size();
  if(result_size != time_size)
  {
    std::cout << "The result vector size is not equal to time vector size !!! cannot write to csv file" << std::endl; 
    return;
  }
  std::ofstream ofs (file_name, std::ofstream::out);
  for(int i = 0; i < result_size; i++)
  {
    if(i == result_size - 1)
    {
      ofs << result_vec[i];
      break;
    }
    ofs << result_vec[i] << '\t';
  }
  ofs << '\n';
  for(int i = 0; i < time_size; i++)
  {
    if(i == time_size - 1)
    {
      ofs << time_vec[i];
      break;
    }
    ofs << time_vec[i] << '\t';
  }
  ofs << '\n';
  for(int i = 0; i < type_size; i++)
  {
    if(i == type_size - 1)
    {
      ofs << type_vec[i];
      break;
    }
    ofs << type_vec[i] << '\t';
  }
  ofs.close();
}

int main(int argc, const char *argv[])
{
  if (argc < 2) 
  {
    printf("Please provide paths to image files or directories as arguments.\n");
    return 0;
  }
  printf("Scandit SDK Version: %s\n", SC_VERSION_STRING);

  // get file name
  std::string file_name = "scandit";
  // std::cout << argv[1] << std::endl;
  std::vector<std::string> suffix_vec;
  char * token;
  char * str = strdup(argv[1]);
  token = strtok(str, "/");
  while(token != NULL){
      suffix_vec.push_back(std::string(token));
      // the call is treated as a subsequent calls to strtok:
      // the function continues from where it left in previous invocation
      token = strtok(NULL, "/");
  }
  std::reverse(suffix_vec.begin(), suffix_vec.end()); 
  suffix_vec.erase(suffix_vec.begin() + 3, suffix_vec.end());
  std::reverse(suffix_vec.begin(), suffix_vec.end()); 
  for(auto it = suffix_vec.begin(); it != suffix_vec.end(); it++){
    file_name = file_name + "_" + *it;
  }



  int return_code = 0;
  float fCostTime = 0;
  std::vector<std::string> result_vector;
  std::vector<float> time_vector;
  std::vector<std::string> type_vector;

  ScRecognitionContext *context = NULL;
  ScBarcodeScanner *scanner = NULL;
  ScImageDescription *image_descr = NULL;
  ScBarcodeScannerSettings *settings = NULL;
  // uint8_t *image_data = NULL;

  InputImage const * const images = get_input_files(argc, argv);

  // Create a recognition context. Files created by the recognition context and the 
  // attached scanners should be written to /tmp. You can use any other writable data 
  // directory in your application.
  context = sc_recognition_context_new(SCANDIT_SDK_LICENSE_KEY, "/tmp", NULL);
  if (context == NULL) {
    printf("Could not initialize context.\n");
    return 0;
  }


  image_descr = sc_image_description_new();
  if (image_descr == NULL) {
    printf("Could not initialize image description.\n");
    return 0;
  }

  // The barcode scanner is configured by setting the appropriate properties on an 
  // "barcode scanner settings" instance. This settings object is passed to the barcode 
  // scanner when it is constructed. We start with the settings preset for single frame processing
  // and enable only the symbologies we need. For the purpose of this demo, we would like to scan
  // EAN13/UPCA and QR codes
  settings = sc_barcode_scanner_settings_new_with_preset(SC_PRESET_ENABLE_SINGLE_FRAME_MODE);
  if (settings == NULL) {
    printf("Could not initialize settings.\n");
    return 0;
  }

  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_UPCA, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_UPCE, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_EAN8, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_EAN13, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_CODE39, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_CODE93, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_CODE128, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_CODABAR , SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_INTERLEAVED_2_OF_5, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_QR, SC_TRUE);
  sc_barcode_scanner_settings_set_symbology_enabled(settings, SC_SYMBOLOGY_DATA_MATRIX, SC_TRUE);

  
  // setting: enable variable length setting for ITF
  ScSymbologySettings * itf_settings = sc_barcode_scanner_settings_get_symbology_settings(settings, SC_SYMBOLOGY_INTERLEAVED_2_OF_5); 
  uint16_t active_counts[] = {26};
  uint16_t num_counts = 1; 
  sc_symbology_settings_set_active_symbol_counts(itf_settings, active_counts, num_counts);

  // setting: remove leading zero of upca
  ScSymbologySettings * upca_settings = sc_barcode_scanner_settings_get_symbology_settings(settings, SC_SYMBOLOGY_UPCA); 
  sc_symbology_settings_set_extension_enabled(upca_settings, "remove_leading_zero", SC_TRUE);

  // Set the maximum number of codes to look for in an image, 1 in our case.
  sc_barcode_scanner_settings_set_max_number_of_codes_per_frame(settings, 1);

  // By setting the code location constraints to ignore, we tell
  // the barcode scanner to search for codes in the whole image in every frame.
  sc_barcode_scanner_settings_set_code_location_constraint_1d(settings, SC_CODE_LOCATION_IGNORE);
  sc_barcode_scanner_settings_set_code_location_constraint_2d(settings, SC_CODE_LOCATION_IGNORE);

  // We make no assumptions about the most likely orientation of the codes.
  sc_barcode_scanner_settings_set_code_direction_hint(settings, SC_CODE_DIRECTION_NONE);

  // The barcode scanner allows to prevent codes from getting scanned again in
  // a certain time interval (e.g., 500ms). The default setting is 0 what
  // effectively disables this duplicate filtering.
  //sc_barcode_scanner_settings_set_code_duplicate_filter(settings, 500);

  // Create a barcode scanner for our context and settings.
  scanner = sc_barcode_scanner_new_with_settings(context, settings);
  if (scanner == NULL) 
  {
    printf("Could not initialize scanner.\n");
    return 0;
  }

  // Wait for the initialization of the barcode scanner. We could omit this call
  // and start scanning immediately, but there is no guarantee that the barcode scanner 
  // operates at full capacity.
  if (!sc_barcode_scanner_wait_for_setup_completed(scanner)) 
  {
    printf("barcode scanner setup failed.\n");
    return 0;
  }

  for (InputImage const *current_image = images; current_image != NULL; current_image = current_image->next) 
  {
    // opencv read the image from disc.
    uint32_t image_width, image_height, row_stride;
    // if (read_image(current_image->file_name, &image_data, &image_width, &image_height, &row_stride) == SC_FALSE) {
    //   printf("Failed to load image '%s'.\n", current_image->file_name);
    //   return_code = -1;
    //   // goto cleanup;
    // }
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

    // begin time
    struct timeval ullTimeBegin, ullTimeEnd;
    gettimeofday(&ullTimeBegin, NULL);  

    // Fill the image description for our loaded image.
    const uint32_t image_memory_size = row_stride * image_height;
    sc_image_description_set_layout(image_descr, SC_IMAGE_LAYOUT_RGB_8U);
    sc_image_description_set_width(image_descr, image_width);
    sc_image_description_set_height(image_descr, image_height);
    sc_image_description_set_first_plane_row_bytes(image_descr, row_stride);
    sc_image_description_set_memory_size(image_descr, image_memory_size);

    // Signal to the context that a new sequence of frames starts. This call is mandatory,
    // even if we are only going to process one image. Scanning will fail with
    // SC_RECOGNITION_CONTEXT_STATUS_FRAME_SEQUENCE_NOT_STARTED otherwise.
    sc_recognition_context_start_new_frame_sequence(context);

    ScProcessFrameResult result = sc_recognition_context_process_frame(context, image_descr, cv_image.data);
    if (result.status != SC_RECOGNITION_CONTEXT_STATUS_SUCCESS) {
      printf("Processing frame failed with error %d: '%s'\n", result.status,
              sc_context_status_flag_get_message(result.status));
      return 0;
    }

    // Signal to the context that the frame sequence is finished.
    sc_recognition_context_end_frame_sequence(context);

    // Retrieve the barcode scanner object to get the list of codes that were recognized in
    // the last frame.
    ScBarcodeScannerSession *session = sc_barcode_scanner_get_session(scanner);

    // Get the list of codes that have been found in the last process frame call.
    ScBarcodeArray * new_codes = sc_barcode_scanner_session_get_newly_recognized_codes(session);

    // ?????? calculate decoding time cost ??????
    gettimeofday(&ullTimeEnd, NULL);
    fCostTime = (float)((ullTimeEnd.tv_sec * 1000 * 1000 +  ullTimeEnd.tv_usec) - (ullTimeBegin.tv_sec * 1000 * 1000 + ullTimeBegin.tv_usec))/(1000 * 1000);
    time_vector.push_back(fCostTime);

    uint32_t num_codes = sc_barcode_array_get_size(new_codes);
    if (num_codes == 0) 
    {
      std::cout << "    No barcode found. Total time spent: " << fCostTime << " seconds."<< std::endl;
      result_vector.push_back("");
      type_vector.push_back("");
    }
    else
    {
      std::cout << "    Total barcode(s) found: " << num_codes << ". Total time spent: " << fCostTime << std::endl;
      for (uint32_t iIndex = 0; iIndex < num_codes; iIndex++)
      {
        const ScBarcode * barcode = sc_barcode_array_get_item_at(new_codes, iIndex);
        ScByteArray data = sc_barcode_get_data(barcode);
        ScSymbology symbology = sc_barcode_get_symbology(barcode);
        const char *symbology_name = sc_symbology_to_string(symbology);
        // For simplicity it is assumed that the barcode contains textual data, even
        // though it is possible to encode binary data in QR codes that contain null-
        // bytes at any position. For applications expecting binary data, use
        // sc_byte_array_get_data_size() to determine the length of the returned data.

        // printf("barcode: symbology=%s, data='%s'\n", symbology_name, data.str);
        std::cout << "        Barcode " << iIndex + 1 << ":" << std::endl;
        std::cout << "            Type " << symbology_name << std::endl;
        std::cout << "            Value " << data.str << std::endl;
        result_vector.push_back(data.str);
        type_vector.push_back(symbology_name);
      }
    }


    sc_barcode_array_release(new_codes);
    cv_image.release();
    // free(image_data);
    // image_data = NULL;

  }
  // write to csv file

  writeToFile(time_vector, result_vector, type_vector, file_name + "_result.csv");

  return 0;
}
