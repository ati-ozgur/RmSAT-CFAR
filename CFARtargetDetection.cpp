#include "stdafx.h"
#include <omp.h>
#include <iostream>
#include <windows.h>
#include <opencv2\opencv.hpp>
#include "RayleighMixtureSummedAreaTableCFAR.h"
#include "targetDetectors\AdaptiveAndFastCFAR.h"
#include "targetDetectors\CellAveragingCFAR.h"
#include "targetDetectors\AutoCensoredCFAR.h"
#include "targetDetectors\VariabilityIndexCFAR.h"

using namespace std;
using namespace cv;


enum TargetDetector { TargetDetectorRmSAT_CFAR, TargetDetectorAAF_CFAR, TargetDetectorCA_CFAR, TargetDetectorAC_CFAR, TargetDetectorVI_CFAR };

void dummyInitialization()
{
	Mat dummyImage(5, 5, CV_8UC1);
	medianBlur(dummyImage, dummyImage, 3);
}

void detectTargets(TargetDetector targetDetector, string inputFileName, string outputFileName, map<string, double>& parameters, double probabilityOfFalseAlarm = 1e-4, int threadCount = INT_MAX)
{
	AbstractCFAR* CFARtargetDetector = NULL;
	switch (targetDetector)
	{
	case TargetDetectorRmSAT_CFAR	: CFARtargetDetector = new RayleighMixtureSummedAreaTableCFAR;	break;
	case TargetDetectorAAF_CFAR		: CFARtargetDetector = new AdaptiveAndFastCFAR;					break;
	case TargetDetectorCA_CFAR		: CFARtargetDetector = new CellAveragingCFAR;					break;
	case TargetDetectorAC_CFAR		: CFARtargetDetector = new AutoCensoredCFAR;					break;
	case TargetDetectorVI_CFAR		: CFARtargetDetector = new VariabilityIndexCFAR;				break;
	default: 
		cout << "Unknown target detector! (Detector ID=" << targetDetector << ")" << endl;
		return;
	}

	CFARtargetDetector->setThreadCount(threadCount);

	Mat image = imread(inputFileName , CV_LOAD_IMAGE_UNCHANGED);

	// detect targets
	Mat targetMap = CFARtargetDetector->execute(image, probabilityOfFalseAlarm, parameters);

	imwrite(outputFileName, targetMap);

	delete CFARtargetDetector;
}


int _tmain(int argc, _TCHAR* argv[])
{
	// to force OpenCV initialize before measuring the execution time of actual Rayleigh-mixture
	dummyInitialization();

	const int minimumArgumentCount = 5;
	if (argc >= minimumArgumentCount) {
		string inputFileName = string(argv[1]);
		string outputFileName = string(argv[2]);
		string targetDetectionMethodName = string(argv[3]);
		string probabilityOfFalseAlarmStr = string(argv[4]);
		
		TargetDetector targetDetector = TargetDetectorRmSAT_CFAR;
		if (targetDetectionMethodName == "AAF-CFAR") {
			targetDetector = TargetDetectorAAF_CFAR;
		}
		if (targetDetectionMethodName == "CA-CFAR") {
			targetDetector = TargetDetectorCA_CFAR;
		}
		if (targetDetectionMethodName == "AC-CFAR") {
			targetDetector = TargetDetectorAC_CFAR;
		}
		if (targetDetectionMethodName == "VI-CFAR") {
			targetDetector = TargetDetectorVI_CFAR;
		}

		switch (targetDetector)
		{
		case TargetDetectorRmSAT_CFAR: cout << "Rayleigh-mixture Summed Area Table CFAR (RmSAT-CFAR)" << endl;	break;
		case TargetDetectorAAF_CFAR: cout << "Adaptive and Fast CFAR (AAF-CFAR)" << endl;						break;
		case TargetDetectorCA_CFAR: cout << "Cell Averaging CFAR (CA-CFAR)" << endl;							break;
		case TargetDetectorAC_CFAR: cout << "Auto Censored CFAR (AC-CFAR)" << endl;								break;
		case TargetDetectorVI_CFAR: cout << "Variability Index CFAR (VI-CFAR)" << endl;							break;
		default:
			cout << "Unknown target detector! (Detector ID=" << targetDetector << ")" << endl;
			return 0;
		}

		double probabilityOfFalseAlarm = stod(probabilityOfFalseAlarmStr);
		cout << " Probability of false alarm = " << probabilityOfFalseAlarm << endl;

		map<string, double> parameters;

		int threadCount = INT_MAX;
		const int parameterCount = (argc - (minimumArgumentCount - 1)) / 2;
		for (int i = 0; i < parameterCount; i++) {
			string key = string(argv[minimumArgumentCount + 2 * i]);
			double value = stod(string(argv[minimumArgumentCount + 2 * i + 1]));

			if (key == "ThreadCount")
				threadCount = (int)value;
			else {
				parameters[key] = value;
				cout << " " << key << " = " << value << endl;
			}
		}

		cout << " Thread count = " << (threadCount == INT_MAX ? omp_get_max_threads() : threadCount) << endl;

		detectTargets(targetDetector, inputFileName, outputFileName, parameters, probabilityOfFalseAlarm, threadCount);
	}
	else {
		cout << "CFARtargetDetector v1.0" << endl;
		cout << "CFARtargetDetector  [Input File Name] [Output File Name] [Target Detection Method] [Probability Of False Alarm] [Key1] [Value1] ... [KeyN] [ValueN]" << endl;
		cout << " Example : CFARtargetDetector  im1024.tif im1024_targets.png RmSAT-CFAR 1e-5 ThreadCount 1 RmSAT-CFAR.guardRadius 10 RmSAT-CFAR.maximumMixtureCount 6" << endl << endl;

		cout << "RmSAT-CFAR parameters" << endl;
		cout << "---------------------" << endl;
		cout << "RmSAT-CFAR.guardRadius" << endl;
		cout << "RmSAT-CFAR.clutterRadius" << endl;
		cout << "RmSAT-CFAR.minimumMixtureCount" << endl;
		cout << "RmSAT-CFAR.maximumMixtureCount" << endl << endl;

		cout << "AAF-CFAR parameters" << endl;
		cout << "-------------------" << endl;
		cout << "AAF-CFAR.guardRadius" << endl;
		cout << "AAF-CFAR.clutterRadius" << endl;
		cout << "AAF-CFAR.censoringPercentile" << endl << endl;

		cout << "CA-CFAR, AC-CFAR, VI-CFAR parameters" << endl;
		cout << "------------------------------------" << endl;
		cout << "WB-CFAR.targetRadius" << endl;
		cout << "WB-CFAR.guardRadius" << endl;
		cout << "WB-CFAR.clutterRadius" << endl << endl;

		cout << "AC-CFAR parameters" << endl;
		cout << "------------------" << endl;
		cout << "AC-CFAR.censoringPercentile" << endl << endl;
	}

	return 0;
}
