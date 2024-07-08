#include <fstream>
#include <iostream>
#include <math.h>
#include <cstdint>
#include <unistd.h>
#include <getopt.h>
#include <iomanip>
#include "util.h"
#include "entropyCoding.h"

using namespace std;

int32_t main(int argc, char *argv[])
{
	cout << "start encoder!" << endl;
	std::string imageFile, outputFileName, image_type;
    int32_t width, height, tile_width, tile_height, transLevel;
	int32_t bitLevel = 16; // less than 32
	int32_t color_channel = 1; // 1, 2 or 3
	int32_t total_bits = 0;
	int32_t opt;
    int32_t option_index = 0;
	bool lossless = true;
	bool rearrange_flag = false;
	bool reshape_flag = false;
	bool isHaar = false;
    // define long options
    static struct option long_options[] = {
        {"input-file", required_argument, nullptr, 'i'},
        {"output-file", required_argument, nullptr, 'o'},
        {"image-type", required_argument, nullptr, 't'},
        {"trans_level", required_argument, nullptr, 'l'},
        {"width", required_argument, nullptr, 'w'},
        {"height", required_argument, nullptr, 'h'},
        {"tile-width", required_argument, nullptr, 'x'},
        {"tile-height", required_argument, nullptr, 'y'},
        {"lossy", required_argument, nullptr, 's'},
        {"rearrange", required_argument, nullptr, 'g'},
        {"reshape", required_argument, nullptr, 'r'},
        {"isHaar", required_argument, nullptr, 'a'},
        {nullptr, 0, nullptr, 0}
    };
    while ((opt = getopt_long(argc, argv, "i:o:t:l:w:h:x:y:sgra", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                imageFile = optarg;
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
            case 'w':
                width = std::stoi(optarg);
                break;
            case 'h':
                height = std::stoi(optarg);
                break;
            case 'x':
                tile_width = std::stoi(optarg);
                break;
            case 'y':
                tile_height = std::stoi(optarg);
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
                std::cerr << "Usage: " << argv[0] << " [-i <imageFile>] [-o <outputFileName>] [-t <image_type>] [-l <transLevel>] [-w <width>] [-h <height>] [-x <tile_width>] [-y <tile_height>] [--input-file <imageFile>] [--output-file <outputFileName>] [--image-type <image_type>] [--width <width>] [--height <height>] [--tile-width <tile_width>] [--tile-height <tile_height>] [-s] [-g] [-r] [-a]" << std::endl;
		}
	}
	cout << "------" << image_type << "------" << endl; 
	cout << std::left << std::setw(20) << "image:" << imageFile << endl; 
	cout << std::left << std::setw(20) << "lossless:" << lossless << endl;
	cout << std::left << std::setw(20) << "rearrange_flag:" << rearrange_flag << endl;
	cout << std::left << std::setw(20) << "reshape_flag:" << reshape_flag << endl;
	cout << std::left << std::setw(20) << "isHaar:" << isHaar << endl;
	cout << std::left << std::setw(20) << "transLevel:" << transLevel << endl;
	cout << std::left << std::setw(20) << "width:" << width << endl;
	cout << std::left << std::setw(20) << "height:" << height << endl;
	cout << std::left << std::setw(20) << "tile_width:" << tile_width << endl;
	cout << std::left << std::setw(20) << "tile_height:" << tile_height <<endl;
	int32_t startOffset = writeHeightWidth(outputFileName, height, width, tile_height, tile_width); // write (height, width, tileheight, tilewidth) to bitstream
	fstream outputFile(outputFileName, ios::out | ios::binary | ios::app);
	outputFile.seekg(startOffset, ios::beg);
	BitOutputStream outFileBin(outputFile);
	startOffset = 0;
	if (image_type == "raw")
	{
		bool long_flag = true;
		bool chroma = false;
		encodeChannel(width, height, tile_width, tile_height, startOffset, bitLevel, transLevel, image_type, imageFile, outFileBin, lossless, rearrange_flag, reshape_flag, long_flag, chroma, isHaar);
	}
	else if (image_type == "gray" or image_type == "YUV420_Y")
	{
		bool long_flag = false;
		bool chroma = false;
		encodeChannel(width, height, tile_width, tile_height, startOffset, bitLevel, transLevel, image_type, imageFile, outFileBin, lossless, rearrange_flag, reshape_flag, long_flag, chroma, isHaar);
	}
	else if (image_type == "YUV420_UV")
	{
		bool long_flag = false;
		bool chroma = true;
		// read uv channel only
		color_channel = 2;
		startOffset += height * width;
		width = width / 2;
		height = height / 2;
		if (tile_height != 1){
			tile_height = tile_height / 2;
		}
		encodeChannel(width, height, tile_width, tile_height, startOffset, bitLevel, transLevel, image_type, imageFile, outFileBin, lossless, rearrange_flag, reshape_flag, long_flag, chroma, isHaar);
	}
	else if (image_type == "YUV420")
	{
		bool long_flag = false;
		bool chroma = false;
		// encode Y channel first
		encodeChannel(width, height, tile_width, tile_height, startOffset, bitLevel, transLevel, image_type, imageFile, outFileBin, lossless, rearrange_flag, reshape_flag, long_flag, chroma, isHaar);
		// encode uv channel 
		color_channel = 2;
		startOffset += height * width;
		width = width / 2;
		height = height / 2;
		chroma = true;
		if (tile_height != 1){
			tile_height = tile_height / 2;
		}
		encodeChannel(width, height, tile_width, tile_height, startOffset, bitLevel, transLevel, image_type, imageFile, outFileBin, lossless, rearrange_flag, reshape_flag, long_flag, chroma, isHaar);
	}
	else{
		cout << "invalid image_type" << endl;
	}
	
	outFileBin.close();
	total_bits = outFileBin.countBit;
	cout << "finish coding" << endl;
	cout << std::left << std::setw(20) << "total bits:" << total_bits << endl;
	return 0;
}