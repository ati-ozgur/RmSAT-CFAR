#pragma once

#include <opencv2\opencv.hpp>
#include "MathUtilities.h"
#include "AbstractCostFunction.h"
#include "RayleighMixtureData.h"

using namespace std;
using namespace cv;


template<typename T>
class RayleighMixtureCostFunction : public AbstractCostFunction {
public:
	RayleighMixtureCostFunction(RayleighMixtureData& rayleighMixtureData, int minimumMixtureCount)
		: AbstractCostFunction(rayleighMixtureData.dimension)
	{
		this->rayleighMixtureData = &rayleighMixtureData;
		this->minimumMixtureCount = minimumMixtureCount;
	}

	virtual ~RayleighMixtureCostFunction()
	{

	}

	// override
	virtual double evaluate(double* x)
	{
		if (estimateSigmasAndWeights(x)) {
			// calculate error between empirical and estimated distributions
			const int histogramSize = rayleighMixtureData->histogramSize;
			double* pdfEmpirical = rayleighMixtureData->pdfEmpirical;
			double* pdfEstimated = rayleighMixtureData->pdfEstimated;

			const double dataErrorTerm = absoluteDifferenceBetweenPDFs(pdfEmpirical, pdfEstimated, histogramSize);
			const double modelComplexityRegularizer = rayleighMixtureData->intervalCount;

			const double lambda = 0.0;
			if (dataErrorTerm > 0.0) 
				return log(dataErrorTerm) + lambda * modelComplexityRegularizer;
			else
				return lambda * modelComplexityRegularizer;
		}
		else {
			rayleighMixtureData->intervalCount = 0;
			return numeric_limits<double>::infinity();
		}
	}

	inline double absoluteDifferenceBetweenPDFs(double* pdfEmpirical, double* pdfEstimated, int histogramSize)
	{
		double errorSum = 0.0;

		for (int k = 0; k < histogramSize; k++) {
			const double absoluteDifference = abs(pdfEmpirical[k] - pdfEstimated[k]);
			errorSum += absoluteDifference;
		}

		return (errorSum / histogramSize);
	}

	bool estimateSigmasAndWeights(double* x_current)
	{
		const int histogramSize = rayleighMixtureData->histogramSize;

		int*& H = rayleighMixtureData->H;
		double*& pdfEmpirical = rayleighMixtureData->pdfEmpirical;
		double*& pdfEstimated = rayleighMixtureData->pdfEstimated;
		double*& Sn = rayleighMixtureData->Sn;
		double*& Sd = rayleighMixtureData->Sd;
		double*& Weights = rayleighMixtureData->Weights;
		double*& Sigmas = rayleighMixtureData->Sigmas;
		double*& sqrSigmas = rayleighMixtureData->sqrSigmas;
		double*& intervals = rayleighMixtureData->intervals;
		int& intervalCount = rayleighMixtureData->intervalCount;

		intervalCount = createIntervals(x_current, intervals);
		if (intervalCount < minimumMixtureCount || intervalCount > dimension) {
			return false;
		}

		// calculate parameters of each Rayleigh
		double weightSum = 0.0;
		for (int i = 0; i<intervalCount; i++) {
			const int intervalIndex = i + 1;
			const int intervalStart = rayleighMixtureData->getPercentileIndex(intervals[intervalIndex - 1]);
			const int intervalEnd = rayleighMixtureData->getPercentileIndex(intervals[intervalIndex + 1]);

			pair<double, double> estimation = rayleighMixtureData->estimateSigmaSqr(intervalStart, intervalEnd);

			const double phatSqr = estimation.first;
			const double sumOfPdf = estimation.second;

			const double epsilon = 1e-16;
			if (sumOfPdf < epsilon) {
				return false;
			}

			Weights[i] = sumOfPdf;
			Sigmas[i] = sqrt(phatSqr);
			sqrSigmas[i] = phatSqr;

			weightSum += Weights[i];
		}

		if (weightSum > 0) {
			for (int i = 0; i<intervalCount; i++) {
				Weights[i] /= weightSum;
			}
		}
		else {
			return false;
		}

		// calculate estimated pdf (mixture of Rayleigh)
		const double histogramStep = (double)rayleighMixtureData->histogram.cols / histogramSize;

		double maxPdfValue = 0.0;
		int maxPdfIndices = 0;
		for (int k = 0; k<histogramSize; k++) {
			const double x = (k * histogramStep);

			//TODO: Erman'in PDF icin tanimladigi recursive formulasini kullanip hizlandir
			pdfEstimated[k] = rayleighMixtureData->calculateProbability(x);

			if (pdfEstimated[k] >= maxPdfValue) {
				maxPdfValue = pdfEstimated[k];
				maxPdfIndices = k;
			}
		}

		if (maxPdfValue <= 0) {
			return false;
		}

		// prevent multi-modal distributions
		for (int k = 1; k < maxPdfIndices - 1; k++) {
			if (pdfEstimated[k] < pdfEstimated[k - 1]) {
				return false;
			}
		}
		for (int k = maxPdfIndices + 1; k < histogramSize; k++) {
			if (pdfEstimated[k] > pdfEstimated[k - 1]) {
				return false;
			}
		}

		return true;
	}

	void initializeX0(double* x)
	{
		const double intervalValue = 100.0 / (dimension + 1.0);

		for (int k = 0; k < dimension; k++) {
			x[k] = intervalValue;
		}
	}

	int createIntervals(double* x, double* intervals)
	{
		intervals[0] = 0.0;
		intervals[dimension + 1] = 100.0;

		double cumulativePercentileSum = 0.0;
		for (int k = 0; k < dimension; k++) {
			if (x[k] >= lowerBound && x[k] <= upperBound) {
				cumulativePercentileSum += x[k];

				if (cumulativePercentileSum < 100.0 && k < dimension)
					intervals[k + 1] = cumulativePercentileSum;
				else {
					intervals[k + 1] = 100.0;

					return k;
				}
			}
			else
				return 0;
		}

		return dimension;
	}

private:
	RayleighMixtureData* rayleighMixtureData;
	int minimumMixtureCount;

};
