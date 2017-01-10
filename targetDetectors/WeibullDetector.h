#pragma once

#include "Detector.h"
#include <cmath>
#include "StatisticalUtils.h"

class WeibullDetector : public Detector {
public:

protected:
	virtual void estimatePdfParameters(double* clutterValues, const int numberOfClutterValues)
	{
		logData(clutterValues, numberOfClutterValues);
		EstimateExtremeValueDistributionParams(clutterValues, numberOfClutterValues, lambda, k);
		lambda = exp(lambda);
		k = 1/k;
	}

	virtual double estimateThreshold(const double probabilityOfFalseAlarm)
	{
		return lambda*( pow(-log(probabilityOfFalseAlarm), 1/k) );
	}

private:
	double k;
	double lambda;

	inline void logData(double* clutterValues, const int& numberOfClutterValues)
	{
		for(int i=0; i<numberOfClutterValues; i++)
		{
			clutterValues[i] = log(clutterValues[i]);
		}
	}

	inline void EstimateExtremeValueDistributionParams(double* clutterValues, const int& numberOfClutterValues, double& param1, double& param2)
	{
		///const double alpha = 0.99;
		double maxVal, range;

		ShiftDataBetweenMinusOneToZero(clutterValues, numberOfClutterValues, maxVal, range);
		double std, mean;
		meanAndStd<double>(clutterValues, numberOfClutterValues, mean, std, true);
		double sigmaHat = sqrt(6.0)*std/M_PI;
		//double wgtmeanUnc = mean;

		double upper, lower;

		if(LikelihoodForExtremeValueScaleParam(clutterValues, numberOfClutterValues,  mean, sigmaHat) > 0)
		{
			upper = sigmaHat;
			lower = 0.5*upper;
			while(LikelihoodForExtremeValueScaleParam(clutterValues, numberOfClutterValues,  mean, lower) > 0 )
			{
				upper = lower;
				lower = 0.5*upper;
			}
		}
		else
		{
			lower = sigmaHat;
			upper = 2*lower;
			while( LikelihoodForExtremeValueScaleParam(clutterValues, numberOfClutterValues,  mean, upper) < 0 )
			{
				lower = upper;
				upper = 2*lower;
			}
		}

		sigmaHat = FindRootOfLikelihoodFuncInRange(clutterValues, numberOfClutterValues,  mean, lower, upper);
		double muHat = 0.0;
		for(int i=0; i<numberOfClutterValues;i++)
		{
			muHat += exp(clutterValues[i]/sigmaHat);
		}
		muHat = sigmaHat * log( muHat );

		param1 = range*muHat+maxVal;
		param2 = range*sigmaHat;
	}

	inline void ShiftDataBetweenMinusOneToZero(double* clutterValues, const int& numberOfClutterValues, double& maxData, double& range)
	{
		double minData = clutterValues[0];
		maxData = clutterValues[0];

		for(int i=1; i<numberOfClutterValues; i++)
		{
			if(clutterValues[i] > maxData)
			{
				maxData = clutterValues[i];
			}
			else if(clutterValues[i] < minData)
			{
				minData = clutterValues[i];
			}
		}

		range = maxData - minData;

		for(int i=1; i<numberOfClutterValues; i++)
		{
			clutterValues[i] = (clutterValues[i]-maxData)/range;
		}
	}

	inline double LikelihoodForExtremeValueScaleParam(double* clutterValues, const int& numberOfClutterValues,  double mu, double sigma)
	{
		double sumTerm = 0.0;

		for(int i=1; i<numberOfClutterValues; i++)
		{
			sumTerm += clutterValues[i]*exp(clutterValues[i]/sigma);
		}

		return (sigma + mu - (1/numberOfClutterValues)*sumTerm);
	}

	inline double FindRootOfLikelihoodFuncInRange(double* clutterValues, const int& numberOfClutterValues, double mu, const double& lower, const double& upper)
	{
		const double valTol = 1e-6;
		const double gapTol = 0.1;

		double sigmaUp = upper;
		double sigmaLow = lower;

		double sigma;

		while(true)
		{
			double valUp = abs(LikelihoodForExtremeValueScaleParam(clutterValues, numberOfClutterValues, mu, sigmaUp));
			double valLow = abs(LikelihoodForExtremeValueScaleParam(clutterValues, numberOfClutterValues, mu, sigmaLow));
			if(valUp < valLow)
			{
				sigmaLow = 0.5*(sigmaUp+sigmaLow);
				if(valUp < valTol)
				{
					sigma = sigmaUp;
					break;
				}
			}
			else
			{
				sigmaUp = 0.5*(sigmaUp+sigmaLow);
				if(valLow < valTol)
				{
					sigma = sigmaLow;
					break;
				}
			}

			if(abs(sigmaUp-sigmaLow) < gapTol)
			{
				sigma =  0.5*(sigmaUp+sigmaLow);
				break;
			}
		}

		return sigma;
	}
};
