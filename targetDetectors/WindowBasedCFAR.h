#pragma once

#include <opencv2\opencv.hpp>
#include "AbstractCFAR.h"
#include "Detector.h"
#include "GaussianDetector.h"
#include "LogNormalDetector.h"
#include "G0Detector.h"
#include "GammaDetector.h"
#include "RayleighDetector.h"
#include "WeibullDetector.h"

using namespace cv;
using namespace std;


class WindowBasedCFAR : public AbstractCFAR {
public:
	enum ClutterDistribution {Unknown=0, Gaussian=1, LogNormal=2, Rayleigh=3, G0=4, Gamma=5, Weibull=6};

	WindowBasedCFAR(ClutterDistribution clutterDistribution = Gaussian)
	{
		this->clutterDistribution = clutterDistribution;
		bool orderClutterRegions = false;
	}

	virtual ~WindowBasedCFAR()
	{

	}

	virtual Mat execute(Mat image, double probabilityOfFalseAlarm, map<string, double>& parameters)
	{
		this->targetRadius = (int)getParameterValue(parameters, "WB-CFAR.targetRadius", 2);
		this->guardRadius = (int)getParameterValue(parameters, "WB-CFAR.guardRadius", 3);
		this->clutterRadius = (int)getParameterValue(parameters, "WB-CFAR.clutterRadius", 5);
		this->osPercent = getParameterValue(parameters, "AC-CFAR.censoringPercentile", 99.0);

		const bool usePowerImage = (clutterDistribution == G0);
		if (usePowerImage) {
			image.convertTo(image, CV_32F);
			image = image.mul(image);
		}

		Mat targetImage(image.rows, image.cols, CV_8UC1);

		switch (image.depth())
		{
		case CV_8U:  detectTargets<unsigned char>(image, targetImage, probabilityOfFalseAlarm, clutterDistribution);	break;
		case CV_8S:  detectTargets<char>(image, targetImage, probabilityOfFalseAlarm, clutterDistribution);				break;
		case CV_16U: detectTargets<unsigned short>(image, targetImage, probabilityOfFalseAlarm, clutterDistribution);	break;
		case CV_16S: detectTargets<short>(image, targetImage, probabilityOfFalseAlarm, clutterDistribution);			break;
		case CV_32S: detectTargets<int>(image, targetImage, probabilityOfFalseAlarm, clutterDistribution);				break;
		case CV_32F: detectTargets<float>(image, targetImage, probabilityOfFalseAlarm, clutterDistribution);			break;
		case CV_64F: detectTargets<double>(image, targetImage, probabilityOfFalseAlarm, clutterDistribution);			break;
		default: targetImage = Scalar(0);
		}

		return targetImage;
	}

	virtual int getClutterArea(map<string, double>& parameters)
	{
		this->targetRadius = (int)parameters["WB-CFAR.targetRadius"];
		this->guardRadius = (int)parameters["WB-CFAR.guardRadius"];
		this->clutterRadius = (int)parameters["WB-CFAR.clutterRadius"];
		const int windowRadius = (targetRadius + guardRadius + clutterRadius);

		const int clutterArea = sqr(2 * windowRadius + 1) - sqr(2 * (guardRadius + targetRadius) + 1);

		return clutterArea;
	}

	virtual int getBandWidth(map<string, double>& parameters)
	{
		this->targetRadius = (int)parameters["WB-CFAR.targetRadius"];
		this->guardRadius = (int)parameters["WB-CFAR.guardRadius"];
		this->clutterRadius = (int)parameters["WB-CFAR.clutterRadius"];
		const int windowRadius = (targetRadius + guardRadius + clutterRadius);

		return calculateBandSize(windowRadius);
	}

protected:
	ClutterDistribution clutterDistribution;
	bool orderClutterRegions;
	int targetRadius;
	int guardRadius;
	int clutterRadius;
	double osPercent;


	template<typename T>
	static inline T sqr(T x)
	{
		return x * x;
	}

