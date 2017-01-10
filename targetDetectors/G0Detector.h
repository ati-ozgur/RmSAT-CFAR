#pragma once

#include <cmath>
#include <iostream>
#include "Detector.h"

using namespace std;


class G0Detector : public Detector {
public:

protected:
	virtual void estimatePdfParameters(double* clutterValues, const int numberOfClutterValues)
	{
		double numberOfLooks = 1.0; //1.375;
		double mu = 0.0;
		double sigmaSq = 0.0;

		for (int i=0; i<numberOfClutterValues; i++) {
			mu += clutterValues[i];
			sigmaSq += clutterValues[i] * clutterValues[i];
		}

		mu /= numberOfClutterValues;
		sigmaSq /= (numberOfClutterValues - 1);

		alpha = -1 - (numberOfLooks * sigmaSq) / (numberOfLooks * sigmaSq - (numberOfLooks + 1) * mu * mu);
		gamma = (-alpha - 1) * mu;
	}

	virtual double estimateThreshold(const double probabilityOfFalseAlarm)
	{
		return gamma*(pow(probabilityOfFalseAlarm, 1 / alpha) - 1);
	}

private:
	double alpha;
	double gamma;
};
