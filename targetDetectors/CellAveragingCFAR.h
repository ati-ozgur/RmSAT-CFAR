#pragma once

#include <opencv2\opencv.hpp>
#include "WindowBasedCFAR.h"
#include "Detector.h"

using namespace std;
using namespace cv;


class CellAveragingCFAR : public WindowBasedCFAR {
public:	
	CellAveragingCFAR(ClutterDistribution clutterDistribution = Gaussian) : WindowBasedCFAR(clutterDistribution)
	{
		this->clutterDistribution = clutterDistribution;
		bool orderClutterRegions = false;
	}

	virtual AbstractCFAR* clone()
	{
		return new CellAveragingCFAR;
	}

protected:
	virtual bool checkTargetExistance(double probabilityOfFalseAlarm, Detector* detector, double* candidateRegion, const int& numCandidatePixels, double* clutterRegion, const int& numClutterPixels, const int& numRegion1, const int& numRegion2, const int& numRegion3, const int& numRegion4)
	{
		double meanVal = 0.0;
		for(int i=0; i<numCandidatePixels; i++)
		{
			meanVal += candidateRegion[i];
		}
		meanVal /= numCandidatePixels;

		const bool result = detector->detect(meanVal, clutterRegion, numClutterPixels, probabilityOfFalseAlarm);
		
		return result;
	}
};
