#pragma once

#include <opencv2\opencv.hpp>
#include "WindowBasedCFAR.h"

using namespace cv;
using namespace std;


#define	K_VI  3.0	// Variability Threshold
#define K_MS  0.8	// Mean Similarity Threshold

//VI-CFAR: A Novel CFAR Algorithm Based on Data Variability, Smith, M.E., Varshney, P.K., 1997.
class VariabilityIndexCFAR : public WindowBasedCFAR {
public:	
	VariabilityIndexCFAR()
	{
		orderClutterRegions = true;
	}

	virtual AbstractCFAR* clone()
	{
		return new VariabilityIndexCFAR;
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

		double meanRegion1, meanRegion2, meanRegion3, meanRegion4;
		double variabilityIndex1, variabilityIndex2, variabilityIndex3, variabilityIndex4;

		double* region = clutterRegion;
		calcHypothesisParams(region, numRegion1, meanRegion1, variabilityIndex1);
		region += numRegion1;
		calcHypothesisParams(region, numRegion2, meanRegion2, variabilityIndex2);
		region += numRegion2;
		calcHypothesisParams(region, numRegion3, meanRegion3, variabilityIndex3);
		region += numRegion3;
		calcHypothesisParams(region, numRegion4, meanRegion4, variabilityIndex4);

		double* selectedClutterPixels;
		int numSelectedClutterPixels;
		selectClutterRegion(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, variabilityIndex1, variabilityIndex2, variabilityIndex3, variabilityIndex4, selectedClutterPixels, numSelectedClutterPixels);

		/*cout << "Selected Region Pointer			: " << selectedClutterPixels << endl;
		cout << "Number of Pixels in Sleected Region: " << numSelectedClutterPixels << endl;
		cout << endl;*/

		result = detector->detect(meanVal, selectedClutterPixels, numSelectedClutterPixels, probabilityOfFalseAlarm);
		
		return result;
	}

private:
	void calcHypothesisParams(double* region, const int& numOfPixelsInRegion, double& mean, double& variabilityIndex)
	{
		double sum = 0.0;
		double squareSum = 0.0;

		for(int i=0; i<numOfPixelsInRegion; i++)
		{
			sum += region[i];
			squareSum += (region[i]*region[i]);
		}

		mean = sum/numOfPixelsInRegion;
		if(sum > 0)
		{
			variabilityIndex = numOfPixelsInRegion * squareSum / (sum*sum);
		}
		else
		{
			variabilityIndex = DBL_MAX;
		}
	}

