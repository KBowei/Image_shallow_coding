#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include "util.h"
#include "entropyCoding.h"

using namespace std;

const std::vector<std::vector<int32_t>> reshapeTile(const std::vector<std::vector<int32_t>> &tile, int width, int height)
{
	int pre_height = tile.size();
	int pre_width = tile[0].size();
	if (pre_height * pre_width != height * width)
	{
		// cout << "element num mismatch" << endl;
		std::vector<std::vector<int32_t>> reshapedTile = tile;
	}
	std::vector<std::vector<int32_t>> reshapedTile(height, std::vector<int32_t>(width, 0));
	int cur_height = 0, cur_width = 0;
	for (int i = 0; i < pre_height; i++)
	{
		for (int j = 0; j < pre_width; j++)
		{
			reshapedTile[cur_height][cur_width] = tile[i][j];
			cur_width++;
			if (cur_width == width)
			{
				cur_height++;
				cur_width = 0;
			}
		}
	}
	return reshapedTile;
}

int readHeightWidth(std::string file, int &height, int &width, int &tile_height, int &tile_width)
{
	int offset = 8;
	// 先读低8bit，再读高8bit
	fstream inFile(file, ios::in | ios::binary);
	unsigned char tmpWidth, tmpHeight, tmptileWidth, tmptileHeight;
	inFile.read((char *)&tmpHeight, 1);
	inFile.read((char *)&tmpWidth, 1);
	inFile.read((char *)&tmptileHeight, 1);
	inFile.read((char *)&tmptileWidth, 1);
	height = tmpHeight;
	width = tmpWidth;
	tile_height = tmptileHeight;
	tile_width = tmptileWidth;
	inFile.read((char *)&tmpHeight, 1);
	inFile.read((char *)&tmpWidth, 1);
	inFile.read((char *)&tmptileHeight, 1);
	inFile.read((char *)&tmptileWidth, 1);
	height = (tmpHeight << 8) + height;
	width = (tmpWidth << 8) + width;
	tile_height = (tmptileHeight << 8) + tile_height;
	tile_width = (tmptileWidth << 8) + tile_width;
	inFile.close();
	return offset;
}

int writeHeightWidth(std::string file, int const height, int const width, int const tile_height, int const tile_width)
{
	int offset = 8;
	// 依次写入低8bit和高8bit数据，超过16bit需要额外处理
	fstream outFile(file, ios::out | ios::binary);
	unsigned char tmpHeight = height, tmpWidth = width, tmptileHeight = tile_height, tmptileWidth = tile_width;
	outFile.write((const char *)&tmpHeight, 1);
	outFile.write((const char *)&tmpWidth, 1);
	outFile.write((const char *)&tmptileHeight, 1);
	outFile.write((const char *)&tmptileWidth, 1);

	tmpHeight = height >> 8, tmpWidth = width >> 8, tmptileHeight = tile_height >> 8, tmptileWidth = tile_width >> 8;
	outFile.write((const char *)&tmpHeight, 1);
	outFile.write((const char *)&tmpWidth, 1);
	outFile.write((const char *)&tmptileHeight, 1);
	outFile.write((const char *)&tmptileWidth, 1);
	outFile.close();
	return offset;
}

void readImage(vector<vector<int32_t>> &image, std::string fileName, int height, int width, std::streampos startOffset, bool long_flag)
{
	fstream inFile(fileName, ios::in | ios::binary);

	if (!inFile)
	{
		cout << "read image fail!";
		exit(-1);
	}
	inFile.seekg(startOffset, std::ios::beg);
	for (int32_t h = 0; h < height; h++)
	{
		for (int32_t w = 0; w < width; w++)
		{
			if (long_flag){
				inFile.read((char *)(&image[h][w]), sizeof(uint16_t));
			}
			else{
				inFile.read((char *)(&image[h][w]), sizeof(uint8_t));
			}
		}
	}
	if (inFile.eof())
	{
		cout << "Reach the end of file" << endl;
	}
	else
	{
		streampos cur_pos = inFile.tellg();
		inFile.seekg(0, std::ios::end);
		streampos file_size = inFile.tellg();
		streamsize remaining_bytes = file_size - cur_pos;
		cout << "There remains " << remaining_bytes << " bytes" << endl;
	}
	inFile.close();
}

