#ifndef TOOLS_H
#define TOOLS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <dirent.h>
#include <stdio.h>
#include <assert.h>
// #include <sys/time.h>
// #include <SDL2/SDL_image.h>
#include <opencv2/imgcodecs.hpp>

class Tools{
public:


  Tools();
  ~Tools();

  void generate_filename(char const *argv[], std::string &file_name);


  void get_input_files(int argc, char const *argv[]);

  bool read_image(std::string image_name);

  void writeToFile(std::string file_name);

  std::vector<std::string> file_name_vec;

  uint32_t image_width, image_height, row_stride, blob_size;
  // cv::Mat cv_image;
  uint8_t *data;
  float fCostTime = 0;
  std::vector<std::string> result_vec;
  std::vector<float> time_vec;
  std::vector<std::string> type_vec;
  



};

#endif