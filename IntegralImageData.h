#pragma once

#include <opencv2\opencv.hpp>
#include "RayleighMixtureData.h"
#include "MathUtilities.h"

using namespace std;
using namespace cv;


template<typename T>
class IntegralImageData {
public:
	Mat I2, C;

	IntegralImageData(RayleighMixtureData& rayleighMixtureData)
	{
		Mat& image = rayleighMixtureData.image;
		Mat& censorMap = rayleighMixtureData.censorMap;
		const int intervalCount = rayleighMixtureData.intervalCount;
		double* intervals = rayleighMixtureData.intervals;

		int* intervalIndices = createIntervalIndices(rayleighMixtureData.histogram, intervalCount, intervals);

		createIntegralImages<T>(image, censorMap, intervalCount, intervalIndices);

		delete[] intervalIndices;
	}

private:

	int* createIntervalIndices(Mat& histogram, const int intervalCount, double* intervals)
	{
		int* intervalIndices = new int[intervalCount + 2];
		intervalIndices[0] = 1;
		for (int intervalIndex = 1; intervalIndex <= intervalCount + 1; intervalIndex++) {
			intervalIndices[intervalIndex] = ImageUtilities::getPercentileIndex<int>(histogram, intervals[intervalIndex] * 0.01);
		}

		return intervalIndices;
	}

	template<typename T>
	void createIntegralImages(Mat& image, Mat& censorMap, int intervalCount, int* intervalIndices)
	{
		I2 = Mat(image.rows * intervalCount, image.cols, CV_64FC1);
		C = Mat(image.rows * intervalCount, image.cols, CV_32SC1);

		const int intervalShiftPerInterval = (image.cols * image.rows);

		for (int y = 0; y<image.rows; y++) {
			T* imageRow = (T*)(image.data + y * image.step);
			unsigned char* censoringRow = (unsigned char*)(censorMap.data + y  * censorMap.step);

			double* I2row = (double*)(I2.data + y * I2.step);
			double* I2rowUp = (y > 0 ? (double*)(I2.data + (y - 1) * I2.step) : NULL);

			int* Crow = (int*)(C.data + y * C.step);
			int* CrowUp = (y > 0 ? (int*)(C.data + (y - 1) * C.step) : NULL);

			for (int x = 0; x<image.cols; x++) {
				const bool isNotCensored = (imageRow[x] > 0 && censoringRow[x] == 0);
				const T pixelvalue = (isNotCensored ? imageRow[x] : 0);

				int intervalShift = 0;
				for (int intervalIndex=1; intervalIndex<=intervalCount; intervalIndex++) {
					const int intervalStart = intervalIndices[intervalIndex - 1];
					const int intervalEnd = intervalIndices[intervalIndex + 1];

					const bool isInInterval = (pixelvalue >= intervalStart && pixelvalue <= intervalEnd);

					const int xShifted = x + intervalShift;

					I2row[xShifted] = (isInInterval ? MathUtilities::sqr<double>(pixelvalue) : 0)
									+ (x > 0 ? I2row[xShifted - 1] : 0)
									+ (y > 0 ? I2rowUp[xShifted] : 0)
									- (x > 0 && y > 0 ? I2rowUp[xShifted - 1] : 0);

					Crow[xShifted] = (isInInterval && isNotCensored ? 1 : 0)
									+ (x > 0 ? Crow[xShifted - 1] : 0)
									+ (y > 0 ? CrowUp[xShifted] : 0)
									- (x >0 && y > 0 ? CrowUp[xShifted - 1] : 0);

					intervalShift += intervalShiftPerInterval;
				}
			}
		}
	}

};