std::vector<std::vector<int32_t>> gridMap(const std::vector<std::vector<int>>& tile)
{
	int tileHeight = tile.size();
	int tileWidth = tile[0].size();
	std::vector<std::vector<int32_t>> gridMappedTile(tileHeight, std::vector<int32_t>(tileWidth, 0));
	if (tileHeight != 1)
	{
		for (int k=0; k<4; k++)
		{
			for (int i = k/2; i < tileHeight; i += 2){
				for (int j = k%2; j < tileWidth; j += 2){
					// gridMappedTile[(i-k/2)/2 + (k/2)*tileHeight/2][(j-k%2)/2 + (k%2)*tileWidth/2] = tile[i][j];
					if (k == 0){
						gridMappedTile[i/2][j/2] = tile[i][j];
					}
					else if (k == 1){
						gridMappedTile[i/2][(j-1)/2 + tileWidth/2] = tile[i][j];
					}
					else if (k == 2){
						gridMappedTile[(i-1)/2 + tileHeight/2][j/2 + tileWidth/2] = tile[i][j];
					}
					else{
						gridMappedTile[(i-1)/2 + tileHeight/2][(j-1)/2] = tile[i][j];
					}
				}
			}
		}
	}
	else
	{
		for (int k=0; k<2; k++)
		{
			for (int j = k%2; j < tileWidth; j += 2){
				// gridMappedTile[(i-k/2)/2 + (k/2)*tileHeight/2][(j-k%2)/2 + (k%2)*tileWidth/2] = tile[i][j];
				if (k == 0){
					gridMappedTile[0][j/2] = tile[0][j];
				}
				else if (k == 1){
					gridMappedTile[0][(j-1)/2 + tileWidth/2] = tile[0][j];
				}
			}	
		}
	}
	
	return gridMappedTile;
}

std::vector<std::vector<int32_t>> gridMapInverse(const std::vector<std::vector<int>>& tile)
{
	int tileHeight = tile.size();
	int tileWidth = tile[0].size();
	std::vector<std::vector<int32_t>> gridMapInversedTile(tileHeight, std::vector<int32_t>(tileWidth, 0));
	if (tileHeight != 1)
	{
		for (int k=0; k<4; k++)
		{
			for (int i = k/2; i < tileHeight; i += 2){
				for (int j = k%2; j < tileWidth; j += 2){
					if (k == 0){
						gridMapInversedTile[i][j] = tile[i/2][j/2];
					}
					else if (k == 1){
						gridMapInversedTile[i][j] = tile[i/2][(j-1)/2 + tileWidth/2];
					}
					else if (k == 2){
						gridMapInversedTile[i][j] = tile[(i-1)/2 + tileHeight/2][j/2 + tileWidth/2];
					}
					else{
						gridMapInversedTile[i][j] = tile[(i-1)/2 + tileHeight/2][(j-1)/2];
					}
				}
			}
		}
	}
	else
	{
		for (int k=0; k<2; k++)
		{
			for (int j = k%2; j < tileWidth; j += 2){
				// gridMappedTile[(i-k/2)/2 + (k/2)*tileHeight/2][(j-k%2)/2 + (k%2)*tileWidth/2] = tile[i][j];
				if (k == 0){
					gridMapInversedTile[0][j] = tile[0][j/2];
				}
				else if (k == 1){
					gridMapInversedTile[0][j] = tile[0][(j-1)/2 + tileWidth/2];
				}
			}	
		}
	}
	return gridMapInversedTile;
}

int calculateTileNum(int height, int width, int tile_height, int tile_width) {
    int num_tiles_height = ceil(static_cast<double>(height) / tile_height);
    int num_tiles_width = ceil(static_cast<double>(width) / tile_width);
    return num_tiles_height * num_tiles_width;
}

vector<vector<vector<int32_t>>> splitImageIntoTiles(const vector<vector<int>> &image, int tileHeight, int tileWidth)
{
	int height = image.size();
	int width = image[0].size();
	vector<vector<vector<int>>> tiles;
	for (int row = 0; row < height; row += tileHeight)
	{
		for (int col = 0; col < width; col += tileWidth)
		{
			vector<vector<int>> tile;
			for (int i = row; i < row + tileHeight && i < height; ++i)
			{
				vector<int> rowBlock;
				for (int j = col; j < col + tileWidth && j < width; ++j)
				{
					rowBlock.push_back(image[i][j]);
				}
				tile.push_back(rowBlock);
			}
			tiles.push_back(tile);
		}
	}

	return tiles;
}

std::vector<std::vector<int>> mergeTileIntoImage(const std::vector<std::vector<std::vector<int>>>& tiles, int imageHeight, int imageWidth) {
    const int tile_num = tiles.size();
	const int tileHeight = tiles[0].size();
    const int tileWidth = tiles[0][0].size();
	const int tile_rows = ceil(float(imageHeight) / tileHeight);
	const int tile_cols = ceil(float(imageWidth) / tileWidth);
    std::vector<std::vector<int>> image(imageHeight, std::vector<int>(imageWidth, 0));

    // merge tiles into image
    for (int i = 0; i < tiles.size(); ++i) {
        int cur_row = int(i / tile_cols);
		int cur_col = int(i % tile_cols);
		// judge the last row or the last col
		int curTileHeight = (cur_row == tile_rows - 1) ? imageHeight % tileHeight : tileHeight;
        int curTileWidth = (cur_col == tile_cols - 1) ? imageWidth % tileWidth : tileWidth;
		if (curTileHeight == 0) {
			curTileHeight = tileHeight;
		}
		if (curTileWidth == 0) {
			curTileWidth = tileWidth;
		}
		for (int r = 0; r < curTileHeight; ++r) {
            for (int c = 0; c < curTileWidth; ++c) {
                image[cur_row * tileHeight + r][cur_col * tileWidth + c] = tiles[i][r][c];
			}
    	}
	}
    return image;
}

