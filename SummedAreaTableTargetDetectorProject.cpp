#include "stdafx.h"
#include <omp.h>
#include <locale>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <windows.h>
#include <opencv2\opencv.hpp>
#include "TimeMeasurer.h"
#include "TileManager.h"
#include "AdaptiveSimulatedAnnealingTest.h"
#include "RayleighMixtureSummedAreaTableCFAR.h"
#include "targetDetectors\AdaptiveAndFastCFAR.h"
#include "targetDetectors\CellAveragingCFAR.h"
#include "targetDetectors\AutoCensoredCFAR.h"
#include "targetDetectors\VariabilityIndexCFAR.h"

using namespace std;
using namespace cv;


void dummyInitialization()
{
	Mat dummyImage(5, 5, CV_8UC1);
	medianBlur(dummyImage, dummyImage, 3);
}

pair<double, double> createPerformanceValues(Mat targetMap, Mat& groundtruthImage)
{
	int TP = 0;
	int FP = 0;
	int TN = 0;
	int FN = 0;
	for (int y = 0; y < targetMap.rows; y++) {
		unsigned char* trow = (unsigned char*)(targetMap.data + y * targetMap.step);
		unsigned char* grow = (unsigned char*)(groundtruthImage.data + y * groundtruthImage.step);

		for (int x = 0; x < targetMap.cols; x++) {
			if (trow[x] > 0 && grow[x] > 0) {
				TP++;
			}

			if (trow[x] > 0 && grow[x] == 0) {
				FP++;
			}

			if (trow[x] == 0 && grow[x] == 0) {
				TN++;
			}

			if (trow[x] == 0 && grow[x] > 0) {
				FN++;
			}
		}
	}

	
	const double FalsePositiveRate = (double)FP / (FP + TN);
	const double TruePositiveRate = (double)TP / (TP + FN);

	return pair<double, double>(FalsePositiveRate, TruePositiveRate);
	
	/*
	const double Precision = (double)TP / (TP + FP);
	const double Recall = (double)TP / (TP + FN);

	return pair<double, double>(1.0-Precision, Recall);
	*/
}

