#pragma once

#include <opencv2\opencv.hpp>
#include "MathUtilities.h"
#include "ImageUtilities.h"

using namespace std;
using namespace cv;


class RayleighMixtureData {
public:
	Mat image;
	Mat censorMap;
	int histogramSize;
	int dimension;
	Mat histogram;

	int* H;
	double* pdfEmpirical;
	double* pdfEstimated;
	double* Sn;
	double* Sd;
	double* Weights;
	double* Sigmas;
	double* sqrSigmas;
	double* intervals;
	int intervalCount;

	double histogramSum;
	int histogramMaximumOccurance;

	int iterationCount;
	double initialError;
	double finalError;

	RayleighMixtureData(Mat& image, Mat& globalHistogram, int histogramSize, int dimension, double probabilityOfFalseAlarm)
	{
		// create empirical histogram
		Mat originalHistogram = ImageUtilities::createHistogram(image);

		const int medianFilterSize = 3;
		const double censoringPercentile = 0.20;
		switch (image.type())
		{
		case CV_8U:  censorMap = createCensorMap<unsigned char>(image, globalHistogram, originalHistogram, medianFilterSize, censoringPercentile, histogramSize);		break;
		case CV_8S:  censorMap = createCensorMap<char>(image, globalHistogram, originalHistogram, medianFilterSize, censoringPercentile, histogramSize);				break;
		case CV_16U: censorMap = createCensorMap<unsigned short>(image, globalHistogram, originalHistogram, medianFilterSize, censoringPercentile, histogramSize);		break;
		case CV_16S: censorMap = createCensorMap<short>(image, globalHistogram, originalHistogram, medianFilterSize, censoringPercentile, histogramSize);				break;
		case CV_32S: censorMap = createCensorMap<int>(image, globalHistogram, originalHistogram, medianFilterSize, censoringPercentile, histogramSize);					break;
		default: censorMap = Scalar(0);
		}

		histogram = ImageUtilities::createHistogram(image, censorMap);

		histogramSize = min(histogramSize, histogram.cols);

		this->image = image;
		this->histogramSize = histogramSize;
		this->dimension = dimension;

		// create arrays
		H = new int[histogramSize];
		pdfEmpirical = new double[histogramSize];
		pdfEstimated = new double[histogramSize];
		Sn = new double[histogramSize];
		Sd = new double[histogramSize];
		Weights = new double[dimension];
		Sigmas = new double[dimension];
		sqrSigmas = new double[dimension];
		intervals = new double[dimension + 2];
		intervalCount = 0;

		histogramSum = 0.0;
		histogramMaximumOccurance = 0;

		iterationCount = 0;
		initialError = 0.0;
		finalError = 0.0;

		initializePDFandSATs(histogram);
	}

	virtual ~RayleighMixtureData()
	{
		delete[] H;
		delete[] pdfEmpirical;
		delete[] pdfEstimated;
		delete[] Sn;
		delete[] Sd;
		delete[] Weights;
		delete[] Sigmas;
		delete[] sqrSigmas;
		delete[] intervals;
	}

	template<typename T>
	static Mat createCensorMap(Mat& image, Mat& globalHistogram, Mat& histogram, int medianFilterSize, double censoringPercentile, int histogramSize)
	{
		Mat censorMap(image.rows, image.cols, CV_8UC1);
		//censorMap = Scalar(0);
		//return censorMap;

		Mat imageFiltered;
		medianBlur(image, imageFiltered, medianFilterSize);

		Mat fusedHistogram = fuseHistograms<int>(globalHistogram, histogram);

		const double contrastPercentile = 1.0 - censoringPercentile;
		const double contrastThreshold = ImageUtilities::getPercentileIndex<int>(fusedHistogram, contrastPercentile);

		const double reflectivityUpperBound = 2.5 * histogramSize;

		for (int y = 0; y < imageFiltered.rows; y++) {
			T* irow = (T*)(image.data + y * image.step);
			T* ifrow = (T*)(imageFiltered.data + y * imageFiltered.step);
			unsigned char* cmap = (unsigned char*)(censorMap.data + y * censorMap.step);

			for (int x = 0; x < imageFiltered.cols; x++) {
				cmap[x] = (irow[x] > reflectivityUpperBound || irow[x] - ifrow[x] > contrastThreshold ? UCHAR_MAX : 0);
			}
		}

		Mat structuringElement = getStructuringElement(MORPH_ELLIPSE, Size(3, 3), Point(1, 1));
		dilate(censorMap, censorMap, structuringElement);

		return censorMap;
	}