const std::vector<std::vector<int32_t>> paddingImage(const std::vector<std::vector<int32_t>> &image, int padWidth, int padHeight)
{
	int width = image[0].size(), height = image.size();
	vector<vector<int32_t>> paddedImage(padHeight, vector<int32_t>(padWidth, 0));
	// copy image into padimage
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			paddedImage[h][w] = image[h][w];
		}
	}
	for (int h = 0; h < height; h++)
	{
		for (int w = width; w < padWidth; w++)
		{
			paddedImage[h][w] = image[h][width - 1];
		}
	}
	for (int h = height; h < padHeight; h++)
	{
		for (int w = 0; w < padWidth; w++)
		{
			paddedImage[h][w] = paddedImage[height - 1][w];
		}
	}
	return paddedImage;
}

const std::vector<std::vector<int32_t>> cropImage(const std::vector<std::vector<int>>& image, int cropHeight, int cropWidth)
{
    std::vector<std::vector<int32_t>> croppedImage(cropHeight, std::vector<int>(cropWidth));
    for (int i = 0; i < cropHeight; ++i) {
        for (int j = 0; j < cropWidth; ++j) {
            croppedImage[i][j] = image[i][j];
        }
    }
    return croppedImage;
}

void saveImage(std::string file, std::vector<std::vector<int32_t>> const &reconstructImage, int height, int width, bool long_flag, bool chroma)
{
	fstream outFile(file, ios::out | ios::binary);
	if (!outFile)
	{
		cout << "save image fail!";
		exit(-1);
	}
	if (!chroma)
	{
		// reconstructImage.height = height; reconstructImage.width = width
		for (int32_t h = 0; h < height; h++)
		{
			for (int32_t w = 0; w < width; w++)
			{
				if (long_flag){
					outFile.write((char *)(&reconstructImage[h][w]), sizeof(uint16_t));
				}
				else{
					outFile.write((char *)(&reconstructImage[h][w]), sizeof(uint8_t));
				}
			}
		}
	}
	else
	{
		// reconstructImage.heigth = height; reconstructImage.width = 2*width
		// save u_channel first
		for (int32_t h = 0; h < height; h++)
		{
			for (int32_t w = 0; w < width; w++)
			{
				if (long_flag){
					outFile.write((char *)(&reconstructImage[h][2*w]), sizeof(uint16_t));
				}
				else{
					outFile.write((char *)(&reconstructImage[h][2*w]), sizeof(uint8_t));
				}
			}
		}
		// save v_channel
		for (int32_t h = 0; h < height; h++)
		{
			for (int32_t w = 0; w < width; w++)
			{
				if (long_flag){
					outFile.write((char *)(&reconstructImage[h][2*w+1]), sizeof(uint16_t));
				}
				else{
					outFile.write((char *)(&reconstructImage[h][2*w+1]), sizeof(uint8_t));
				}
			}
		}
	}
	outFile.close();
}

void haarForwardTrans1D(std::vector<std::vector<int32_t>> const &image, std::vector<std::vector<int32_t>> &waveletCoeff, int32_t width, int32_t height)
{
	if (height != 1)
	{
		cout << "invalid shape for 1D WT!" << endl;
		exit(-1);
	}
	vector<vector<int32_t>> tempcoloum(height, vector<int32_t>(width, 0));
	//  transform coloum
	for (int32_t w = 0; w < width / 2; w++)
	{
		tempcoloum[0][w + width / 2] = image[0][w * 2 + 1] - image[0][w * 2];
		tempcoloum[0][w] = image[0][w * 2] + tempcoloum[0][w + width / 2] / 2;
	}
	for (int32_t w = 0; w < width ; w++)
	{
		waveletCoeff[0][w] = tempcoloum[0][w];
	}
}

void haarInverseTrans1D(std::vector<std::vector<int32_t>> const &waveletCoeff, std::vector<std::vector<int32_t>> &image, int32_t width, int32_t height)
{
	if (height != 1)
	{
		cout << "invalid shape for 1D WT!" << endl;
		exit(-1);
	}
	// transform coloum
	vector<vector<int32_t>> tempcoloum(height, vector<int32_t>(width, 0));
	for (int32_t w = 0; w < width / 2; w++)
	{
		tempcoloum[0][2 * w] = waveletCoeff[0][w] - waveletCoeff[0][w + width / 2] / 2;
		tempcoloum[0][2 * w + 1] = tempcoloum[0][2 * w] + waveletCoeff[0][w + width / 2];
	}
	for (int32_t w = 0; w < width ; w++)
	{
		image[0][w] = tempcoloum[0][w];
	}
}

