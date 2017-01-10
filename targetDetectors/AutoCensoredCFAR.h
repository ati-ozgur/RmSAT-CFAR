#pragma once

#include <opencv2\opencv.hpp>
#include "WindowBasedCFAR.h"
#include "QuickSelect.h"

using namespace cv;
using namespace std;


class AutoCensoredCFAR : public WindowBasedCFAR {
public:	
	AutoCensoredCFAR()
	{
		bool orderClutterRegions = true;
	}

	virtual AbstractCFAR* clone()
	{
		return new AutoCensoredCFAR;
	}

protected:
	virtual bool checkTargetExistance(double probabilityOfFalseAlarm, Detector* detector, double* candidateRegion, const int& numCandidatePixels, double* clutterRegion, const int& numClutterPixels, const int& numRegion1, const int& numRegion2, const int& numRegion3, const int& numRegion4)
	{
		bool result = false;
				
		double meanVal = 0.0;
		for(int i=0; i<numCandidatePixels; i++)
		{
			meanVal += candidateRegion[i];
		}
		meanVal /= numCandidatePixels;

		int numClutterPixelsToUseInThresholdEst = (int)(numClutterPixels * osPercent / 100.0 + 0.5);
		quickSelect<double>(clutterRegion, numClutterPixels, numClutterPixelsToUseInThresholdEst);
		
		result = detector->detect(meanVal, clutterRegion, numClutterPixelsToUseInThresholdEst, probabilityOfFalseAlarm);
		
		return result;
	}
private:

};
