#pragma once

#include <math.h>
#include <omp.h>
#include <opencv2\opencv.hpp>

using namespace cv;
using namespace std;


class AdaptiveAndFastCFAR : public AbstractCFAR {
public:
	AdaptiveAndFastCFAR()
	{
	}

	virtual ~AdaptiveAndFastCFAR()
	{
	}

	virtual AbstractCFAR* clone()
	{
		return new AdaptiveAndFastCFAR;
	}

	virtual Mat execute(Mat image, double probabilityOfFalseAlarm, map<string, double>& parameters)
	{
		const int guardRadius = (int)getParameterValue(parameters, "AAF-CFAR.guardRadius", 5);
		const int clutterRadius = (int)getParameterValue(parameters, "AAF-CFAR.clutterRadius", 5);
		const double censoringPercentile = getParameterValue(parameters, "AAF-CFAR.censoringPercentile", 99.9);

		Mat targetImage(image.rows, image.cols, CV_8UC1, Scalar(0));

		const int windowRadius = guardRadius + clutterRadius;

		const int startIndex = 1;
		Mat histogram = ImageUtilities::createHistogram(image, startIndex);
		const double globalTargetThreshold = sqr(ImageUtilities::getPercentileIndex<int>(histogram, censoringPercentile / 100.0));

		Mat powerImage = createPowerImage(image);

		const int threadCount = min(getThreadCount(), image.rows);

		double* powerImageRow;
		unsigned char* targetImageRow;
		double* powerImageWindowRow;
		bool fullWindowUpdate;
		double sumI, sumI2, meanI, meanI2, alpha, gamma;
		int x, y, xx, yy, xa, ya, sumC;
		#pragma omp parallel private(sumI, sumI2, sumC) num_threads(threadCount)
		{
			#pragma omp for private(x, y, xx, yy, xa, ya, powerImageRow, targetImageRow, powerImageWindowRow, fullWindowUpdate, meanI, meanI2, alpha, gamma)
			for (y = 0; y<image.rows; y++) {
				powerImageRow = (double*)(powerImage.data + y * powerImage.step);
				targetImageRow = (unsigned char*)(targetImage.data + y * targetImage.step);

				for (x = 0; x<image.cols; x++) {
					if (x == 0) {
						fullWindowUpdate = true;

						sumI = 0.0;
						sumI2 = 0.0;
						sumC = 0;
					}

					if (fullWindowUpdate) {
						fullWindowUpdate = false;

						for (yy = -windowRadius; yy <= windowRadius; yy++) {
							ya = y + yy;

							if (ya >= 0 && ya < image.rows) {
								powerImageWindowRow = (double*)(powerImage.data + ya * powerImage.step);

								for (xx = -windowRadius; xx <= windowRadius; xx++) {
									if (xx == -guardRadius && yy >= -guardRadius && yy <= guardRadius)
										xx = guardRadius;
									else {
										xa = x + xx;

										if (xa >= 0 && xa < image.cols && powerImageWindowRow[xa] > 0 && powerImageWindowRow[xa] < globalTargetThreshold) {
											sumI += powerImageWindowRow[xa];
											sumI2 += sqr(powerImageWindowRow[xa]);
											sumC++;
										}
									}
								}
							}
						}
					}
					else {
						for (yy = -windowRadius; yy <= windowRadius; yy++) {
							ya = y + yy;

							if (ya >= 0 && ya < image.rows) {
								powerImageWindowRow = (double*)(powerImage.data + ya * powerImage.step);

								// remove
								xa = x - windowRadius - 1;
								if (xa >= 0 && xa < image.cols && powerImageWindowRow[xa] > 0 && powerImageWindowRow[xa] < globalTargetThreshold) {
									sumI -= powerImageWindowRow[xa];
									sumI2 -= sqr(powerImageWindowRow[xa]);
									sumC--;
								}

								// add
								xa = x + windowRadius;
								if (xa >= 0 && xa < image.cols && powerImageWindowRow[xa] > 0 && powerImageWindowRow[xa] < globalTargetThreshold) {
									sumI += powerImageWindowRow[xa];
									sumI2 += sqr(powerImageWindowRow[xa]);
									sumC++;
								}
							}
						}

						for (yy = -guardRadius; yy <= guardRadius; yy++) {
							ya = y + yy;

							if (ya >= 0 && ya < image.rows) {
								powerImageWindowRow = (double*)(powerImage.data + ya * powerImage.step);

								// remove
								xa = x - guardRadius - 1;
								if (xa >= 0 && xa < image.cols && powerImageWindowRow[xa] > 0 && powerImageWindowRow[xa] < globalTargetThreshold) {
									sumI += powerImageWindowRow[xa];
									sumI2 += sqr(powerImageWindowRow[xa]);
									sumC++;
								}

								// add
								xa = x + guardRadius;
								if (xa >= 0 && xa < image.cols && powerImageWindowRow[xa] > 0 && powerImageWindowRow[xa] < globalTargetThreshold) {
									sumI -= powerImageWindowRow[xa];
									sumI2 -= sqr(powerImageWindowRow[xa]);
									sumC--;
								}
							}
						}
					}

					if (sumC > 0) {
						meanI = (sumI / sumC);
						meanI2 = (sumI2 / sumC);

						alpha = -1 - meanI2 / (meanI2 - 2 * sqr(meanI));
						gamma = (-alpha - 1) * meanI;

						if (powerImageRow[x] > gamma * (pow(probabilityOfFalseAlarm, 1.0 / alpha) - 1)) {
							targetImageRow[x] = UCHAR_MAX;
						}
					}
				}
			}
		}

		return targetImage;
	}

	virtual int getClutterArea(map<string, double>& parameters)
	{
		const int guardRadius = (int)parameters["AAF-CFAR.guardRadius"];
		const int clutterRadius = (int)parameters["AAF-CFAR.clutterRadius"];
		const int windowRadius = (guardRadius + clutterRadius);

		const int clutterArea = sqr(2 * windowRadius + 1) - sqr(2 * guardRadius + 1);

		return clutterArea;
	}

	virtual int getBandWidth(map<string, double>& parameters)
	{
		const int guardRadius = (int)parameters["AAF-CFAR.guardRadius"];
		const int clutterRadius = (int)parameters["AAF-CFAR.clutterRadius"];
		const int windowRadius = (guardRadius + clutterRadius);

		return calculateBandSize(windowRadius);
	}

	virtual bool requiresGlobalHistogram() const { return true; }

private:

	Mat createPowerImage(Mat image)
	{
		Mat powerImage;
		image.convertTo(powerImage, CV_64FC1);

		for (int y = 0; y < powerImage.rows; y++) {
			double* pirow = (double*)(powerImage.data + y * powerImage.step);

			for (int x = 0; x < powerImage.cols; x++) {
				pirow[x] *= pirow[x];
			}
		}

		return powerImage;
	}
};