void haarForwardTrans(std::vector<std::vector<int32_t>> const &image, std::vector<std::vector<int32_t>> &waveletCoeff, int32_t width, int32_t height)
{
	// row transform
	vector<vector<int32_t>> tempRow(height, vector<int32_t>(width, 0));
	for (int32_t h = 0; h < height / 2; h++)
	{
		for (int32_t w = 0; w < width; w++)
		{
			tempRow[height / 2 + h][w] = image[h * 2 + 1][w] - image[2 * h][w];
			tempRow[h][w] = image[2 * h][w] + tempRow[height / 2 + h][w] / 2;
		}
	}

	//  transform coloum
	for (int32_t h = 0; h < height; h++)
	{
		for (int32_t w = 0; w < width / 2; w++)
		{
			waveletCoeff[h][w + width / 2] = tempRow[h][w * 2 + 1] - tempRow[h][w * 2];
			waveletCoeff[h][w] = tempRow[h][w * 2] + waveletCoeff[h][w + width / 2] / 2;
		}
	}
}

void haarInverseTrans(std::vector<std::vector<int32_t>> const &waveletCoeff, std::vector<std::vector<int32_t>> &image, int32_t width, int32_t height)
{
	// transform coloum
	vector<vector<int32_t>> tempcoloum(height, vector<int32_t>(width, 0));
	for (int32_t h = 0; h < height; h++)
	{
		for (int32_t w = 0; w < width / 2; w++)
		{
			tempcoloum[h][2 * w] = waveletCoeff[h][w] - waveletCoeff[h][w + width / 2] / 2;
			tempcoloum[h][2 * w + 1] = tempcoloum[h][2 * w] + waveletCoeff[h][w + width / 2];
		}
	}

	// transform row
	for (int32_t h = 0; h < height / 2; h++)
	{
		for (int32_t w = 0; w < width; w++)
		{
			image[2 * h][w] = tempcoloum[h][w] - tempcoloum[h + height / 2][w] / 2;
			image[2 * h + 1][w] = image[2 * h][w] + tempcoloum[h + height / 2][w];
		}
	}
}

// 1D 53wavelet transform
void CDF53ForwardTrans1D(std::vector<std::vector<int32_t>> const &image, std::vector<std::vector<int32_t>> &waveletCoeff, int32_t width, int32_t height)
{
	if (height != 1)
	{
		cout << "invalid shape for 1D WT!" << endl;
		exit(-1);
	}
	vector<vector<int32_t>> tempcoloum(height, vector<int32_t>(width, 0));
	for (int32_t w = 0; w < width / 2; w++)
	{
		tempcoloum[0][width / 2 + w] = image[0][2 * w + 1] - image[0][2 * w] / 2 - image[0][w * 2 + 2 < width ? w * 2 + 2 : 2 * w] / 2;
	}
	for (int32_t w = 0; w < width / 2; w++)
	{
		tempcoloum[0][w] = image[0][2 * w] + tempcoloum[0][w - 1 >= 0 ? width / 2 + w - 1 : width / 2 + w] / 4 + tempcoloum[0][width / 2 + w] / 4;
	}
	for (int32_t w = 0; w < width; w++)
	{
		waveletCoeff[0][w] = tempcoloum[0][w];
	}
}

void CDF53InverseTrans1D(std::vector<std::vector<int32_t>> const &waveletCoeff, std::vector<std::vector<int32_t>> &image, int32_t width, int32_t height)
{
	vector<vector<int32_t>> tempcoloum(height, vector<int32_t>(width, 0));
	if (height != 1)
	{
		cout << "invalid shape for 1D WT!" << endl;
		exit(-1);
	}
	for (int32_t w = 0; w < width / 2; w++)
	{
		tempcoloum[0][2 * w] = waveletCoeff[0][w] - waveletCoeff[0][w - 1 >= 0 ? width / 2 + w - 1 : width / 2 + w] / 4 - waveletCoeff[0][w + width / 2] / 4;
	}
	for (int32_t w = 0; w < width / 2; w++)
	{
		tempcoloum[0][2 * w + 1] = waveletCoeff[0][w + width / 2] + tempcoloum[0][w * 2] / 2 + tempcoloum[0][w * 2 + 2 < width ? w * 2 + 2 : w * 2] / 2;
	}
	for (int32_t w = 0; w < width ; w++)
	{
		image[0][w] = tempcoloum[0][w];
	}
}