	template<typename T>
	void detectTargets(Mat& image, Mat& targetImage, double probabilityOfFalseAlarm, ClutterDistribution clutterDistribution)
	{
		const int threadCount = min(getThreadCount(), image.rows);

		Detector* detector;
		double* candidateRegion;
		double* clutterRegion;
		T* imageDataRow;
		unsigned char* targetImageRow;
		int x, y, numCandidatePixels, numClutterPixels, limit1, limit2, limit3, numRegion1, numRegion2, numRegion3, numRegion4;
		#pragma omp parallel private(detector, candidateRegion, clutterRegion, limit1, limit2, limit3) num_threads(threadCount)
		{
			detector = createDetector(clutterDistribution);

			createCFARRegions(candidateRegion, clutterRegion, limit1, limit2, limit3);

			#pragma omp for private(x, y, numCandidatePixels, numClutterPixels, numRegion1, numRegion2, numRegion3, numRegion4, imageDataRow, targetImageRow)
			for (y=0; y<image.rows; y++) {
				imageDataRow = (T*)(image.data + y * image.step);
				targetImageRow = (unsigned char*)(targetImage.data + y * targetImage.step);
			
				for (x=0; x<image.cols; x++) {
					targetImageRow[x] = 0;

					if (imageDataRow[x] > 0) {
						numCandidatePixels = 0;
						numClutterPixels = 0;
						getRegionPixels<T>(image, x, y, candidateRegion, numCandidatePixels, clutterRegion, numClutterPixels, limit1, limit2, limit3, numRegion1, numRegion2, numRegion3, numRegion4);
							
						if (checkTargetExistance(probabilityOfFalseAlarm, detector, candidateRegion, numCandidatePixels, clutterRegion, numClutterPixels, numRegion1, numRegion2, numRegion3, numRegion4)) {
							targetImageRow[x] = UCHAR_MAX;
						}
					}
				}
			}

			removeCFARRegions(candidateRegion, clutterRegion);

			removeDetectors(detector);
		}
	}

	virtual Detector* createDetector(ClutterDistribution clutterDistribution)
	{
		switch (clutterDistribution )
		{
		case Gaussian	: return new GaussianDetector();	break;
		case LogNormal	: return new LogNormalDetector();	break;
		case Rayleigh	: return new RayleighDetector();	break;
		case G0			: return new G0Detector();			break;
		case Gamma		: return new GammaDetector();		break;
		case Weibull	: return new WeibullDetector();		break;
		default: return NULL;
		}
	}

	virtual void removeDetectors(Detector* detector)
	{
		if (detector != NULL) {
			delete detector;
		}
	}

	virtual void createCFARRegions(double*& candidateRegion, double*& clutterRegion, int& limit1, int& limit2, int& limit3)
	{
		limit1 = targetRadius + guardRadius + clutterRadius;
		limit2 = targetRadius + guardRadius;
		limit3 = targetRadius;

		int maxCandidatePixels = sqr(2 * targetRadius + 1);
		int maxClutterPixels = sqr(2 * limit1 + 1) - sqr(2 * limit2 + 1);

		candidateRegion = new double[maxCandidatePixels];
		clutterRegion = new double[maxClutterPixels];
	}

	virtual void removeCFARRegions(double*& candidateRegion, double*& clutterRegion)
	{
		delete candidateRegion;
		delete clutterRegion;
	}

	template<typename T>
	void getRegionPixels(Mat& image, const int x, const int y, 
						 double* candidateRegion, int& numCandidatePixels, double* clutterRegion, int& numClutterPixels, 
						 const int& limit1, const int& limit2, const int& limit3, 
						 int& numRegion1, int& numRegion2, int& numRegion3, int& numRegion4)
						 //Last 4 parameters are only used in VI-CFAR
	{
		numRegion1 = -9999;
		numRegion2 = -9999;
		numRegion3 = -9999;
		numRegion4 = -9999;

		if(orderClutterRegions)
			getRegionPixelsClutterOrdered<T>(image, x, y, candidateRegion, numCandidatePixels, clutterRegion, numClutterPixels, limit1, limit2, limit3, numRegion1, numRegion2, numRegion3, numRegion4);
		else
			getRegionPixelsClutterDisordered<T>(image, x, y, candidateRegion, numCandidatePixels, clutterRegion, numClutterPixels, limit1, limit2, limit3);
	}

