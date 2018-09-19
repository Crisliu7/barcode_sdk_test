#include "Tools.h" 

Tools::Tools()
{

}
Tools::~Tools()
{

}


// static char const * const ENABLED_FILE_EXTENSIONS[] = {
//     "png",
//     "jpg",
//     "jpeg",
//     "tif",
//     "bmp",
//     NULL
// };

// bool Tools::has_valid_extension(char const *file_name)
// {
//   char const * const *extension = ENABLED_FILE_EXTENSIONS;
//   for (; *extension != NULL; ++extension) 
//   {
//     const size_t file_name_length = strlen(file_name);
//     const size_t extension_length = strlen(*extension);

//     char const * const file_extension = file_name + (file_name_length - extension_length);
//     if (file_name_length >= extension_length && strcmp(file_extension, *extension) == 0) 
//     {
//       return true;
//     }
//   }

//   return false;
// }

void Tools::generate_filename(char const *argv[], std::string &file_name)
{
   // generate file name for final writing csv
  // std::string file_name = "scandit";
  // std::cout << argv[1] << std::endl;
  std::vector<std::string> suffix_vec;
  char * token;
  char * str = strdup(argv[1]);
  token = strtok(str, "/");
  while(token != NULL)
  {
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
}

void Tools::get_input_files(int argc, char const *argv[])
{
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
        if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
        {
          char *combined_name = (char *)malloc(sizeof(current_arg) + sizeof(ent->d_name) + 2);
          combined_name[0] = '\0';
          strcat(combined_name, current_arg);
          strcat(combined_name, "/");
          strcat(combined_name, ent->d_name);
          // if (has_valid_extension(combined_name) == true)
          // {
          file_name_vec.push_back(combined_name);
          // }
        }

      }
      std::sort(file_name_vec.begin(), file_name_vec.end());
      closedir (dir);
    } 
    else 
    {
      // We have a file
      // if (has_valid_extension(current_arg) == true)
      // {
      file_name_vec.push_back(current_arg);
      // }
    }
  }

  // return file_name_vec;
}


/**
 * opencv read single image
 */
// bool Tools::read_image(const char* image_name, uint8_t **data, uint32_t *width, uint32_t *height, uint32_t *row_stride)
bool Tools::read_image(std::string image_name)

{
  cv::Mat cv_image;
  // const std::string image_name_str(image_name);
  cv_image = cv::imread(image_name, cv::IMREAD_GRAYSCALE);
  if(cv_image.empty())               
  {
    std::cout <<  "OpenCV could not open or find the image" << std::endl ;
    return false;
  }
  // data = cv_image.data;
  // std::cout << "value: " << *data << std ::endl; 

  // std::cout << data << std::endl; 

  image_width = cv_image.cols;
  image_height = cv_image.rows;
  row_stride = cv_image.step[0];
  blob_size = row_stride * image_height;

  data = (uint8_t *)malloc(blob_size);
  memcpy(data, cv_image.data, blob_size);

  // std::cout<< "Image: "<< image_name << ", size: " << *width << " * " << *height << ", stride " << *row_stride << "("<< blob_size << " bytes)" << std::endl;
  cv_image.release();

  return true;
}

void Tools::writeToFile(std::string file_name)
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