void CDF53ForwardTrans(std::vector<std::vector<int32_t>> const &image, std::vector<std::vector<int32_t>> &waveletCoeff, int32_t width, int32_t height)
{
	// row transform
	vector<vector<int32_t>> tempRow(height, vector<int32_t>(width, 0));
	for (int32_t h = 0; h < height / 2; h++)
	{
		for (int32_t w = 0; w < width; w++)
		{
			tempRow[height / 2 + h][w] = image[2 * h + 1][w] - image[h * 2][w] / 2 - image[h * 2 + 2 < height ? h * 2 + 2 : h * 2][w] / 2;
		}
	}
	for (int32_t h = 0; h < height / 2; h++)
	{
		for (int32_t w = 0; w < width; w++)
		{
			tempRow[h][w] = image[2 * h][w] + tempRow[h - 1 >= 0 ? height / 2 + h - 1 : height / 2 + h][w] / 4 + tempRow[height / 2 + h][w] / 4;
		}
	}

	//  transform coloum
	for (int32_t h = 0; h < height; h++)
	{
		for (int32_t w = 0; w < width / 2; w++)
		{
			waveletCoeff[h][w + width / 2] = tempRow[h][w * 2 + 1] - tempRow[h][w * 2] / 2 - tempRow[h][w * 2 + 2 < width ? w * 2 + 2 : w * 2] / 2;
		}
	}
	for (int32_t h = 0; h < height; h++)
	{
		for (int32_t w = 0; w < width / 2; w++)
		{
			waveletCoeff[h][w] = tempRow[h][w * 2] + waveletCoeff[h][w - 1 >= 0 ? width / 2 + w - 1 : width / 2 + w] / 4 + waveletCoeff[h][w + width / 2] / 4;
		}
	}
}

void CDF53InverseTrans(std::vector<std::vector<int32_t>> const &waveletCoeff, std::vector<std::vector<int32_t>> &image, int32_t width, int32_t height)
{
	// transform coloum
	vector<vector<int32_t>> tempcoloum(height, vector<int32_t>(width, 0));
	for (int32_t h = 0; h < height; h++)
	{
		for (int32_t w = 0; w < width / 2; w++)
		{
			tempcoloum[h][2 * w] = waveletCoeff[h][w] - waveletCoeff[h][w - 1 >= 0 ? width / 2 + w - 1 : width / 2 + w] / 4 - waveletCoeff[h][w + width / 2] / 4;
		}
	}
	for (int32_t h = 0; h < height; h++)
	{
		for (int32_t w = 0; w < width / 2; w++)
		{
			tempcoloum[h][2 * w + 1] = waveletCoeff[h][w + width / 2] + tempcoloum[h][w * 2] / 2 + tempcoloum[h][w * 2 + 2 < width ? w * 2 + 2 : w * 2] / 2;
		}
	}

	// transform row
	for (int32_t h = 0; h < height / 2; h++)
	{
		for (int32_t w = 0; w < width; w++)
		{
			image[2 * h][w] = tempcoloum[h][w] - tempcoloum[h - 1 >= 0 ? height / 2 + h - 1 : height / 2 + h][w] / 4 - tempcoloum[height / 2 + h][w] / 4;
		}
	}
	for (int32_t h = 0; h < height / 2; h++)
	{
		for (int32_t w = 0; w < width; w++)
		{
			image[2 * h + 1][w] = tempcoloum[height / 2 + h][w] + image[h * 2][w] / 2 + image[h * 2 + 2 < height ? h * 2 + 2 : h * 2][w] / 2;
		}
	}
}

void forwardTrans(std::vector<std::vector<int32_t>> const &image, std::vector<std::vector<int32_t>> &waveletCoeff, int32_t transLevel, int32_t isHaar)
{
	if (isHaar == 1)
	{
		if (image.size() != 1)
		{
			haarForwardTrans(image, waveletCoeff, waveletCoeff[0].size(), waveletCoeff.size());
			for (int32_t i = 1; i < transLevel; i++)
			{
				haarForwardTrans(waveletCoeff, waveletCoeff, waveletCoeff[0].size() / pow(2, i), waveletCoeff.size() / pow(2, i));
			}
		}
		else
		{
			haarForwardTrans1D(image, waveletCoeff, waveletCoeff[0].size(), waveletCoeff.size());
			for (int32_t i = 1; i < transLevel; i++)
			{
				haarForwardTrans1D(waveletCoeff, waveletCoeff, waveletCoeff[0].size() / pow(2, i), waveletCoeff.size());
			}
		}
	}
	else
	{
		if (image.size() != 1)
		{
			CDF53ForwardTrans(image, waveletCoeff, waveletCoeff[0].size(), waveletCoeff.size());
			for (int32_t i = 1; i < transLevel; i++)
			{
				CDF53ForwardTrans(waveletCoeff, waveletCoeff, waveletCoeff[0].size() / pow(2, i), waveletCoeff.size() / pow(2, i));
			}
		}
		else
		{
			CDF53ForwardTrans1D(image, waveletCoeff, waveletCoeff[0].size(), waveletCoeff.size());
			for (int32_t i = 1; i < transLevel; i++)
			{
				CDF53ForwardTrans1D(waveletCoeff, waveletCoeff, waveletCoeff[0].size() / pow(2, i), waveletCoeff.size());
			}
		}
	}	
}