	void selectClutterRegion(double* clutterRegion, const int& numRegion1, const int& numRegion2, const int& numRegion3, const int& numRegion4, 
							 const double& meanRegion1, const double& meanRegion2, const double& meanRegion3, const double& meanRegion4, 
							 const double& variabilityIndex1, const double& variabilityIndex2, const double& variabilityIndex3, const double& variabilityIndex4, 
							 double*& selectedClutterPixels, int& numSelectedClutterPixels)
	{
		bool interestedRegions[4] = {false, false, false, false};

		double meanRatio, meanRatio1, meanRatio2, meanRatio3;

		vector<double*> regionStartPts(4);
		regionStartPts.at(0) = clutterRegion;
		regionStartPts.at(1) = clutterRegion + numRegion1;
		regionStartPts.at(2) = regionStartPts.at(1) + numRegion2;
		regionStartPts.at(3) = regionStartPts.at(2) + numRegion3;

		vector<int> numRegionElems(4);
		numRegionElems.at(0) = numRegion1;
		numRegionElems.at(1) = numRegion2;
		numRegionElems.at(2) = numRegion3;
		numRegionElems.at(3) = numRegion4;

		int variabilityCond = 0;
		if(variabilityIndex1 < K_VI)
			variabilityCond += 1;
		if(variabilityIndex2 < K_VI)
			variabilityCond += 2;
		if(variabilityIndex3 < K_VI)
			variabilityCond += 4;
		if(variabilityIndex4 < K_VI)
			variabilityCond += 8;

		/*cout << "Region 1: " << endl;
		cout << "	Mean			  : " << meanRegion1 << endl;
		cout << "	VI  			  : " << variabilityIndex1 << endl; 
		cout << "	Start Pointer	  : " << regionStartPts.at(0) << endl;
		cout << "	Number of Elements: " << numRegionElems.at(0) << endl;
		cout << "Region 2: " << endl;
		cout << "	Mean			  : " << meanRegion2 << endl;
		cout << "	VI  			  : " << variabilityIndex2 << endl; 
		cout << "	Start Pointer	  : " << regionStartPts.at(1) << endl;
		cout << "	Number of Elements: " << numRegionElems.at(1) << endl;
		cout << "Region 3: " << endl;
		cout << "	Mean			  : " << meanRegion3 << endl;
		cout << "	VI  			  : " << variabilityIndex3 << endl; 
		cout << "	Start Pointer	  : " << regionStartPts.at(2) << endl;
		cout << "	Number of Elements: " << numRegionElems.at(2) << endl;
		cout << "Region 4: " << endl;
		cout << "	Mean			  : " << meanRegion4 << endl;
		cout << "	VI  			  : " << variabilityIndex4 << endl; 
		cout << "	Start Pointer	  : " << regionStartPts.at(3) << endl;
		cout << "	Number of Elements: " << numRegionElems.at(3) << endl;

		cout << "Variablity Condition: " << variabilityCond << endl;*/
		
		switch (variabilityCond)
		{
		case 0:
			interestedRegions[0] = true;
			interestedRegions[1] = true;
			interestedRegions[2] = true;
			interestedRegions[3] = true;
			selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			break;
		case 1:
			selectedClutterPixels = clutterRegion;
			numSelectedClutterPixels = numRegion1;
			break;
		case 2:
			selectedClutterPixels = clutterRegion + numRegion1;
			numSelectedClutterPixels = numRegion2;
			break;
		case 3:
			meanRatio = min(meanRegion1/meanRegion2, meanRegion2/meanRegion1);
			if(meanRatio >= K_MS)
			{
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion2;
			}
			else
			{
				interestedRegions[0] = true;
				interestedRegions[1] = true;
				selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			}
			break;
		case 4:
			selectedClutterPixels = clutterRegion + numRegion1 + numRegion2;
			numSelectedClutterPixels = numRegion3;
			break;
		case 5:
			meanRatio = min(meanRegion1/meanRegion3, meanRegion3/meanRegion1);
			if(meanRatio >= K_MS)
			{
				swapRegions(regionStartPts, numRegionElems, 2, 3);
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion3;
			}
			else
			{
				interestedRegions[0] = true;
				interestedRegions[2] = true;
				selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			}
			break;
		case 6:
			meanRatio = min(meanRegion2/meanRegion3, meanRegion3/meanRegion2);
			if(meanRatio >= K_MS)
			{
				selectedClutterPixels = clutterRegion + numRegion1;
				numSelectedClutterPixels = numRegion2 + numRegion3;
			}
			else
			{
				interestedRegions[1] = true;
				interestedRegions[2] = true;
				selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			}
			break;
		case 7:
			meanRatio1 = min(meanRegion1/meanRegion2, meanRegion2/meanRegion1);
			meanRatio2 = min(meanRegion1/meanRegion3, meanRegion3/meanRegion1);
			meanRatio3 = min(meanRegion3/meanRegion2, meanRegion2/meanRegion3);

			if(meanRatio1 >= K_MS && meanRatio2 < K_MS && meanRatio3 < K_MS)
			{
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion2;
			}
			else if(meanRatio1 < K_MS && meanRatio2 >= K_MS && meanRatio3 < K_MS)
			{
				swapRegions(regionStartPts, numRegionElems, 2, 3);
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion3;
			}
			else if(meanRatio1 < K_MS && meanRatio2 < K_MS && meanRatio3 >= K_MS)
			{
				selectedClutterPixels = clutterRegion + numRegion1;
				numSelectedClutterPixels = numRegion2 + numRegion3;
			}
			else if(meanRatio1 < K_MS && meanRatio2 < K_MS && meanRatio3 < K_MS)
			{
				interestedRegions[0] = true;
				interestedRegions[1] = true;
				interestedRegions[2] = true;
				selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			}
			else
			{
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion2 + numRegion3;
			}
			break;
		case 8:
			selectedClutterPixels = clutterRegion + numRegion1 + numRegion2 + numRegion3;
			numSelectedClutterPixels = numRegion4;
			break;
		case 9:
			meanRatio = min(meanRegion1/meanRegion4, meanRegion4/meanRegion1);
			if(meanRatio >= K_MS)
			{
				swapRegions(regionStartPts, numRegionElems, 2, 4);
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion4;
			}
			else
			{
				interestedRegions[0] = true;
				interestedRegions[3] = true;
				selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			}
			break;
		case 10:
			meanRatio = min(meanRegion2/meanRegion4, meanRegion4/meanRegion2);
			if(meanRatio >= K_MS)
			{
				swapRegions(regionStartPts, numRegionElems, 3, 4);
				selectedClutterPixels = clutterRegion + numRegion1;
				numSelectedClutterPixels = numRegion2 + numRegion4;
			}
			else
			{
				interestedRegions[1] = true;
				interestedRegions[3] = true;
				selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			}
			break;
		case 11:
			meanRatio1 = min(meanRegion1/meanRegion2, meanRegion2/meanRegion1);
			meanRatio2 = min(meanRegion1/meanRegion4, meanRegion4/meanRegion1);
			meanRatio3 = min(meanRegion2/meanRegion4, meanRegion4/meanRegion2);

			if(meanRatio1 >= K_MS && meanRatio2 < K_MS && meanRatio3 < K_MS)
			{
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion2;
			}
			else if(meanRatio1 < K_MS && meanRatio2 >= K_MS && meanRatio3 < K_MS)
			{
				swapRegions(regionStartPts, numRegionElems, 2, 4);
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion4;
			}
			else if(meanRatio1 < K_MS && meanRatio2 < K_MS && meanRatio3 >= K_MS)
			{
				swapRegions(regionStartPts, numRegionElems, 3, 4);
				selectedClutterPixels = clutterRegion + numRegion1;
				numSelectedClutterPixels = numRegion2 + numRegion4;
			}
			else if(meanRatio1 < K_MS && meanRatio2 < K_MS && meanRatio3 < K_MS)
			{
				interestedRegions[0] = true;
				interestedRegions[1] = true;
				interestedRegions[3] = true;
				selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			}
			else
			{
				swapRegions(regionStartPts, numRegionElems, 3, 4);
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion2 + numRegion4;
			}
			break;
		case 12:
			meanRatio = min(meanRegion3/meanRegion4, meanRegion4/meanRegion3);
			if(meanRatio >= K_MS)
			{
				selectedClutterPixels = clutterRegion + numRegion1 + numRegion2;
				numSelectedClutterPixels = numRegion3 + numRegion4;
			}
			else
			{
				interestedRegions[2] = true;
				interestedRegions[3] = true;
				selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			}
			break;
		case 13:
			meanRatio1 = min(meanRegion1/meanRegion3, meanRegion3/meanRegion1);
			meanRatio2 = min(meanRegion1/meanRegion4, meanRegion4/meanRegion1);
			meanRatio3 = min(meanRegion3/meanRegion4, meanRegion4/meanRegion3);

			if(meanRatio1 >= K_MS && meanRatio2 < K_MS && meanRatio3 < K_MS)
			{
				swapRegions(regionStartPts, numRegionElems, 2, 3);
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion3;
			}
			else if(meanRatio1 < K_MS && meanRatio2 >= K_MS && meanRatio3 < K_MS)
			{
				swapRegions(regionStartPts, numRegionElems, 2, 4);
				selectedClutterPixels = clutterRegion;
				numSelectedClutterPixels = numRegion1 + numRegion4;
			}
			else if(meanRatio1 < K_MS && meanRatio2 < K_MS && meanRatio3 >= K_MS)
			{
				selectedClutterPixels = clutterRegion + numRegion1 + numRegion2;
				numSelectedClutterPixels = numRegion3 + numRegion4;
			}
			else if(meanRatio1 < K_MS && meanRatio2 < K_MS && meanRatio3 < K_MS)
			{
				interestedRegions[0] = true;
				interestedRegions[2] = true;
				interestedRegions[3] = true;
				selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			}
			else
			{
				swapRegions(regionStartPts, numRegionElems, 1, 2);
				selectedClutterPixels = clutterRegion + numRegion2;
				numSelectedClutterPixels = numRegion1 + numRegion3 + numRegion4;
			}
			break;
		case 14:
			meanRatio1 = min(meanRegion2/meanRegion3, meanRegion3/meanRegion2);
			meanRatio2 = min(meanRegion2/meanRegion4, meanRegion4/meanRegion2);
			meanRatio3 = min(meanRegion3/meanRegion4, meanRegion4/meanRegion3);

			if(meanRatio1 >= K_MS && meanRatio2 < K_MS && meanRatio3 < K_MS)
			{
				selectedClutterPixels = clutterRegion + numRegion1;
				numSelectedClutterPixels = numRegion2 + numRegion3;
			}
			else if(meanRatio1 < K_MS && meanRatio2 >= K_MS && meanRatio3 < K_MS)
			{
				swapRegions(regionStartPts, numRegionElems, 3, 4);
				selectedClutterPixels = clutterRegion + numRegion1;
				numSelectedClutterPixels = numRegion2 + numRegion4;
			}
			else if(meanRatio1 < K_MS && meanRatio2 < K_MS && meanRatio3 >= K_MS)
			{
				selectedClutterPixels = clutterRegion + numRegion1 + numRegion2;
				numSelectedClutterPixels = numRegion3 + numRegion4;
			}
			else if(meanRatio1 < K_MS && meanRatio2 < K_MS && meanRatio3 < K_MS)
			{
				interestedRegions[1] = true;
				interestedRegions[2] = true;
				interestedRegions[3] = true;
				selectRegionWithMinFeature(clutterRegion, numRegion1, numRegion2, numRegion3, numRegion4, meanRegion1, meanRegion2, meanRegion3, meanRegion4, interestedRegions, selectedClutterPixels, numSelectedClutterPixels);
			}
			else
			{
				selectedClutterPixels = clutterRegion + numRegion1;
				numSelectedClutterPixels = numRegion2 + numRegion3 + numRegion4;
			}
			break;
		case 15:
			selectedClutterPixels = clutterRegion;
			numSelectedClutterPixels = numRegion1 + numRegion2 + numRegion3 + numRegion4;
			break;
		default:
			selectedClutterPixels = clutterRegion;
			numSelectedClutterPixels = numRegion1 + numRegion2 + numRegion3 + numRegion4;
		}


		/*cout << "Selected Region Pointer			: " << selectedClutterPixels << endl;
		cout << "Number of Pixels in Sleected Region: " << numSelectedClutterPixels << endl;*/
	}