void createPerformanceTest(Mat& image, Mat& groundtruthImage, string testResultsPath, string inputFileName, AbstractCFAR* CFARtargetDetector, map<string, double>& parameters)
{
	if (groundtruthImage.empty()) {
		return;
	}

	const int binCount = 24;
	double* FalsePositiveRateIndices = new double[binCount];
	double* TruePositiveRateArr = new double[binCount];
	memset(TruePositiveRateArr, 0, sizeof(double) * binCount);


	const double minimumFARpower = -6.0;
	const double minimumFPR = pow(10.0, minimumFARpower);

	vector<double> PfaPowerList;
	for (double PfaPower = 0.0; PfaPower >= minimumFARpower; PfaPower -= 0.1) {
		PfaPowerList.push_back(PfaPower);
	}

	const int experimentCount = (CFARtargetDetector->isDeterministic() ? 1 : 5);

	cout << "Creating ROC data for " << inputFileName << "  (Experiment count = " << PfaPowerList.size() << "x" << experimentCount << ")" << endl;
	cout << "-------------------------------------------------------" << endl;

	TimeMeasurer timeMeasurer;

	Mat bestTargetMap;
	double bestScore = 0.0;
	double bestPfa = 0.0;
	double bestFPR = 0.0;
	double bestTPR = 0.0;

	for (int i = 0; i < PfaPowerList.size(); i++) {
		const double PfaPower = PfaPowerList.at(i);
		const double probabilityOfFalseAlarm = pow(10.0, PfaPower);

		for (int j=0; j<experimentCount; j++) {
			Mat targetMap = CFARtargetDetector->execute(image, probabilityOfFalseAlarm, parameters);

			pair<double, double>performanceValues = createPerformanceValues(targetMap, groundtruthImage);
			const double FalsePositiveRate = performanceValues.first;
			const double TruePositiveRate = performanceValues.second;

			if (FalsePositiveRate >= minimumFARpower) {
				const double logFalsePositiveRate = log(FalsePositiveRate) / log(10.0);
				const int FalsePositiveRateIndex = (int)(binCount * (1.0 - logFalsePositiveRate / minimumFARpower));

				if (FalsePositiveRateIndex >= 0  && FalsePositiveRateIndex < binCount) {
					if (j == 0) {
						cout << "log(Pfa) = " << probabilityOfFalseAlarm << " : FPR = " << FalsePositiveRate << ",  TPR = " << TruePositiveRate << endl;
					}

					TruePositiveRateArr[FalsePositiveRateIndex] = max(TruePositiveRateArr[FalsePositiveRateIndex], TruePositiveRate);

					const double score = log(FalsePositiveRate) / minimumFARpower + 2.0 * TruePositiveRate;

					if (score >= bestScore && FalsePositiveRate <= 0.25 && TruePositiveRate >= 0.75) {
						bestTargetMap = targetMap;
						bestScore = score;
						bestPfa = probabilityOfFalseAlarm;
						bestFPR = FalsePositiveRate;
						bestTPR = TruePositiveRate;
					}
				}
			}
		}
	}
	cout << "Completed in " << timeMeasurer.getTimeNanosecond() / 1000.0 << " seconds" << endl;

	// save results
	stringstream sst;
	sst << testResultsPath << inputFileName << "_bestTargetMap_Pfa=" << bestPfa << "_FPR=" << bestFPR << "_TPR=" << bestTPR << ".png";
	imwrite(sst.str(), UCHAR_MAX - bestTargetMap);


	vector<double> FalsePositiveRateList;
	vector<double> TruePositiveRateList;
	double minimumObtainedFPR = 1.0;
	for (int FalsePositiveRateIndex = binCount-1; FalsePositiveRateIndex >= 0; FalsePositiveRateIndex--) {
		const double FalsePositiveRate = pow(10.0, (1.0 - FalsePositiveRateIndex / (double)binCount) * minimumFARpower);
		const double TruePositiveRate = TruePositiveRateArr[FalsePositiveRateIndex];

		if (TruePositiveRate > 0 && FalsePositiveRate >= minimumFPR) {
			minimumObtainedFPR = min(minimumObtainedFPR, FalsePositiveRate);
		}

		if (FalsePositiveRateIndex == binCount - 1 && (FalsePositiveRate != 1.0 || TruePositiveRate != 1.0)) {
			FalsePositiveRateList.push_back(1.0);
			TruePositiveRateList.push_back(1.0);
		}

		if (TruePositiveRate > 0) {
			FalsePositiveRateList.push_back(FalsePositiveRate);
			TruePositiveRateList.push_back(TruePositiveRate);
		}

		if (FalsePositiveRateIndex == 0 && (FalsePositiveRate != 0.0 || TruePositiveRate != 0.0)) {
			FalsePositiveRateList.push_back(minimumObtainedFPR * 0.99);
			TruePositiveRateList.push_back(0.0);
		}
	}
	delete[] FalsePositiveRateIndices;
	delete[] TruePositiveRateArr;


	double areaUnderCurve = 0.0;
	stringstream ss;
	ss << testResultsPath << inputFileName << "_ROC.txt";
	ofstream rocFile;
	rocFile.open(ss.str());
	for (int i = 0; i < FalsePositiveRateList.size(); i++) {
		rocFile << FalsePositiveRateList.at(i) << "\t" << TruePositiveRateList.at(i) << endl;

		if (i > 0) {
			areaUnderCurve += (FalsePositiveRateList.at(i-1) - FalsePositiveRateList.at(i)) * TruePositiveRateList.at(i);
		}
	}
	rocFile.close();

	cout << "Area Under Curve (AUC) = " << areaUnderCurve << endl;
	cout << endl << endl;
}


