#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <cstdint>
#include <unistd.h>
#include <getopt.h>
#include "util.h"
#include "entropyCoding.h"

using namespace std;

int main(int argc, char* argv[])
{
	cout << "start decoder!" << endl;
	std::string inputFileName, outputFileName, image_type;
	int32_t bitLevel = 16, transLevel = 3, color_channel = 1;
	int32_t height = 0, width = 0, tile_height = 0, tile_width = 0;
	int32_t opt;
    int32_t option_index = 0;
	bool lossless = true;
	bool reshape_flag = false;
	bool rearrange_flag = false;
	bool isHaar = false;
	// define long options
    static struct option long_options[] = {
        {"input-file", required_argument, nullptr, 'i'},
        {"output-file", required_argument, nullptr, 'o'},
        {"image-type", required_argument, nullptr, 't'},
        {"trans_level", required_argument, nullptr, 'l'},
        {"lossy", required_argument, nullptr, 's'},
        {"rearrange", required_argument, nullptr, 'g'},
        {"reshape", required_argument, nullptr, 'r'},
        {"isHaar", required_argument, nullptr, 'a'},
        {nullptr, 0, nullptr, 0}
    };
    while ((opt = getopt_long(argc, argv, "i:o:t:l:sgra", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                inputFileName = optarg;
                break;
            case 'o':
                outputFileName = optarg;
                break;
            case 't':
                image_type = optarg;
                break;
			case 'l':
				transLevel = std::stoi(optarg);
				break;
			case 's':
				lossless = false;
				break;
			case 'g':
				rearrange_flag = true;
				break;
			case 'r':
				reshape_flag = true;
				break;
			case 'a':
				isHaar = true;
				break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-i <imageFile>] [-o <outputFileName>] [-t <image_type>] [-l <transLevel>] [-s] [-g] [-r] [-a]" << std::endl;
		}
	}
	
	int startOffset = readHeightWidth(inputFileName, height, width, tile_height, tile_width);
	fstream inputFile(inputFileName, ios::in | ios::binary);
	inputFile.seekg(startOffset, ios::beg);
	BitInputStream inFileBin(inputFile);
	cout << std::left << std::setw(20) << "------" << image_type << "------" << endl; 
	cout << std::left << std::setw(20) << "lossless:" << lossless << endl;
	cout << std::left << std::setw(20) << "rearrange_flag:" << rearrange_flag << endl;
	cout << std::left << std::setw(20) << "reshape_flag:" << reshape_flag << endl;
	cout << std::left << std::setw(20) << "isHaar:" << isHaar << endl;
	cout << std::left << std::setw(20) << "transLevel:" << transLevel << endl;
	cout << std::left << std::setw(20) << "width:" << width << endl;
	cout << std::left << std::setw(20) << "height:" << height << endl;
	cout << std::left << std::setw(20) << "tile_width:" << tile_width << endl;
	cout << std::left << std::setw(20) << "tile_height:" << tile_height <<endl;
	
	if (image_type == "raw")
	{
		bool long_flag = true;
		bool chroma = false;
		std::vector<std::vector<int>> recImage = decodeChannel(width, height, tile_width, tile_height, startOffset, bitLevel, transLevel, image_type, inFileBin, lossless, rearrange_flag, reshape_flag, chroma, isHaar);
		saveImage(outputFileName, recImage, height, width, long_flag);
	}
	else if (image_type == "gray" or image_type == "YUV420_Y")
	{
		bool long_flag = false;
		bool chroma = false;
		std::vector<std::vector<int>> recImage = decodeChannel(width, height, tile_width, tile_height, startOffset, bitLevel, transLevel, image_type, inFileBin, lossless, rearrange_flag, reshape_flag, chroma, isHaar);
		saveImage(outputFileName, recImage, height, width, long_flag);
	}
	else if (image_type == "YUV420_UV")
	{
		bool long_flag = false;
		bool chroma = true;
		color_channel = 2;
		startOffset += height * width;
		width = width / 2;
		height = height / 2;
		if (tile_height != 1)
		{
			tile_height = tile_height / 2;
		}
		std::vector<std::vector<int>> recImage = decodeChannel(width, height, tile_width, tile_height, startOffset, bitLevel, transLevel, image_type, inFileBin, lossless, rearrange_flag, reshape_flag, chroma, isHaar);
		saveImage(outputFileName, recImage, height, width, long_flag, chroma);
	}
	else if (image_type == "YUV420")
	{
		bool long_flag = false;
		bool chroma = false;
		std::vector<std::vector<int>> recImage = decodeChannel(width, height, tile_width, tile_height, startOffset, bitLevel, transLevel, image_type, inFileBin, lossless, rearrange_flag, reshape_flag, chroma, isHaar);
		saveImage(outputFileName, recImage, height, width, long_flag);
		color_channel = 2;
		startOffset += height * width;
		width = width / 2;
		height = height / 2;
		chroma = true;
		if (tile_height != 1)
		{
			tile_height = tile_height / 2;
		}
		recImage = decodeChannel(width, height, tile_width, tile_height, startOffset, bitLevel, transLevel, image_type, inFileBin, lossless, rearrange_flag, reshape_flag, chroma, isHaar);
		saveImage(outputFileName, recImage, height, width, long_flag, chroma);
	}
	else{
		cout << "invalid image_type" << endl;
	}
	inFileBin.close();
	cout << "finish decoding"<< endl;
}