	template<typename T>
	static Mat fuseHistograms(Mat& globalHistogram, Mat& histogram)
	{
		const int binCount = 3 * min(globalHistogram.cols, histogram.cols) + 1;

		Mat fusedHistogram(1, binCount, CV_32SC1);

		int* ghist = (int*)globalHistogram.data;
		int* thist = (int*)histogram.data;
		int* fhist = (int*)fusedHistogram.data;

		for (int i = 0; i < binCount; i++) {
			fhist[i] = (i < globalHistogram.cols ? ghist[i] : 0) + (i < histogram.cols ? thist[i] : 0);
		}

		return fusedHistogram;
	}

	void initializePDFandSATs(Mat& histogram)
	{
		int* histogramData = (int*)histogram.data;

		histogramSum = 0.0;
		for (int i = 0; i<histogram.cols; i++) {
			histogramSum += histogramData[i];
			histogramMaximumOccurance = max(histogramMaximumOccurance, histogramData[i]);
		}

		const double histogramStep = (double)histogram.cols / histogramSize;

		double pdfEmpiricalSum = 0.0;
		for (int k = 0; k<histogramSize; k++) {
			// pdf
			H[k] = histogramData[(int)(k * histogramStep)];
			pdfEmpirical[k] = H[k] / histogramSum;

			pdfEmpiricalSum += pdfEmpirical[k];

			// SAT for fast Rayleigh fitting
			if (k > 0) {
				const double x = (k * histogramStep);
				Sn[k] = Sn[k - 1] + pdfEmpirical[k] * MathUtilities::sqr(x);
				Sd[k] = Sd[k - 1] + pdfEmpirical[k];
			}
			else {
				Sn[0] = 0.0;
				Sd[0] = 0.0;
			}
		}
	}

	int getPercentileIndex(double percentile)
	{
		return ImageUtilities::getPercentileIndex<double>(pdfEmpirical, histogramSize, percentile * 0.01);
	}

	// https://en.wikipedia.org/wiki/Rayleigh_distribution
	static inline double RayleighPDF(double x, double sigmaSqr)
	{
		const double t = (x / sigmaSqr);
		const double p = t * exp(-0.5 * x * t);
		return p;
	}

	// calculate Rayleigh mixture at x
	double inline calculateProbability(double x)
	{
		double p = 0.0;
		for (int k = 0; k < intervalCount; k++) {
			p += Weights[k] * RayleighPDF(x, sqrSigmas[k]);
		}

		return p;
	}

	// traditional (slow) Maximum Likelihood Estimation (MLE) using Summed Area Tables (SAT)
	inline pair<double, double> estimateSigmaSqrTraditional(int intervalStart, int intervalEnd)
	{
		const double histogramStep = (double)histogram.cols / histogramSize;

		double sumOfPdf = 0.0;
		double sumOfPdfSqr = 0.0;
		for (int k = intervalStart; k <= intervalEnd; k++) {
			const double x = (k * histogramStep);
			sumOfPdf += pdfEmpirical[k];
			sumOfPdfSqr += x * x * pdfEmpirical[k];
		}
		const double phatSqr = (0.5 * sumOfPdfSqr) / sumOfPdf;

		return pair<double, double>(phatSqr, sumOfPdf);
	}

	// fast Maximum Likelihood Estimation (MLE) using Summed Area Tables (SAT)
	inline pair<double, double> estimateSigmaSqr(int intervalStart, int intervalEnd)
	{
		const double sumOfPdf = Sd[intervalEnd] - Sd[intervalStart];
		const double sumOfPdfSqr = Sn[intervalEnd] - Sn[intervalStart];

		const double phatSqr = (0.5 * sumOfPdfSqr) / sumOfPdf;

		return pair<double, double>(phatSqr, sumOfPdf);
	}

};
