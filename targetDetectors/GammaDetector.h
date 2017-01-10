#pragma once

#include <cmath>
#include <iostream>
#include "Detector.h"
#include "SpecialFunctions.h"

using namespace std;

class GammaDetector : public Detector {
public:

protected:
	virtual void estimatePdfParameters(double* clutterValues, const int numberOfClutterValues)
	{
		double mu = 0.0;
		double sigmaSq = 0.0;

		for (int i=0; i<numberOfClutterValues; i++) {
			mu += clutterValues[i];
			sigmaSq += clutterValues[i] * clutterValues[i];
		}

		mu /= numberOfClutterValues;
		sigmaSq /= (numberOfClutterValues - 1);
		sigmaSq -= numberOfClutterValues * mu * mu / (numberOfClutterValues - 1);

		beta = sigmaSq / mu;
		gamma = mu / beta;
	}

	virtual double estimateThreshold(const double probabilityOfFalseAlarm)
	{
		return inverseIncompleteGammaFunc(gamma, probabilityOfFalseAlarm) * beta;
	}

private:
	double gamma;
	double beta;
};
