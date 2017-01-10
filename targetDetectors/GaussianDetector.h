#pragma once

#include <cmath>
#include <iostream>
#include "Detector.h"
#include "SpecialFunctions.h"

using namespace std;

class GaussianDetector : public Detector {
public:

protected:
	double mu;
	double sigma;

	virtual void estimatePdfParameters(double* clutterValues, const int numberOfClutterValues)
	{
		mu = 0.0;
		sigma = 0.0;

		for(int i=0; i<numberOfClutterValues; i++) {
			mu += clutterValues[i];
			sigma += (clutterValues[i] * clutterValues[i]);
		}

		mu /= numberOfClutterValues;
		sigma /= (numberOfClutterValues - 1);
		sigma -= numberOfClutterValues * mu * mu / (numberOfClutterValues - 1);
		sigma = sqrt(sigma);
	}
	
	virtual double estimateThreshold(const double probabilityOfFalseAlarm)
	{
		return (1.414213562373095 * sigma * inverseCompErrorFunc(2 * probabilityOfFalseAlarm) + mu);
	}

private:

};
