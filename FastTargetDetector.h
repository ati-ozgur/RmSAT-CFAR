#pragma once

#include <opencv2\opencv.hpp>
#include "RayleighMixtureData.h"
#include "IntegralImageData.h"
#include "MathUtilities.h"

using namespace std;
using namespace cv;


struct TargetDetectionInformation {
	double targetRatio;
	double expansionRatio;

	TargetDetectionInformation(double targetRatio, double expansionRatio)
	{
		this->targetRatio = targetRatio;
		this->expansionRatio = expansionRatio;
	}
};

template<typename T>
class FastTargetDetector {
public:
	FastTargetDetector(int maximumExpansion = 4)
	{
		this->maximumExpansion = maximumExpansion;
	}

	int getMaximumExpansion() const
	{
		return maximumExpansion;
	}

	int setMaximumExpansion(int maximumExpansion)
	{
		this->maximumExpansion = maximumExpansion;
	}

	TargetDetectionInformation execute(RayleighMixtureData& rayleighMixtureData, IntegralImageData<T>& integralImageData, int guardRadius, int windowRadius, double probabilityOfFalseAlarm, Mat& targetMap, Rect workingRect)
	{
		Mat& image = rayleighMixtureData.image;
		const int intervalCount = rayleighMixtureData.intervalCount;
		double*& weights = rayleighMixtureData.Weights;

		Mat& I2 = integralImageData.I2;
		Mat& C = integralImageData.C;

		const int intervalShiftPerInterval = (image.cols * image.rows);

		// detect targets using SAT & Rayleigh mixtures
		const int minimumClutterArea = (MathUtilities::sqr(2 * windowRadius + 1) - MathUtilities::sqr(2 * guardRadius + 1)) / 2;

		double* I2w1;
		double* I2w2;
		double* I2g1;
		double* I2g2;
		int* Cw1;
		int* Cw2;
		int* Cg1;
		int* Cg2;

		const int x1 = workingRect.x;
		const int y1 = workingRect.y;
		const int x2 = x1 + workingRect.width;
		const int y2 = y1 + workingRect.height;

		const double minimumTargetValue = 1.0;

		int targetCount = 0;
		int expansionCount = 0;

		for (int y = y1; y < y2; y++) {	
			T* irow = (T*)(image.data + y * image.step);
			unsigned char* trow = (unsigned char*)(targetMap.data + y * targetMap.step);

			double* I2row = (double*)(I2.data + y * I2.step);
			int* Crow = (int*)(C.data + y * C.step);

			int lastExpansion = 0;
			for (int x = x1; x < x2; x++) {
				trow[x] = 0;

				const double pixelValue = irow[x];
				if (pixelValue >= minimumTargetValue) {
					const double pixelValueSqr = (pixelValue * pixelValue);

					for (int expansion=1; expansion<maximumExpansion; expansion++) {
						if (expansion != lastExpansion) {
							determineRowPointers(guardRadius, expansion * windowRadius, image.rows, y, I2, C, I2w1, I2w2, I2g1, I2g2, Cw1, Cw2, Cg1, Cg2);
							lastExpansion = expansion;
						}

						double probabilitySum = 0.0;
						double clutterArea = 0.0;

						int intervalShift = 0;
						for (int intervalIndex = 0; intervalIndex < intervalCount; intervalIndex++) {
							const int wx1 = intervalShift + max(x - expansion * windowRadius, 0);
							const int wx2 = intervalShift + min(x + expansion * windowRadius, image.cols - 1);
							const int gx1 = intervalShift + max(x - guardRadius, 0);
							const int gx2 = intervalShift + min(x + guardRadius, image.cols - 1);

							const double sumC = (Cw1[wx1] + Cw2[wx2] - Cw1[wx2] - Cw2[wx1]) - (Cg1[gx1] + Cg2[gx2] - Cg1[gx2] - Cg2[gx1]);
							if (sumC > 0) {
								clutterArea += sumC;

								const double sumI2 = (I2w1[wx1] + I2w2[wx2] - I2w1[wx2] - I2w2[wx1]) - (I2g1[gx1] + I2g2[gx2] - I2g1[gx2] - I2g2[gx1]);

								const double sigmaSqr = (0.5 * sumI2 / sumC);
								const double RayleighProbability = exp(-0.5 * pixelValueSqr / sigmaSqr);
								probabilitySum += weights[intervalIndex] * RayleighProbability;

								// if not target decision is certain then do early-exit 
								if (clutterArea >= minimumClutterArea && probabilitySum > probabilityOfFalseAlarm) {
									break;
								}
							}

							intervalShift += intervalShiftPerInterval;
						}

						// target decision rule
						if (clutterArea >= minimumClutterArea) {
							trow[x] = (probabilitySum < probabilityOfFalseAlarm ? UCHAR_MAX : 0);

							if (trow[x] > 0) {
								targetCount++;
							}
							break;
						}

						expansionCount++;
					}
				}
			}
		}

		const int totalPixelCount = (y2 - y1 + 1) * (x2 - x1 + 1);
		const double targetRatio = (double)targetCount / totalPixelCount;
		const double expansionRatio = double(expansionCount) / totalPixelCount;

		return TargetDetectionInformation(targetRatio, expansionRatio);
	}

private:
	int maximumExpansion;

	inline void determineRowPointers(int guardRadius, int windowRadius, int height, int y, Mat& I2, Mat& C,
		double*& I2w1, double*& I2w2, double*& I2g1, double*& I2g2, int*& Cw1, int*& Cw2, int*& Cg1, int*& Cg2)
	{
		const int wy1 = max(y - windowRadius, 0);
		const int wy2 = min(y + windowRadius, height - 1);
		const int gy1 = max(y - guardRadius, 0);
		const int gy2 = min(y + guardRadius, height - 1);

		I2w1 = (double*)(I2.data + wy1 * I2.step);
		I2w2 = (double*)(I2.data + wy2 * I2.step);
		I2g1 = (double*)(I2.data + gy1 * I2.step);
		I2g2 = (double*)(I2.data + gy2 * I2.step);

		Cw1 = (int*)(C.data + wy1 * C.step);
		Cw2 = (int*)(C.data + wy2 * C.step);
		Cg1 = (int*)(C.data + gy1 * C.step);
		Cg2 = (int*)(C.data + gy2 * C.step);
	}
};