void inverseTrans(std::vector<std::vector<int32_t>> const &waveletCoeff, std::vector<std::vector<int32_t>> &image, int32_t transLevel, int32_t isHaar)
{
	for (int32_t h = 0; h < waveletCoeff.size(); h++)
	{
		for (int32_t w = 0; w < waveletCoeff[0].size(); w++)
		{
			image[h][w] = waveletCoeff[h][w];
		}
	}
	if (isHaar == 1)
	{
		if (waveletCoeff.size() != 1){
			haarInverseTrans(image, image, image[0].size() / pow(2, transLevel - 1), image.size() / pow(2, transLevel - 1));
			for (int32_t i = transLevel - 2; i >= 0; i--)
			{
				haarInverseTrans(image, image, image[0].size() / pow(2, i), image.size() / pow(2, i));
			}
		}
		else{
			haarInverseTrans1D(image, image, image[0].size() / pow(2, transLevel - 1), image.size());
			for (int32_t i = transLevel - 2; i >= 0; i--)
			{
				haarInverseTrans1D(image, image, image[0].size() / pow(2, i), image.size());
			}
		}
	}
	else
	{
		if (waveletCoeff.size() != 1){
			CDF53InverseTrans(image, image, image[0].size() / pow(2, transLevel - 1), image.size() / pow(2, transLevel - 1));
			for (int32_t i = transLevel - 2; i >= 0; i--)
			{
				CDF53InverseTrans(image, image, image[0].size() / pow(2, i), image.size() / pow(2, i));
			}
		}
		else{
			CDF53InverseTrans1D(image, image, image[0].size() / pow(2, transLevel - 1), image.size());
			for (int32_t i = transLevel - 2; i >= 0; i--)
			{
				CDF53InverseTrans1D(image, image, image[0].size() / pow(2, i), image.size());
			}
		}
	}	
}

void entropyCoding(std::vector<std::vector<int32_t>> const waveletCoeff, BitOutputStream &outFileBin, int bitLevel, int transLevel, bool lossless)
{
	int32_t height = waveletCoeff.size(), width = waveletCoeff[0].size();
	int32_t length = height * width;
	int32_t start_bit_num = outFileBin.countBit;
	// get the shape of subbands, prepare for the lossy mode. 
	// subband_height = [height, height/2, height/4, height/8, ...]
	// subband_width = [width, width/2, width/4, widtht/8, ...]
	std::vector<int32_t> subband_height;
	std::vector<int32_t> subband_width;

	subband_height.push_back(height);
	subband_width.push_back(width);
	// controlling write bits into the bitstream
	bool write_flag = true;

	for (int32_t i = 1; i <= transLevel; i++)
	{
		if (height != 1){
			subband_height.push_back(height / pow(2, i));
		}
		subband_width.push_back(width / pow(2, i));
	}
	// flatten one dim
	vector<int32_t> waveletCoffFlat(length, 0);
	for (int32_t h = 0; h < height; h++)
	{
		for (int32_t w = 0; w < width; w++)
		{
			waveletCoffFlat[h * width + w] = waveletCoeff[h][w];
		}
	}
	vector<int> onePosition(length, 0); // value range: 1 ~ bitLevel

	// sign: positive 0, negative 1
	vector<char> sign(length, 0);

	PositionCoding positionCoding(outFileBin);

	// coding
	for (int bitIndex = bitLevel; bitIndex > 0; bitIndex--)
	{

		int32_t MASK = 1 << (bitIndex - 1);
		for (int i = 0; i < length; i++)
		{
			if (onePosition[i] != 0)
			// only skip the detail data
			{
				write_flag = lossless ? true : false;
				// get the position of the current element
				int row = i / width;
				int col = i % width;
				if (height == 1)
				{
					// for the lossy mode, tileheight = 1 skip H0 bitIndex<=3
					if (!((subband_width[1] < col) && (col < subband_width[0]) && (bitIndex <= 3))){
						write_flag = true;
					}
				}
				else{
					// for the lossy mode, tileheight != 1 skip HH0 bitIndex<=1
					if (!((subband_height[1] < row) && (row < subband_height[0]) && (subband_width[1] < col) && (col < subband_width[0]) && (bitIndex <= 1))){
						write_flag = true;
					}
				}
				if (write_flag)
				{
					char bit = (abs(waveletCoffFlat[i]) & MASK) != 0;
					outFileBin.write(bit);
				}
			}
		}

		// position data
		for (int i = 0; i < length; i++)
		{
			if (onePosition[i] == 0)
			{
				char bit = (abs(waveletCoffFlat[i]) & MASK) != 0;
				if (bit != 0)
				{
					onePosition[i] = bitIndex;
					sign[i] = waveletCoffFlat[i] < 0;
				}
				positionCoding.write(bit, sign[i]);
			}
		}
		positionCoding.tailCoding();
	}
	// byte alignment
	int num_bits = outFileBin.countBit - start_bit_num;
	int align_bits = ceil(float(num_bits) / 64 / 8) * 64 * 8 - num_bits;
	for (int i=0; i<align_bits; i++)
	{
		char bit = 0;
		outFileBin.write(bit);
	}
}

