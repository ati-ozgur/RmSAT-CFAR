#pragma once

#include <opencv2\opencv.hpp>
#include "MathUtilities.h"
#include "AdaptiveSimulatedAnnealing.h"
#include "RayleighMixtureData.h"
#include "DetermineMixtureParameters.h"
#include "IntegralImageData.h"
#include "FastTargetDetector.h"
#include "TargetDetectorBaseLogger.h"

using namespace std;
using namespace cv;


class SummedAreaTableTargetDetector {
public:
	SummedAreaTableTargetDetector(int minimumMixtureCount, int maximumMixtureCount, int guardRadius, int clutterRadius)
	{
		this->minimumMixtureCount = min(minimumMixtureCount, maximumMixtureCount);
		this->dimension = maximumMixtureCount;
		this->guardRadius = guardRadius;
		this->windowRadius = guardRadius + clutterRadius;

		histogramSize = 250;
		_internalLogger = new TargetDetectorBaseLogger;
		_logger = _internalLogger;
	}

	virtual ~SummedAreaTableTargetDetector()
	{
		delete _internalLogger;
	}

	void execute(Mat& image, Mat& targetMap, Mat& globalHistogram, double probabilityOfFalseAlarm, Rect workingRect = Rect())
	{
		if (workingRect.width == 0 || workingRect.height == 0) {
			workingRect.x = 0;
			workingRect.y = 0;
			workingRect.width = image.cols;
			workingRect.height = image.rows;
		}

		switch (image.type())
		{
		case CV_8U:  detectTargets<unsigned char>(image, targetMap, globalHistogram, probabilityOfFalseAlarm, workingRect);		break;
		case CV_8S:  detectTargets<char>(image, targetMap, globalHistogram, probabilityOfFalseAlarm, workingRect);				break;
		case CV_16U: detectTargets<unsigned short>(image, targetMap, globalHistogram, probabilityOfFalseAlarm, workingRect);	break;
		case CV_16S: detectTargets<short>(image, targetMap, globalHistogram, probabilityOfFalseAlarm, workingRect);				break;
		case CV_32S: detectTargets<int>(image, targetMap, globalHistogram, probabilityOfFalseAlarm, workingRect);				break;
		default: targetMap = Scalar(0);
		}
	}

	void setLogger(TargetDetectorBaseLogger* logger)
	{
		this->_logger = logger;
	}

	TargetDetectorBaseLogger* getLogger() const
	{
		return _logger;
	}

	void setHistogramSize(int histogramSize)
	{
		this->histogramSize = histogramSize;
	}

	int getHistogramSize() const
	{
		return histogramSize;
	}

private:
	int dimension;
	int minimumMixtureCount;
	int guardRadius;
	int windowRadius;
	int histogramSize;
	
	TargetDetectorBaseLogger* _logger;
	TargetDetectorBaseLogger* _internalLogger;

	template<typename T>
	void detectTargets(Mat& image, Mat& targetMap, Mat& globalHistogram, double probabilityOfFalseAlarm, Rect workingRect)
	{
		_logger->startTotalTimer();

		T startIndex = 1;
		if (doesContainData<T>(image, startIndex)) {
			_logger->startTimer();
			RayleighMixtureData rayleighMixtureData(image, globalHistogram, histogramSize, dimension, probabilityOfFalseAlarm);
			_logger->endTimer("RayleighMixtureData\t\t\t= ");

			// uncomment to see created censor-map as a result image
			///targetMap = rayleighMixtureData.censorMap;   return;

			DetermineMixtureParameters::set<T>(rayleighMixtureData, minimumMixtureCount);
			_logger->endTimer("DetermineMixtureParameters::set<T>\t= ");

			IntegralImageData<T> integralImageData(rayleighMixtureData);
			_logger->endTimer("IntegralImageData<T>\t\t\t= ");

			FastTargetDetector<T> fastTargetDetector;
			TargetDetectionInformation targetDetectionInformation = fastTargetDetector.execute(rayleighMixtureData, integralImageData, guardRadius, windowRadius, probabilityOfFalseAlarm, targetMap, workingRect);
			_logger->endTimer("FastTargetDetector<T>\t\t\t= ");

			_logger->endTotalTimer("TOTAL TIME \t\t\t\t= ");

			stringstream ss;
			ss << "Detected target pixel ratio = " << fixed << targetDetectionInformation.targetRatio << " (PFA = " << probabilityOfFalseAlarm << ")" << endl;
			ss << "Clutter region expainsion ratio = " << fixed << targetDetectionInformation.targetRatio << endl;
			_logger->printText(ss.str());

			_logger->showFitting(rayleighMixtureData);
		}
		else {
			_logger->printText("Does not contain any data pixel!");

			for (int y = 0; y < targetMap.rows; y++) {
				unsigned char* trow = (unsigned char*)(targetMap.data + y * targetMap.step);

				for (int x = 0; x < targetMap.cols; x++) {
					trow[x] = 0;
				}
			}
		}
	}

	template<typename T>
	bool doesContainData(Mat& image, T startIndex)
	{
		for (int y = 0; y < image.rows; y++) {
			T* irow = (T*)(image.data + y * image.step);

			for (int x = 0; x < image.cols; x++) {
				if (irow[x] >= startIndex) {
					return true;
				}
			}
		}

		return false;
	}

};