	void selectRegionWithMinFeature(double* clutterRegion, const int& numRegion1, const int& numRegion2, const int& numRegion3, const int& numRegion4, 
									const double& feature1, const double& feature2, const double& feature3, const double& feature4, bool interestedRegions[4],
									double*& selectedClutterPixels, int& numSelectedClutterPixels)
	{
		double minFeature = DBL_MAX;
		
		if(interestedRegions[0])
		{
			minFeature = feature1;
			selectedClutterPixels = clutterRegion;
			numSelectedClutterPixels = numRegion1;
		}

		if(interestedRegions[1] && feature2 < minFeature)
		{
			minFeature = feature2;
			selectedClutterPixels = clutterRegion + numRegion1;
			numSelectedClutterPixels = numRegion2;
		}

		if(interestedRegions[2] && feature3 < minFeature)
		{
			minFeature = feature3;
			selectedClutterPixels = clutterRegion + numRegion1 + numRegion2;
			numSelectedClutterPixels = numRegion3;
		}

		if(interestedRegions[3] && feature4 < minFeature)
		{
			minFeature = feature4;
			selectedClutterPixels = clutterRegion + numRegion1 + numRegion2 + numRegion3;
			numSelectedClutterPixels = numRegion4;
		}
	}

	void swapRegions(const vector<double*>& regionStartPts, const vector<int>& numRegionElems, int region1, int region2)
	{		
		region1 -= 1;
		region2 -= 1;

		double* reg1 = regionStartPts.at(region1+1);
		double* reg2 = regionStartPts.at(region2);

		for(int i=0; i<numRegionElems.at(region2); i++)
		{
			swap(reg1[i], reg2[i]);
		}
	}
};
