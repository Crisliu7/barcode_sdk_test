/**
************************************************************************
This C++ sample is developed using the C++ API of Dynamsoft Barcode Reader.
The sample demonstrates how to read barcode from the Linux Command Line.
*************************************************************************
*/

#include <dirent.h>
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <fstream>

// #include <vector>
#include <string>

#include <opencv2/imgcodecs.hpp>

#include "include/DynamsoftBarcodeReader.h"
#include "../Tools.h"



int main(int argc, const char* argv[])
{
  if (argc < 2) 
	{
		printf("Usage: ReadBarcode [ImageFilePath]\n");
		return -1; 
	}
  
  int iRet = -1;
	char szErrorMsg[256];

  // create tools object
  Tools dynamsoftTools;
  // generate file name for final writing csv
  std::string file_name = "dynamsoft";
  dynamsoftTools.generate_filename(argv, file_name);
  // load all image file's name
  dynamsoftTools.get_input_files(argc, argv);



  // initialize reader
	CBarcodeReader reader;
	reader.InitLicense("t0068NQAAAFjNxhQOFJImz3ElX+DrOBwwfNaQP3KXFlzWlGtuNW5B2oy6uPJA1TB6o3hxT9qOeEkhIXeG19lt5VVLTENk41s=");
  // reader.InitLicense("t0068MgAAAIl+c7nncpOUUJwhgX7nKryqYnqU/dnX2EC+WLcRUiegYAYnlLdh4l6mKrugNC1JQabojs+yfgUYhJqQ17qpH7o=");

  // reader settings
  PublicRuntimeSettings *settings;
  reader.GetRuntimeSettings(settings);
  // settings -> mExpectedBarcodesCount = 1;
  // settings -> mMaxBarcodesCount = 2;
  // settings -> mAntiDamageLevel = 9;
  // reader.UpdateRuntimeSettings(settings);




  // traverse loaded image
  for (std::string image_name : dynamsoftTools.file_name_vec)
  {
    // opencv read the image from disc.
    if( dynamsoftTools.read_image(image_name) == false)
    {
      std::cout << "Failed to load image " << image_name << std::endl;
      return 0;
    }

    std::cout << "Image: "<< image_name<< ", size: " << dynamsoftTools.image_width << " * " << dynamsoftTools.image_height
               << ", stride " << dynamsoftTools.row_stride << "("<< dynamsoftTools.blob_size << " bytes)" << std::endl;


    // decode and calculate time cost
	  struct timeval ullTimeBegin, ullTimeEnd;
    gettimeofday(&ullTimeBegin, NULL);
    // const char *image_name_char = image_name.c_str();     
    // iRet = reader.DecodeFile(image_name_char);
    iRet = reader.DecodeBuffer(dynamsoftTools.data, dynamsoftTools.image_width, dynamsoftTools.image_height, dynamsoftTools.row_stride, IPF_GrayScaled, "");
    gettimeofday(&ullTimeEnd, NULL);
    dynamsoftTools.fCostTime = (float)((ullTimeEnd.tv_sec * 1000 * 1000 +  ullTimeEnd.tv_usec) - (ullTimeBegin.tv_sec * 1000 * 1000 + ullTimeBegin.tv_usec))/(1000 * 1000);


    if(iRet !=DBR_OK)
    {
      printf("Error code: %d. Error message: %s\n", iRet, szErrorMsg);
      return 0;
    }

    // output decoding result.
    if (iRet != DBR_OK && iRet != DBRERR_LICENSE_EXPIRED && iRet != DBRERR_QR_LICENSE_INVALID &&
      iRet != DBRERR_1D_LICENSE_INVALID && iRet != DBRERR_PDF417_LICENSE_INVALID && iRet != DBRERR_DATAMATRIX_LICENSE_INVALID && iRet != DBRERR_AZTEC_LICENSE_INVALID)
    {
      std::cout << "Failed to read barcode: \r\n" << CBarcodeReader::GetErrorString(iRet) << std::endl;
      return 0;
    }

    STextResultArray *paryResult = NULL;
    reader.GetAllTextResults(&paryResult);

    dynamsoftTools.time_vec.push_back(dynamsoftTools.fCostTime);
    if (paryResult->nResultsCount == 0)
    {
      std::cout << "    No barcode found. Total time spent: " << dynamsoftTools.fCostTime << " seconds."<< std::endl;
      dynamsoftTools.result_vec.push_back("");
      dynamsoftTools.type_vec.push_back("");
    }
    else
    {
      std::cout << "    Total barcode(s) found: " << paryResult->nResultsCount << ". Total time spent: " << dynamsoftTools.fCostTime << std::endl;
      for (int iIndex = 0; iIndex < paryResult->nResultsCount; iIndex++)
      {
        std::cout << "        Barcode " << iIndex + 1 << ":" << std::endl;
        std::cout << "            Type " << paryResult->ppResults[iIndex]->pszBarcodeFormatString << std::endl;
        std::cout << "            Value " << paryResult->ppResults[iIndex]->pszBarcodeText << std::endl;
        dynamsoftTools.result_vec.push_back(paryResult->ppResults[iIndex]->pszBarcodeText);
        dynamsoftTools.type_vec.push_back(paryResult->ppResults[iIndex]->pszBarcodeFormatString);
        break;
      }
    }
    CBarcodeReader::FreeTextResults(&paryResult);
    free(dynamsoftTools.data);

  }

  // write to csv file
  dynamsoftTools.writeToFile("data/" + file_name + "_result.csv");
  return 0;
}

