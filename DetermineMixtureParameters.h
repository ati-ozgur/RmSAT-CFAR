#pragma once

#include <opencv2\opencv.hpp>
#include "RayleighMixtureData.h"
#include "RayleighMixtureCostFunction.h"
#include "AdaptiveSimulatedAnnealing.h"

using namespace std;
using namespace cv;

class DetermineMixtureParameters {
public:
	template<typename T>
	static void set(RayleighMixtureData& rayleighMixtureData, int minimumMixtureCount)
	{
		if (rayleighMixtureData.dimension == 1) {
			// single Rayleigh
			pair<double, double> estimation = rayleighMixtureData.estimateSigmaSqr(0, rayleighMixtureData.histogramSize - 1);
			const double phatSqr = estimation.first;

			rayleighMixtureData.Weights[0] = 1.0;
			rayleighMixtureData.Sigmas[0] = sqrt(phatSqr);
			rayleighMixtureData.sqrSigmas[0] = phatSqr;

			rayleighMixtureData.intervalCount = 1;
			rayleighMixtureData.intervals[0] = 0;
			rayleighMixtureData.intervals[1] = 50;		// a dummy number between 1 and 99 --> resulting interval is [0, 100]
			rayleighMixtureData.intervals[2] = 100;

			rayleighMixtureData.initialError = 0.0;
			rayleighMixtureData.finalError = 0.0;
			rayleighMixtureData.iterationCount = 0;
		}
		else {
			// mixture of Rayleigh
			const double lowerBoundPercentage = 1.0;
			const double upperBoundPercentage = 99.0;
			RayleighMixtureCostFunction<T> rmCostFunction(rayleighMixtureData, minimumMixtureCount);
			rmCostFunction.setLowerBound(lowerBoundPercentage);
			rmCostFunction.setUpperBound(upperBoundPercentage);

			Mat x(rayleighMixtureData.dimension, 1, CV_64FC1, Scalar(0));
			double* x_initial = (double*)x.data;
			rmCostFunction.initializeX0(x_initial);

			rayleighMixtureData.initialError = rmCostFunction.evaluate(x_initial);

			const double initialTemperature = 250;
			const int iterationPerDimension = 1000;
			const double convergenceTolerance = 1e-4;
			AdaptiveSimulatedAnnealing asa(initialTemperature, iterationPerDimension, convergenceTolerance);

			SAOptimimumSolution optimimumSolution = asa.minimize(rmCostFunction, x_initial);
			rayleighMixtureData.finalError = rmCostFunction.evaluate(optimimumSolution.x_optimum);

			rayleighMixtureData.iterationCount = optimimumSolution.iteration;
		}
	}

private:

};
