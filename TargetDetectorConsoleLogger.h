#pragma once

#include <iostream>
#include "TimeMeasurer.h"
#include "TargetDetectorBaseLogger.h"

using namespace std;

class TargetDetectorConsoleLogger  : public TargetDetectorBaseLogger {
public:

	virtual void showTime(string message, double measuredTime)
	{
		cout << message << " " << measuredTime << " msecs" << endl;
	}

	virtual void printText(string text)
	{
		cout << text << endl;
	}

	virtual void showFitting(RayleighMixtureData& rayleighMixtureData)
	{
		Mat empiricalAndEstimatedPDFimage = createEmpiricalAndEstimatedPDFimage(rayleighMixtureData);

		imshow("Empirical and estimated PDFs", empiricalAndEstimatedPDFimage);

		waitKey();
	}

private:

	Mat createEmpiricalAndEstimatedPDFimage(RayleighMixtureData& rayleighMixtureData)
	{
		cout << endl;
		cout << "Rayleigh mixture count = " << rayleighMixtureData.intervalCount << endl;
		cout << "Initial cost value = " << rayleighMixtureData.initialError << endl;
		cout << "Final cost value = " << rayleighMixtureData.finalError << endl;
		cout << "Iteration count = " << rayleighMixtureData.iterationCount << endl << endl;

		for (int i = 0; i < rayleighMixtureData.intervalCount; i++) {
			cout << "Rayleigh " << i + 1 << " : weight = " << rayleighMixtureData.Weights[i] << ", sigma = " << rayleighMixtureData.Sigmas[i] << endl;
		}
		cout << endl;

		for (int i = 0; i < rayleighMixtureData.intervalCount + 2; i++) {
			cout << "Percentile " << i << "  = " << rayleighMixtureData.intervals[i] << endl;
		}
		cout << endl;

		for (int i = 0; i < rayleighMixtureData.intervalCount + 2; i++) {
			cout << "Percentile * " << i << "  = " << (i > 0 ? rayleighMixtureData.intervals[i] - rayleighMixtureData.intervals[i-1] : rayleighMixtureData.intervals[i]) << endl;
		}
		cout << endl;

		// plot empirical and estimated pdf
		const int histogramWidth = rayleighMixtureData.histogram.cols;
		const int histogramHeight = min(max(rayleighMixtureData.histogram.cols / 2, 250), 750);
		const double upperPercentile = 1.0;
		const int extraBand = 16;
		Mat histogramImage = ImageUtilities::createHistogramImage<int>(rayleighMixtureData.histogram, histogramWidth, histogramHeight, upperPercentile, extraBand);
		cvtColor(histogramImage, histogramImage, CV_GRAY2RGB);

		const double pdfStep = (double)rayleighMixtureData.histogramSize / rayleighMixtureData.histogram.cols;
		const double pdfNormalizer = rayleighMixtureData.histogramSum / rayleighMixtureData.histogramMaximumOccurance * histogramHeight;

		Point oldPtsEstimated, oldPtsEmpirical;
		for (int x = 0; x < rayleighMixtureData.histogram.cols; x++) {
			const double pEmpirical = rayleighMixtureData.pdfEmpirical[(int)(x * pdfStep)];
			const double pEstimated = rayleighMixtureData.calculateProbability(x);

			const int yEmpirical = (int)(histogramHeight - pEmpirical * pdfNormalizer);
			Point ptsEmpirical(x + extraBand, yEmpirical + extraBand);
			if (x > 0) {
				line(histogramImage, oldPtsEmpirical, ptsEmpirical, Scalar(255, 0, 0), 2);
			}
			oldPtsEmpirical = ptsEmpirical;

			const int yEstimated = (int)(histogramHeight - pEstimated * pdfNormalizer);
			Point ptsEstimated(x + extraBand, yEstimated + extraBand);
			if (x > 0) {
				line(histogramImage, oldPtsEstimated, ptsEstimated, Scalar(0, 0, 255), 2);
			}
			oldPtsEstimated = ptsEstimated;
		}

		return histogramImage;
	}

};
