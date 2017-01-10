#pragma once

#include <iostream>
#include <opencv2\opencv.hpp>
#include "TileManager.h"
#include "SummedAreaTableTargetDetector.h"
#include "TargetDetectorConsoleLogger.h"
#include "targetDetectors\AbstractCFAR.h"

using namespace std;
using namespace cv;


class RayleighMixtureSummedAreaTableCFAR : public AbstractCFAR {
public:
	RayleighMixtureSummedAreaTableCFAR()
	{
	}

	virtual AbstractCFAR* clone()
	{
		return new RayleighMixtureSummedAreaTableCFAR;
	}

	virtual Mat execute(Mat image, double probabilityOfFalseAlarm, map<string, double>& parameters)
	{
		const int guardRadius = (int)getParameterValue(parameters, "RmSAT-CFAR.guardRadius", 5);
		const int clutterRadius = (int)getParameterValue(parameters, "RmSAT-CFAR.clutterRadius", 5);
		const int minimumMixtureCount = (int)getParameterValue(parameters, "RmSAT-CFAR.minimumMixtureCount", 1);
		const int maximumMixtureCount = (int)getParameterValue(parameters, "RmSAT-CFAR.maximumMixtureCount", 5);

		// fit histogram into mixture of Rayleighs
		const int tileSize = 1024;
		const int bandSize = getBandWidth(parameters);

		TileManager tileManager(image, tileSize, bandSize, CV_8UC1);
		vector<pair<int, int>> tileIndices = tileManager.getTileIndices();

		const int threadCount = min(getThreadCount(), (int)tileIndices.size());

		omp_set_nested(1);

		Mat globalHistogram = createHistogram(image, tileSize, threadCount);

		TargetDetectorConsoleLogger targetDetectorConsoleLogger;

		int i;
		Rect workingRect;
		Mat inputTile;
		Mat targetTile;
		pair<int, int> tileIndex;
		SummedAreaTableTargetDetector* targetDetector = NULL;
		#pragma omp parallel private(targetDetector) num_threads(threadCount)
		{
			targetDetector = new SummedAreaTableTargetDetector(minimumMixtureCount, maximumMixtureCount, guardRadius, clutterRadius);
			
			// set logger
			///targetDetector->setLogger(&targetDetectorConsoleLogger);

			#pragma omp for private(i, tileIndex, inputTile, targetTile, workingRect) schedule(dynamic, 1)
			for (i = 0; i<tileIndices.size(); i++) {
				tileIndex = tileIndices.at(i);

				workingRect = tileManager.getTileWorkingRectangle(tileIndex);

				inputTile = createRayleighCompliantTile(tileManager.getInputTile(tileIndex));
				if (targetTile.cols != inputTile.cols || targetTile.rows != inputTile.rows) {
					targetTile = Mat(inputTile.rows, inputTile.cols, CV_8UC1);
				}

				targetDetector->execute(inputTile, targetTile, globalHistogram, probabilityOfFalseAlarm, workingRect);

				tileManager.setResultTile(tileIndex, targetTile);
			}

			delete targetDetector;
		}

		Mat targetMap = tileManager.getResultImage();

		return targetMap;
	}

	virtual int getClutterArea(map<string, double>& parameters)
	{
		const int guardRadius = (int)parameters["RmSAT-CFAR.guardRadius"];
		const int clutterRadius = (int)parameters["RmSAT-CFAR.clutterRadius"];
		const int windowRadius = (guardRadius + clutterRadius);

		const int clutterArea = sqr(2 * windowRadius + 1) - sqr(2 * guardRadius + 1);

		return clutterArea;
	}

	virtual int getBandWidth(map<string, double>& parameters)
	{
		const int guardRadius = (int)parameters["RmSAT-CFAR.guardRadius"];
		const int clutterRadius = (int)parameters["RmSAT-CFAR.clutterRadius"];
		const int windowRadius = (guardRadius + clutterRadius);

		return calculateBandSize(windowRadius);
	}

	virtual bool isDeterministic() const { return false; }

	virtual bool requiresGlobalHistogram() const { return true; }

private:

	static Mat createRayleighCompliantTile(Mat& tile)
	{
		Mat refinedTile(tile.rows, tile.cols, tile.type());

		switch (tile.type())
		{
		case CV_8U:  createRayleighCompliantTile<unsigned char>(tile, refinedTile);		break;
		case CV_8S:  createRayleighCompliantTile<char>(tile, refinedTile);				break;
		case CV_16U: createRayleighCompliantTile<unsigned short>(tile, refinedTile);	break;
		case CV_16S: createRayleighCompliantTile<short>(tile, refinedTile);				break;
		case CV_32S: createRayleighCompliantTile<int>(tile, refinedTile);				break;
		default: refinedTile = tile.clone();
		}

		return refinedTile;
	}

	template<typename T>
	static void createRayleighCompliantTile(Mat& tile, Mat& refinedTile)
	{
		Mat tileHistogram = ImageUtilities::createHistogram(tile);

		const double backgroundStartPercentile = 0.005;
		const double backgroundStart = ImageUtilities::getPercentileIndex<int>(tileHistogram, backgroundStartPercentile);

		for (int y = 0; y < tile.rows; y++) {
			T* irow = (T*)(tile.data + y * tile.step);
			T* rirow = (T*)(refinedTile.data + y * refinedTile.step);

			for (int x = 0; x < tile.cols; x++) {
				if (irow[x] - backgroundStart > 1)
					rirow[x] = (T)(max(irow[x] - backgroundStart, 0.0) + 1.0); 
				else
					rirow[x] = 0;
			}
		}
	}

	static Mat createHistogram(Mat& image, int tileSize, int simultaneouslyExecutedTile)
	{
		const int gridXcount = (image.cols + tileSize - 1) / tileSize;
		const int gridYcount = (image.rows + tileSize - 1) / tileSize;

		Mat histogram;

		Mat tile;
		Mat privateHistogram;
		int* hptr;
		int* phptr;
		int x, y, x1, y1, x2, y2, i;
		#pragma omp parallel private(privateHistogram)
		{
			#pragma omp for private(tile, x, y, x1, y1, x2, y2, i, hptr, phptr)
			for (y = 0; y < gridYcount; y++) {
				y1 = y * tileSize;
				y2 = min(y1 + tileSize, image.rows);

				for (x = 0; x < gridXcount; x++) {
					x1 = x * tileSize;
					x2 = min(x1 + tileSize, image.cols);

					tile = image(Range(y1, y2), Range(x1, x2));
					ImageUtilities::addToHistogram(tile, privateHistogram);
				}
			}

			#pragma omp critical
			{
				if (histogram.empty())
					histogram = privateHistogram.clone();
				else {
					if (!privateHistogram.empty()) {
						hptr = (int*)histogram.data;
						phptr = (int*)privateHistogram.data;

						for (i = 0; i < histogram.cols; i++) {
							hptr[i] += phptr[i];
						}
					}
				}
			}
		}

		return histogram;
	}

};
