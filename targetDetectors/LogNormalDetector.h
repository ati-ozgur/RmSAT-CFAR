#pragma once

#include <cmath>
#include "GaussianDetector.h"

class LogNormalDetector : public GaussianDetector {
public:

protected:
	virtual void estimatePdfParameters(double* clutterValues, const int numberOfClutterValues)
	{
		double value;

		mu = 0.0;
		sigma = 0.0;

		for(int i=0; i<numberOfClutterValues; i++) {
			value = log(clutterValues[i]);
			mu += value;
			sigma += value * value;
		}

		mu /= numberOfClutterValues;
		sigma /= (numberOfClutterValues - 1);
		sigma -= numberOfClutterValues * mu * mu / (numberOfClutterValues - 1);
		sigma = sqrt(sigma);
	}
	
	virtual bool testValue(const double valueToTest, const double threshold)
	{
		return (log(valueToTest) > threshold);
	}

private:
};