void RayleighMixtureTest()
{
	vector<string> fileNames;

	string imagePath = "_images\\";
	//fileNames.push_back("IndiaLargePatch");
	fileNames.push_back("im1024");
	fileNames.push_back("forest");
	fileNames.push_back("sea_calm");
	fileNames.push_back("urban1");
	fileNames.push_back("urban2");
	fileNames.push_back("urban3");
	//fileNames.push_back("image_HH_ORI_B0");
	//fileNames.push_back("IMAGE_HH_SRA_spot_029");
	//fileNames.push_back("IMAGE_HH_SRA_spot_043");
	//fileNames.push_back("IMAGE_HH_SRA_strip_004");
	//fileNames.push_back("IMAGE_HH_SRA_strip_012");
	//fileNames.push_back("IMAGE_HV_SRA_wide_001"); 
	//fileNames.push_back("IMAGE_VV_SRA_spot_057");

	/*
	string imagePath = "_clutters\\";
	fileNames.push_back("Carabas_Forest");
	fileNames.push_back("TerraSARX_IslandRugen_Farmland");
	fileNames.push_back("TerraSARX_PanamaCanal_Water");
	fileNames.push_back("TerraSARX_RussiaMonino_Soil");
	fileNames.push_back("TerraSARX_Toronto_Urban");
	//fileNames.push_back("TerraSARX_Toronto_DenseUrban");
	//fileNames.push_back("TerraSARX_StraitOfGibraltar_Water");
	//fileNames.push_back("TerraSARX_India_PureWater");
	//fileNames.push_back("TerraSARX_RussiaMonino_PureSoil");
	*/

	string groundtruthPath = "_groundTruths\\";
	string testResultsPath = "_testResults\\";

	AbstractCFAR* CFARtargetDetector = new RayleighMixtureSummedAreaTableCFAR;
	//AbstractCFAR* CFARtargetDetector = new AdaptiveAndFastCFAR;
	//AbstractCFAR* CFARtargetDetector = new CellAveragingCFAR;
	//AbstractCFAR* CFARtargetDetector = new AutoCensoredCFAR;
	//AbstractCFAR* CFARtargetDetector = new VariabilityIndexCFAR;

	map<string, double> parameters;
	parameters["RmSAT-CFAR.guardRadius"] = 5;
	parameters["RmSAT-CFAR.clutterRadius"] = 5;
	parameters["RmSAT-CFAR.minimumMixtureCount"] = 1;
	parameters["RmSAT-CFAR.maximumMixtureCount"] = 5;
	
	parameters["AAF-CFAR.guardRadius"] = 5;
	parameters["AAF-CFAR.clutterRadius"] = 5;
	parameters["AAF-CFAR.censoringPercentile"] = 99.0;

	parameters["WB-CFAR.targetRadius"] = 2;
	parameters["WB-CFAR.guardRadius"] = 3;
	parameters["WB-CFAR.clutterRadius"] = 5;
	parameters["AC-CFAR.censoringPercentile"] = 99.0;

	CFARtargetDetector->setThreadCount(8);

	cout << "Band size = " << CFARtargetDetector->getBandWidth(parameters) << endl;
	cout << "Clutter area = " << CFARtargetDetector->getClutterArea(parameters) << endl;
	cout << "Thread count = " << CFARtargetDetector->getThreadCount() << endl;

	for (int i = 0; i < fileNames.size(); i++) {
		TimeMeasurer timeMeasurer;

		// load SAR image
		string inputFileName = fileNames.at(i);
		Mat image = imread(imagePath + inputFileName + ".tif", CV_LOAD_IMAGE_UNCHANGED);
		Mat groundtruthImage = imread(groundtruthPath + inputFileName + "_groundTruth.png", CV_LOAD_IMAGE_UNCHANGED);

		Rect boundingBox = TileManager::findBoundingBox(image);
		image = image(boundingBox).clone();
		if (!groundtruthImage.empty()) {
			groundtruthImage = groundtruthImage(boundingBox).clone();
		}
		const double imageLoadTime = timeMeasurer.getTimeNanosecond() / 1000.0;


		// performance test
		//createPerformanceTest(image, groundtruthImage, testResultsPath, inputFileName, CFARtargetDetector, parameters); continue;


		// detect targets
		timeMeasurer.resetTimer();
		const double probabilityOfFalseAlarm = 1e-5;
		Mat targetMap = CFARtargetDetector->execute(image, probabilityOfFalseAlarm, parameters);
		const double targetDetectionTime = timeMeasurer.getTimeNanosecond() / 1000.0;


		// save results
		timeMeasurer.resetTimer();

		stringstream ssi;
		ssi << "_experiments\\" + inputFileName + "_targetMap.png";
		imwrite(ssi.str(), targetMap);

		stringstream sst;
		sst << "_experiments\\" + inputFileName + "_8bit.png";
		Mat image8bit = image.mul(0.25);
		image8bit.convertTo(image8bit, CV_8UC1);
		imwrite(sst.str(), image8bit);

		const double resultSaveTime = timeMeasurer.getTimeNanosecond() / 1000.0;

		if (!groundtruthImage.empty()) {
			pair<double, double> performanceValues = createPerformanceValues(targetMap, groundtruthImage);
			const double FalsePositiveRate = performanceValues.first;
			const double TruePositiveRate = performanceValues.second;

			cout << "Pfa = " << probabilityOfFalseAlarm << " : FPR = " << FalsePositiveRate << ",  TPR = " << TruePositiveRate << "   >    ";
		}

		// show execution times
		cout << inputFileName << " " << image.size() << " : targets are detected in " << targetDetectionTime << " seconds";
		//cout << "     (Load time = " << imageLoadTime << " secs, Save time = " << resultSaveTime << " secs)";
		cout << endl;
	}

	delete CFARtargetDetector;
}


int _tmain(int argc, _TCHAR* argv[])
{
	/*
	Mat image = imread("_images\\IMAGE_VV_SRA_spot_057.tif", CV_LOAD_IMAGE_UNCHANGED);

	const int xStart = 4350;
	const int yStart = 4750;

	Mat patch = image(Range(yStart, yStart + 1024 * 8), Range(xStart, xStart + 1024 * 13)).clone();
	imwrite("_images\\IndiaLargePatch.tif", patch);
	cout << patch.size() << endl;

	//imwrite

	patch = patch.mul(0.35);
	patch.convertTo(patch, CV_8UC1);
	resize(patch, patch, Size(750, 750));
	imshow("patch", patch);
	waitKey();

	return 0;
	*/

	// to force OpenCV initialize before measuring the execution time of actual Rayleigh-mixture
	dummyInitialization();

	///AdaptiveSimulatedAnnealingTest::execute();

	RayleighMixtureTest();

	return 0;
}