void entropyDecoding(BitInputStream &inFileBin, std::vector<std::vector<int32_t>> &waveletCoeff, int bitLevel, int transLevel, bool lossless)
{
	int32_t height = waveletCoeff.size(), width = waveletCoeff[0].size();
	int32_t length = height * width;
	int32_t start_bit_num = inFileBin.countBit;

	std::vector<int32_t> subband_height;
	std::vector<int32_t> subband_width;

	subband_height.push_back(height);
	subband_width.push_back(width);
	for (int32_t i = 1; i <= transLevel; i++)
	{
		if (height != 1){
			subband_height.push_back(height / pow(2, i));
		}
		subband_width.push_back(width / pow(2, i));
	}
	// controlling read bits from the bitstream
	bool read_flag = true;

	vector<int32_t> waveletCoffFlat(length, 0);

	// first 1 position,  value range: 1 ~ bitLevel
	vector<int> onePosition(length, 0);

	// sign: positive 0, negative 1
	vector<char> sign(length, 0);

	PositionDecoding positionDecoding(inFileBin);

	// decoding
	for (int bitIndex = bitLevel; bitIndex > 0; bitIndex--)
	{

		int32_t MASK = 1 << (bitIndex - 1);

		// detail data
		for (int i = 0; i < length; i++)
		{
			if (onePosition[i] != 0)
			{
				read_flag = lossless ? true : false;

				int bit = 0;
				int row = i / width;
				int col = i % width;
				//skip mode keep the same as encode
				if (height == 1){
					if (!((subband_width[1] < col) && (col < subband_width[0]) && (bitIndex <= 3))){
						read_flag = true;
					}
				}
				else{
					if (!((subband_height[1] < row) && (row < subband_height[0]) && (subband_width[1] < col) && (col < subband_width[0]) && (bitIndex <= 1))){
						read_flag = true;
					}
				}
				if (read_flag)
				{
					bit = inFileBin.read();
				}
				if (bit != 0)
				{
					waveletCoffFlat[i] |= ((bit << (bitIndex - 1)) & MASK);
				}
			}
		}
		// position data
		for (int i = 0; i < length; i++)
		{
			if (onePosition[i] == 0)
			{
				int32_t bit = 0;
				positionDecoding.read(bit, sign[i]);
				// cout << int(bit) << " ";
				if (bit != 0)
				{
					onePosition[i] = bitIndex;
					waveletCoffFlat[i] |= ((bit << (bitIndex - 1)) & MASK);
				}
			}
		}
		positionDecoding.tailDecoding();
	}

	// resize two dim
	for (int32_t h = 0; h < height; h++)
	{
		for (int32_t w = 0; w < width; w++)
		{
			if (sign[h * width + w] != 0)
				waveletCoeff[h][w] = -waveletCoffFlat[h * width + w];
			else
				waveletCoeff[h][w] = waveletCoffFlat[h * width + w];
		}
	}
	// byte alignment
	int num_bits = inFileBin.countBit - start_bit_num;
	int align_bits = ceil(float(num_bits) / 64 / 8) * 64 * 8 - num_bits;
	for (int i=0; i<align_bits; i++)
	{
		inFileBin.read();
	}
}

void encodeChannel(int32_t width, int32_t height, int32_t tile_width, int32_t tile_height, int32_t startOffset, int32_t bitLevel, int32_t transLevel, std::string image_type, std::string imageFile, BitOutputStream &outFileBin, bool lossless, bool rearrange_flag, bool reshape_flag, bool long_flag, bool chroma, bool isHaar)
{
	vector<vector<vector<int>>> tiles;
	if (chroma)
	{
		// read uv and remap them
		vector<vector<int32_t>> image(height, vector<int32_t>(width*2, 0));
		vector<vector<int32_t>> u_channel(height, vector<int32_t>(width, 0));
		readImage(u_channel, imageFile, height, width, startOffset, long_flag);
		startOffset += height * width;
		vector<vector<int32_t>> v_channel(height, vector<int32_t>(width, 0));
		readImage(v_channel, imageFile, height, width, startOffset, long_flag);
		for (int i=0; i < height; i++)
		{
			for (int j=0; j < width; j++)
			{
				image[i][2*j] = u_channel[i][j];
				image[i][2*j+1] = v_channel[i][j];
			}
		}
		tiles = splitImageIntoTiles(image, tile_height, tile_width);
	}
	else
	{
		// read gray image
		vector<vector<int32_t>> image(height, vector<int32_t>(width, 0));
		readImage(image, imageFile, height, width, startOffset, long_flag);
		tiles = splitImageIntoTiles(image, tile_height, tile_width);
	}
	cout << std::left << std::setw(20) << "num_tiles:" << tiles.size() << endl;
	int tile_num = 0;
	for (auto tile : tiles)
	{
		vector<vector<int32_t>> paddedTile;
		if (tile_height != 1)
		{
			vector<vector<int32_t>> updatedTile;
			if ((tile_height * tile_width == 256) && (reshape_flag))
			{
				updatedTile = reshapeTile(tile, 16, 16);
			}
			else if ((rearrange_flag) && (image_type == "raw"))
			{
				// rearrange the raw image
				updatedTile = gridMap(tile);
			}
			else
			{
				updatedTile = tile;
			}
			int cur_height = updatedTile.size(), cur_width = updatedTile[0].size();
			int padHeight = ceil(float(cur_height) / pow(2, transLevel)) * pow(2, transLevel);
			int padWidth = ceil(float(cur_width) / pow(2, transLevel)) * pow(2, transLevel);
			paddedTile = paddingImage(updatedTile, padWidth, padHeight);
		}
		else
		// use 1 D wavelet for tile_height = 1
		{
			vector<vector<int32_t>> updatedTile;
			if ((rearrange_flag) && (image_type == "raw"))
			{
				updatedTile = gridMap(tile);
			}
			else
			{
				updatedTile = tile;
			}
			int cur_height = updatedTile.size(), cur_width = updatedTile[0].size();
			int padWidth = ceil(float(cur_width) / pow(2, transLevel)) * pow(2, transLevel);
			paddedTile = paddingImage(updatedTile, padWidth, cur_height);
		}
		vector<vector<int32_t>> waveletCoeff(paddedTile.size(), vector<int32_t>(paddedTile[0].size(), 0));
		// forward wavelet transform
		forwardTrans(paddedTile, waveletCoeff, transLevel, isHaar);
		// entropy coding
		entropyCoding(waveletCoeff, outFileBin, bitLevel, transLevel, lossless);
		tile_num += 1;
	}
}

