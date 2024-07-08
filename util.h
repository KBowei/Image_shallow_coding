
#ifndef __UTIL__
#define __UTIL__

#include <vector>
#include <string>
#include "entropyCoding.h"

const std::vector<std::vector<int>> reshapeTile(const std::vector<std::vector<int32_t>>& tile, int width, int height);

int readHeightWidth(std::string file, int &height, int &width, int &tile_height, int &tile_width);

int writeHeightWidth(std::string file, int const height, int const width, int const tile_height, int const tile_width);

void readImage(std::vector<std::vector<int32_t>> &image, std::string fileName, int height, int width, std::streampos startOffset, bool long_flag);

std::vector<std::vector<int32_t>> gridMap(const std::vector<std::vector<int>>& tile);

std::vector<std::vector<int32_t>> gridMapInverse(const std::vector<std::vector<int>>& tile);

int calculateTileNum(int height, int width, int tile_height, int tile_width);

std::vector<std::vector<std::vector<int32_t>>> splitImageIntoTiles(const std::vector<std::vector<int>>& inputArray, int sliceHeight, int sliceWidth);

std::vector<std::vector<int>> mergeTileIntoImage(const std::vector<std::vector<std::vector<int>>>& tiles, int imageHeight, int imageWidth);

const std::vector<std::vector<int32_t>> paddingImage(const std::vector<std::vector<int32_t>> &image, int padWidth, int padHeight);

const std::vector<std::vector<int>> cropImage(const std::vector<std::vector<int>>& image, int cropWidth, int cropHeight);

void saveImage(std::string file, std::vector<std::vector<int32_t>> const& reconstructImage, int height, int width, bool long_flag, bool chroma=false);

void haarForwardTrans1D(std::vector<std::vector<int32_t>> const &image, std::vector<std::vector<int32_t>> &waveletCoeff, int32_t width, int32_t height);

void haarInverseTrans1D(std::vector<std::vector<int32_t>> const &waveletCoeff, std::vector<std::vector<int32_t>> &image, int32_t width, int32_t height);

void haarForwardTrans(std::vector<std::vector<int32_t>> const &image, std::vector<std::vector<int32_t>> &waveletCoeff, int32_t width, int32_t height);

void haarInverseTrans(std::vector<std::vector<int32_t>> const &waveletCoeff, std::vector<std::vector<int32_t>> &image, int32_t width, int32_t height);

void CDF53ForwardTrans1D(std::vector<std::vector<int32_t>> const &image, std::vector<std::vector<int32_t>> &waveletCoeff, int32_t width, int32_t height);

void CDF53InverseTrans1D(std::vector<std::vector<int32_t>> const &waveletCoeff, std::vector<std::vector<int32_t>> &image, int32_t width, int32_t height);

void CDF53ForwardTrans(std::vector<std::vector<int32_t>> const &image, std::vector<std::vector<int32_t>> &waveletCoeff, int32_t width, int32_t height);

void CDF53InverseTrans(std::vector<std::vector<int32_t>> const &waveletCoeff, std::vector<std::vector<int32_t>> &image, int32_t width, int32_t height);

void forwardTrans(std::vector<std::vector<int32_t>> const &image, std::vector<std::vector<int32_t>> &waveletCoeff, int32_t transLevel, int isHaar=0);

void inverseTrans(std::vector<std::vector<int32_t>> const &waveletCoeff, std::vector<std::vector<int32_t>> &image, int32_t transLevel, int isHaar=0);

void entropyCoding(std::vector<std::vector<int32_t>> const waveletCoeff, BitOutputStream &outFileBin, int bitLevel, int transLevel, bool lossless);

void entropyDecoding(BitInputStream &inFileBin, std::vector<std::vector<int32_t>> &waveletCoeff, int bitLevel, int transLevel, bool lossless);

void encodeChannel(int32_t width, int32_t height, int32_t tile_width, int32_t tile_height, int32_t startOffset, int32_t bitLevel, int32_t transLevel, std::string image_type, std::string imageFile, BitOutputStream &outFileBin, bool lossless, bool rearrange_flag, bool reshape_flag, bool long_flag, bool chroma=false, bool isHaar=false);

std::vector<std::vector<int>> decodeChannel(int32_t width, int32_t height, int32_t tile_width, int32_t tile_height, int32_t startOffset, int32_t bitLevel, int32_t transLevel, std::string image_type, BitInputStream &inFileBin, bool lossless, bool rearrange_flag, bool reshape_flag, bool chroma=false, bool isHaar=false);
#endif // !UTIL
