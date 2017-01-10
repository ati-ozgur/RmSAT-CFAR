#pragma once

#include "Node.h"
#include "Parameters.h"
#include "DataUtils.h"
#include "WindowBasedCFAR.h"
#include "opencv2\opencv.hpp"

using namespace cv;
using namespace framework;

class SmallestOfCFAR : public WindowBasedCFAR {

public:	
	SmallestOfCFAR(string name, Node* parent = NULL) : WindowBasedCFAR(name, parent)
	{
	}

protected:
	virtual bool CheckTargetExistance(Detector* detector, double* candidateRegion, const int& numCandidatePixels, double* clutterRegion, const int& numClutterPixels, const int& numRegion1, const int& numRegion2, const int& numRegion3, const int& numRegion4)
	{
		bool result = false; 
		
		double minVal = DBL_MAX;
		for(int i=0; i<numCandidatePixels; i++)
		{
			if(candidateRegion[i] < minVal)
			{
				minVal = candidateRegion[i];
			}
		}

		result = detector->detect(minVal, clutterRegion, numClutterPixels, probabilityOfFalseAlarm);
		
		return result;
	}
};
