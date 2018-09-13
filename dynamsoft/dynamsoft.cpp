/**
************************************************************************
This C++ sample is developed using the C++ API of Dynamsoft Barcode Reader.
The sample demonstrates how to read barcode from the Linux Command Line.
*************************************************************************
*/

#include <dirent.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <sys/time.h>
#include <SDL2/SDL_image.h>
#include "include/DynamsoftBarcodeReader.h"

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

    char const * const file_extension = file_name + (file_name_length - extension_length);
    if (file_name_length >= extension_length && strcmp(file_extension, *extension) == 0) 
    {
      return true;
    }
  }
  return true;
}

static InputImage *push_if_valid_image(char const * filename,
                                      InputImage *container)
{
  if (has_valid_extension(filename) == true) 
  {
    InputImage *new_image = (InputImage *)malloc(sizeof(InputImage));
    const size_t target_size = strlen(filename) + 1;
    new_image->file_name = (char*)malloc(target_size);
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
        char *combined_name = (char*)malloc(sizeof(current_arg) +
                                      sizeof(ent->d_name) + 2);
        combined_name[0] = '\0';
        strcat(combined_name, current_arg);
        strcat(combined_name, "/");
        strcat(combined_name, ent->d_name);

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
 * Helper function to load image from disk using SDL2
 */
static bool load_image(const char* image_name, uint8_t** data,
                         uint32_t* width, uint32_t *height,
                         uint32_t* row_stride)
{
  SDL_Surface *image = IMG_Load(image_name);
  if (image == NULL) {
      printf("IMG_Load '%s' failed: %s\n", image_name, IMG_GetError());
      return false;
  }

  SDL_Surface* image_rgb = SDL_ConvertSurfaceFormat(image,
                                                    SDL_PIXELFORMAT_RGB24,
                                                    0);
  SDL_FreeSurface(image);
  if (image_rgb == NULL) {
      printf("Image '%s' convertion failed: %s\n", image_name, IMG_GetError());
      return false;
  }

  *width = image_rgb->w;
  *height = image_rgb->h;
  *row_stride = image_rgb->pitch;
  const int blob_size = *row_stride * *height;

  printf("Image '%s' size: %ux%u, stride %u (%u bytes)\n", image_name, *width, *height,
          *row_stride, blob_size);
  *data = (uint8_t *)malloc(blob_size);
  memcpy(*data, image_rgb->pixels, blob_size);

  SDL_FreeSurface(image_rgb);

  return true;
}




void OutputResult(CBarcodeReader& reader,int errorcode,float time,char*pszTextResult,int pszTextResultLength)
{
	char * pszTemp1 = NULL;
	char* pszTempResult = NULL;
	int iRet = errorcode;

	if (iRet != DBR_OK && iRet != DBRERR_LICENSE_EXPIRED && iRet != DBRERR_QR_LICENSE_INVALID &&
		iRet != DBRERR_1D_LICENSE_INVALID && iRet != DBRERR_PDF417_LICENSE_INVALID && iRet != DBRERR_DATAMATRIX_LICENSE_INVALID && iRet != DBRERR_AZTEC_LICENSE_INVALID)
	{
		snprintf(pszTextResult, pszTextResultLength, "Failed to read barcode: %s\r\n", CBarcodeReader::GetErrorString(iRet));
		return;
	}

	STextResultArray *paryResult = NULL;
	reader.GetAllTextResults(&paryResult);

	if (paryResult->nResultsCount == 0)
	{
		snprintf(pszTextResult, pszTextResultLength, "No barcode found. Total time spent: %.3f seconds.\r\n", time);
		CBarcodeReader::FreeTextResults(&paryResult);
		return;
	}

	snprintf(pszTextResult, pszTextResultLength, "Total barcode(s) found: %d. Total time spent: %.3f seconds\r\n\r\n", paryResult->nResultsCount, time);
	for (int iIndex = 0; iIndex < paryResult->nResultsCount; iIndex++)
	{
		pszTempResult = (char*)malloc(4096);
		snprintf(pszTempResult, 2048, "Barcode %d:\r\n    Type: %s\r\n    Value: %s\r\n", iIndex + 1, paryResult->ppResults[iIndex]->pszBarcodeFormatString, paryResult->ppResults[iIndex]->pszBarcodeText);

		strcat(pszTextResult,pszTempResult);
		free(pszTempResult);
		free(pszTemp1);
	}

	CBarcodeReader::FreeTextResults(&paryResult);
	return;
}

int main(int argc, const char* argv[])
{
  int iRet = -1;
	int iExitFlag = 0;
	char szErrorMsg[256];
	float fCostTime = 0;


	CBarcodeReader reader;
	reader.InitLicense("t0068NQAAAFjNxhQOFJImz3ElX+DrOBwwfNaQP3KXFlzWlGtuNW5B2oy6uPJA1TB6o3hxT9qOeEkhIXeG19lt5VVLTENk41s=");

	if(argc<=1)
	{
		printf("Usage: ReadBarcode [ImageFilePath]\n");
		return 1; 
	}
  // load image file/folder
  InputImage const * const images = get_input_files(argc, argv);
  uint8_t *image_data = NULL;


  // traverse loaded image
  for (InputImage const *current_image = images; current_image != NULL; current_image = current_image->next)
  {
    uint32_t image_width, image_height, row_stride;
    if (load_image(current_image->file_name, &image_data, &image_width, &image_height, &row_stride) == false)
    {
        printf("Failed to load image '%s'.\n", current_image->file_name);
    }

    // decode and calculate time cost
	  struct timeval ullTimeBegin, ullTimeEnd;
    gettimeofday(&ullTimeBegin, NULL);    
    // iRet = reader.DecodeFile(current_image->file_name);
    iRet = reader.DecodeBuffer(image_data, image_width, image_height, row_stride, IPF_RGB_888, "");
    gettimeofday(&ullTimeEnd, NULL);
    fCostTime = (float)((ullTimeEnd.tv_sec * 1000 * 1000 +  ullTimeEnd.tv_usec) - (ullTimeBegin.tv_sec * 1000 * 1000 + ullTimeBegin.tv_usec))/(1000 * 1000);

    if(iRet !=DBR_OK)
    {
      printf("Error code: %d. Error message: %s\n", iRet, szErrorMsg);
    }

    // output barcode result.
    char * pszTextResult = NULL;
    pszTextResult = (char*)malloc(8192);
    OutputResult(reader, iRet, fCostTime, pszTextResult, 8192);
    printf(pszTextResult);
    free(pszTextResult);


  }

 	return 0;
}
