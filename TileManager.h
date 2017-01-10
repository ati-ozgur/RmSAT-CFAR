#pragma once

#include <opencv2\opencv.hpp>

using namespace cv;
using namespace std;


class TileManager {
public:
	TileManager(Mat& image, int tileSize = 512, int bandSize = 16, int targetImageType = -1)
	{
		this->image = image;
		this->tileSize = tileSize;
		this->bandSize = bandSize;

		const int cellXcount = max((image.cols + tileSize - 1) / tileSize, 1);
		const int cellYcount = max((image.rows + tileSize - 1) / tileSize, 1);

		for (int yy=0; yy<cellYcount; yy++) {
			for (int xx=0; xx<cellXcount; xx++) {
				tileIndices.push_back( pair<int, int>(xx, yy) );
			}
		}

		if (targetImageType < 0) {
			targetImageType = image.type();
		}

		resultImage = Mat(image.rows, image.cols, targetImageType, Scalar(0));
	}

	Mat getimage() const
	{
		return image;
	}

	int getTileSize() const 
	{
		return tileSize;
	}

	int getBandSize() const
	{
		return bandSize;
	}

	vector<pair<int, int>> getTileIndices() const
	{
		return tileIndices;
	}

	Mat getInputTile(pair<int, int> tileIndex) const
	{
		const int tileXindex = tileIndex.first;
		const int tileYindex = tileIndex.second;

		const int x1 = (tileXindex * tileSize);
		const int y1 = (tileYindex * tileSize);
		const int x2 = min(x1 + tileSize, image.cols);
		const int y2 = min(y1 + tileSize, image.rows);

		const int x1e = max(x1 - bandSize, 0);
		const int y1e = max(y1 - bandSize, 0);
		const int x2e = min(x2 + bandSize, image.cols);
		const int y2e = min(y2 + bandSize, image.rows);

		Mat inputTile = image(Range(y1e, y2e), Range(x1e, x2e));

		return inputTile;
	}

	Rect getTileWorkingRectangle(pair<int, int> tileIndex)
	{
		const int tileXindex = tileIndex.first;
		const int tileYindex = tileIndex.second;

		const int x1 = (tileXindex * tileSize);
		const int y1 = (tileYindex * tileSize);
		const int x2 = min(x1 + tileSize, image.cols);
		const int y2 = min(y1 + tileSize, image.rows);

		const int x1e = max(x1 - bandSize, 0);
		const int y1e = max(y1 - bandSize, 0);
		const int x2e = min(x2 + bandSize, image.cols);
		const int y2e = min(y2 + bandSize, image.rows);

		return Rect(x1 - x1e, y1 - y1e, x2 - x1, y2 - y1);
	}

	template<typename T>
	void assignResultTile(pair<int, int> tileIndex, Mat& resultTile)
	{
		const int tileXindex = tileIndex.first;
		const int tileYindex = tileIndex.second;

		const int x1 = (tileXindex * tileSize);
		const int y1 = (tileYindex * tileSize);
		const int x2 = min(x1 + tileSize, image.cols);
		const int y2 = min(y1 + tileSize, image.rows);

		const int x1e = max(x1 - bandSize, 0);
		const int y1e = max(y1 - bandSize, 0);
		const int x2e = min(x2 + bandSize, image.cols);
		const int y2e = min(y2 + bandSize, image.rows);

		const int width = (x2 - x1);
		const int height = (y2 - y1);

		const int expandedWidth = (x2e - x1e);

		for (int y=0; y<height; y++) {
			T* srow = (T*)(resultTile.data + (y + y1 - y1e) * resultTile.step) + (x1 - x1e);
			T* trow = (T*)(resultImage.data + (y + y1) * resultImage.step) + x1;

			for (int  x=0; x<width; x++) {
				trow[x] = srow[x];
			}
		}
	}

	void setResultTile(pair<int, int> tileIndex, Mat& resultTile)
	{
		switch (resultImage.type())
		{
		case CV_8U:  assignResultTile<unsigned char>(tileIndex, resultTile);	break;
		case CV_8S:  assignResultTile<char>(tileIndex, resultTile);				break;
		case CV_16U: assignResultTile<unsigned short>(tileIndex, resultTile);	break;
		case CV_16S: assignResultTile<short>(tileIndex, resultTile);			break;
		case CV_32S: assignResultTile<int>(tileIndex, resultTile);				break;
		case CV_32F: assignResultTile<float>(tileIndex, resultTile);			break;
		case CV_64F: assignResultTile<double>(tileIndex, resultTile);			break;
		}
	}

	Mat getResultImage() const
	{
		return resultImage;
	}

	static Rect findBoundingBox(Mat& image)
	{
		switch (image.type())
		{
		case CV_8U:  return findBoundingBoxTemplated<unsigned char>(image);
		case CV_8S:  return findBoundingBoxTemplated<char>(image);
		case CV_16U: return findBoundingBoxTemplated<unsigned short>(image);
		case CV_16S: return findBoundingBoxTemplated<short>(image);
		case CV_32S: return findBoundingBoxTemplated<int>(image);
		case CV_32F: return findBoundingBoxTemplated<float>(image);
		case CV_64F: return findBoundingBoxTemplated<double>(image);
		default: return Rect(0, 0, image.cols, image.rows);
		}
	}

	template<typename T>
	static Rect findBoundingBoxTemplated(Mat& image)
	{
		int x1 = image.cols;
		int x2 = -1;
		int y1 = image.rows;
		int y2 = -1;

		for (int y=0; y<image.rows; y++) {
			T* imageRow = (T*)(image.data + y * image.step);

			bool rowFound = false;
			for (int x=0; x<image.cols; x++) {
				if (imageRow[x] > 0) {
					x1 = min(x1, x);
					x2 = max(x2, x);
					y1 = min(y1, y);
					y2 = max(y2, y);

					rowFound = true;
						
					break;
				}
			}

			if (rowFound) {
				for (int x=image.cols-1; x>0; x--) {
					if (imageRow[x] > 0) {
						x1 = min(x1, x);
						x2 = max(x2, x);
						y1 = min(y1, y);
						y2 = max(y2, y);

						break;
					}
				}
			}
		}

		x1 = max(x1 - 1, 0);
		x2 = min(x2 + 1, image.cols - 1);
		y1 = max(y1 - 1, 0);
		y2 = min(y2 + 1, image.rows - 1);

		Rect rect;
		rect.x = x1;
		rect.y = y1;
		rect.width = (x2 - x1 + 1);
		rect.height = (y2 - y1 + 1);
		return rect;
	}

private:
	Mat image;
	int tileSize;
	int bandSize;

	vector<pair<int, int>> tileIndices;
	Mat resultImage;

};