	template<typename T>
	void getRegionPixelsClutterDisordered(Mat& image, const int x, const int y,
									double* candidateRegion, int& numCandidatePixels, double* clutterRegion, int& numClutterPixels, 
									const int& limit1, const int& limit2, const int& limit3)
	{
		int jMin = max(0, y-limit1);
		int jMax = min(y-limit2-1, image.rows-1);
		int iMin = max(0, x-limit1);
		int iMax = min(x+limit1, image.cols-1);

		for(int j=jMin; j<=jMax; j++)
		{
			T* imageRow = (T*)(image.data + j * image.step);

			for(int i=iMin; i<=iMax; i++)
			{
				clutterRegion[numClutterPixels] = imageRow[i];
				numClutterPixels++;
			}
		}

		jMin = max(0, y-limit2);
		jMax = min(y+limit2, image.rows-1);
		for(int j=jMin; j<=jMax; j++)
		{
			T* imageRow = (T*)(image.data + j * image.step);

			iMin = max(0, x-limit1);
			iMax = min(x-limit2-1, image.cols-1);
			for(int i=iMin; i<=iMax; i++)
			{
				clutterRegion[numClutterPixels] = imageRow[i];
				numClutterPixels++;
			}

			iMin = max(0, x+limit2+1);
			iMax = min(x+limit1, image.cols-1);
			for(int i=iMin; i<=iMax; i++)
			{
				clutterRegion[numClutterPixels] = imageRow[i];
				numClutterPixels++;
			}

			if(j>=y-limit3 && j<=y+limit3)
			{
				iMin = max(0, x-limit3);
				iMax = min(x+limit3, image.cols-1);
				for(int i=iMin; i<=iMax; i++)
				{
					candidateRegion[numCandidatePixels] = imageRow[i];
					numCandidatePixels++;
				}
			}
		}

		jMin = max(0, y+limit2+1);
		jMax = min(y+limit1, image.rows-1);
		iMin = max(0, x-limit1);
		iMax = min(x+limit1, image.cols-1);

		for(int j=jMin; j<=jMax; j++)
		{
			T* imageRow = (T*)(image.data + j * image.step);

			for(int i=iMin; i<=iMax; i++)
			{
				clutterRegion[numClutterPixels] = imageRow[i];
				numClutterPixels++;
			}
		}
	}

