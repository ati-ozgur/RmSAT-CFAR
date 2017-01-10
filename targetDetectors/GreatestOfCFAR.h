#pragma once

#include "Node.h"
#include "Parameters.h"
#include "DataUtils.h"
#include "WindowBasedCFAR.h"
#include "opencv2\opencv.hpp"

using namespace cv;
using namespace framework;

class GreatestOfCFAR : public WindowBasedCFAR {

public:	
	GreatestOfCFAR(string name, Node* parent = NULL) : WindowBasedCFAR(name, parent)
	{
	}

protected:
	virtual bool CheckTargetExistance(Detector* detector, double* candidateRegion, const int& numCandidatePixels, double* clutterRegion, const int& numClutterPixels, const int& numRegion1, const int& numRegion2, const int& numRegion3, const int& numRegion4)
	{
		bool result = false; 
		
		double maxVal = DBL_MIN;
		for(int i=0; i<numCandidatePixels; i++)
		{
			if(candidateRegion[i] > maxVal)
			{
				maxVal = candidateRegion[i];
			}
		}

		result = detector->detect(maxVal, clutterRegion, numClutterPixels, probabilityOfFalseAlarm);
		
		return result;
	}
};
