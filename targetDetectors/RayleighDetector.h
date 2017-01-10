#pragma once

#include <cmath>
#include <iostream>
#include "Detector.h"

using namespace std;

class RayleighDetector : public Detector {
public:

protected:
	virtual void estimatePdfParameters(double* clutterValues, const int numberOfClutterValues)
	{
		b = 0.0;

		for(int i=0; i<numberOfClutterValues; i++) {
			b += clutterValues[i] * clutterValues[i];
		}

		b /= numberOfClutterValues;
		b = sqrt(b / 2);
	}
	
	virtual double estimateThreshold(const double probabilityOfFalseAlarm)
	{
		return sqrt(- 2 * b * b * log(probabilityOfFalseAlarm) );
	}

private:
	double b;
};