std::vector<std::vector<int>> decodeChannel(int32_t width, int32_t height, int32_t tile_width, int32_t tile_height, int32_t startOffset, int32_t bitLevel, int32_t transLevel, std::string image_type, BitInputStream &inFileBin, bool lossless, bool rearrange_flag, bool reshape_flag, bool chroma, bool isHaar)
{
	vector<vector<vector<int>>> tiles;
	if (chroma)
	{
		width = 2 * width;
	}
	int tile_num = calculateTileNum(height, width, tile_height, tile_width);
	int tile_rows = ceil(float(height) / tile_height);
	int tile_cols = ceil(float(width) / tile_width);
	cout << "tile_rows " << tile_rows << " tile_cols " << tile_cols << endl;
	// tile decode
	for (int i = 0; i < tile_num; i++)
	{
		vector<vector<int32_t>> deWaveletCoeff;
		int cur_height, cur_width, pre_height, pre_width;
		// judge whether the tile is in the last row or col
		if ((int(i / tile_cols) == tile_rows - 1) && (height % tile_height != 0)){
			cur_height = height % tile_height;
		}
		else{
			cur_height = tile_height;
		}
		if ((int(i % tile_cols) == tile_cols - 1) && (width % tile_width != 0)){
			cur_width = width % tile_width;
		}
		else{
			cur_width = tile_width;
		}
		// record the pre shape for inverse
		pre_height = cur_height, pre_width = cur_width;
		vector<vector<int32_t>> tile(cur_height, vector<int32_t>(cur_width, 0));
		// get the shape of the recTile
		if ((cur_height * cur_width == 256) && (reshape_flag == 1) && (cur_height != 1))
		{
			tile = reshapeTile(tile, 16, 16);
			cur_height = 16, cur_width = 16;
		}
		int padHeight = ceil(float(cur_height) / pow(2, transLevel)) * pow(2, transLevel);
		int padWidth = ceil(float(cur_width) / pow(2, transLevel)) * pow(2, transLevel);		
		if (cur_height == 1)
		// 1D wavelet
		{
			padHeight = cur_height;
		}
		deWaveletCoeff = paddingImage(tile, padWidth, padHeight);
		entropyDecoding(inFileBin, deWaveletCoeff, bitLevel, transLevel, lossless);
		vector<vector<int32_t>> recTile(deWaveletCoeff.size(), vector<int32_t>(deWaveletCoeff[0].size(), 0));
		inverseTrans(deWaveletCoeff, recTile, transLevel, isHaar);
		// reshape or rearrange
		if (cur_height != 1)
		{
			if ((cur_height * cur_width == 256) && (reshape_flag))
			{
				recTile = reshapeTile(recTile, pre_width, pre_height);
			}
			else if ((rearrange_flag) && (image_type == "raw"))
			{
				// rearrange only raw image
				recTile = gridMapInverse(recTile);
			}
		}
		else
		{
			if ((rearrange_flag) && (image_type == "raw"))
			{
				recTile = gridMapInverse(recTile);
			}
		}
		
		vector<vector<int32_t>> croppedRecTile = cropImage(recTile, pre_height, pre_width);
		tiles.push_back(croppedRecTile);
	}
	// merge
	std::vector<std::vector<int>> recImage = mergeTileIntoImage(tiles, height, width);
	return recImage;
}