	template<typename T>
	void getRegionPixelsClutterOrdered(Mat& image, const int x, const int y,
									   double* candidateRegion, int& numCandidatePixels, double* clutterRegion, int& numClutterPixels, 
									   const int& limit1, const int& limit2, const int& limit3,
									   int& numRegion1, int& numRegion2, int& numRegion3, int& numRegion4)
	{
		T* imageRow;

		int iMin, iMax, iMin1, iMax1, iMin2, iMax2, iMin3, iMax3;
		int jMin, jMax;
		int i,j;
		int numClutterPixelsInRegion1 = 0;
		int numClutterPixelsInRegion2 = 0;
		int numClutterPixelsInRegion3 = 0;
		int numClutterPixelsInRegion4 = 0;

		calculateNumberOfElementsInClutterRegions(x, y, image.rows, image.cols, limit1, limit2, limit3, numRegion1, numRegion2, numRegion3, numRegion4);
		double* clutterRegion1 = clutterRegion;
		double* clutterRegion2 = clutterRegion1 + numRegion1;
		double* clutterRegion3 = clutterRegion2 + numRegion2;
		double* clutterRegion4 = clutterRegion3 + numRegion3;


		jMin = max(0, y-limit1);
		jMax = min(y-limit2-1, image.rows-1);
		iMin = max(0, x-limit1);
		iMax = min(x+limit1, image.cols-1);
		//Region1 Limits
		iMin1 = iMin;
		iMax1 = min(x+limit2, iMax);
		//Region2Limits
		iMin2 = iMax1 + 1;
		iMax2 = iMax;

		for(j=jMin; j<=jMax; j++)
		{
			imageRow = (T*)(image.data + j*image.step);
			
			for(i=iMin1; i<=iMax1; i++)
			{
				clutterRegion1[numClutterPixelsInRegion1] = imageRow[i];	
				numClutterPixels++;
				numClutterPixelsInRegion1++;
			}
			for(i=iMin2; i<=iMax2; i++)
			{
				clutterRegion2[numClutterPixelsInRegion2] = imageRow[i];	
				numClutterPixels++;
				numClutterPixelsInRegion2++;
			}
		}

		jMin = max(0, y-limit2);
		jMax = min(y+limit2, image.rows-1);
		//Region 3 Limits
		iMin1 = max(0, x-limit1);
		iMax1 = min(x-limit2-1, image.cols-1);
		//Region 2 Limits
		iMin2 = max(0, x+limit2+1);
		iMax2 = min(x+limit1, image.cols-1);
		//Candidate Region Pixels
		iMin3 = max(0, x-limit3);
		iMax3 = min(x+limit3, image.cols-1);
		for(j=jMin; j<=jMax; j++)
		{
			imageRow = (T*)(image.data + j * image.step);

			for(i=iMin1; i<=iMax1; i++)
			{
				clutterRegion3[numClutterPixelsInRegion3] = imageRow[i];
				numClutterPixels++;
				numClutterPixelsInRegion3++;
			}

			if(j>=y-limit3 && j<=y+limit3)
			{
				for(i=iMin3; i<=iMax3; i++)
				{
					candidateRegion[numCandidatePixels] = imageRow[i];
					numCandidatePixels++;
				}
			}

			for(i=iMin2; i<=iMax2; i++)
			{
				clutterRegion2[numClutterPixelsInRegion2] = imageRow[i];
				numClutterPixels++;
				numClutterPixelsInRegion2++;
			}
		}

		jMin = max(0, y+limit2+1);
		jMax = min(y+limit1, image.rows-1);
		//Region 3 Limits
		iMin1 = max(0, x-limit1);
		iMax1 = x-limit2-1;
		//Region 4 Limits
		iMin2 = max(0, x-limit2);
		iMax2 = min(x+limit1, image.cols-1);
		for(j=jMin; j<=jMax; j++)
		{
			imageRow = (T*)(image.data + j * image.step);

			for(i=iMin1; i<=iMax1; i++)
			{
				clutterRegion3[numClutterPixelsInRegion3] = imageRow[i];
				numClutterPixels++;
				numClutterPixelsInRegion3++;
			}

			for(i=iMin2; i<=iMax2; i++)
			{
				clutterRegion4[numClutterPixelsInRegion4] = imageRow[i];
				numClutterPixels++;
				numClutterPixelsInRegion4++;
			}
		}
	}

	virtual void calculateNumberOfElementsInClutterRegions(const int& x, const int& y, const int& imageHeight, const int& imageWidth,
														   const int& limit1, const int& limit2, const int& limit3, 
														   int& numRegion1, int& numRegion2, int& numRegion3, int& numRegion4)
	{
		int minY, maxY, minX, maxX;

		minY = max(y-limit1, 0);
		maxY = min(y-limit2-1, imageHeight-1);
		minX = max(x-limit1, 0);
		maxX = min(x+limit2, imageWidth-1);
		numRegion1 = max((maxY-minY+1)*(maxX-minX+1), 0);

		minY = max(y-limit1, 0);
		maxY = min(y+limit2, imageHeight-1);
		minX = max(x+limit2+1, 0);
		maxX = min(x+limit1, imageWidth-1);
		numRegion2 = max((maxY-minY+1)*(maxX-minX+1), 0);

		minY = max(y-limit2, 0);
		maxY = min(y+limit1, imageHeight-1);
		minX = max(x-limit1, 0);
		maxX = min(x-limit2-1, imageWidth-1);
		numRegion3 = max((maxY-minY+1)*(maxX-minX+1), 0);

		minY = max(y+limit2+1, 0);
		maxY = min(y+limit1, imageHeight-1);
		minX = max(x-limit2, 0);
		maxX = min(x+limit1, imageWidth-1);
		numRegion4 = max((maxY-minY+1)*(maxX-minX+1), 0);
	}

	virtual bool checkTargetExistance(double probabilityOfFalseAlarm, Detector* detector, double* candidateRegion, const int& numCandidatePixels, double* clutterRegion, const int& numClutterPixels, const int& numRegion1, const int& numRegion2, const int& numRegion3, const int& numRegion4)
	{
		return false;
	}

};
