#pragma once

#include <limits>
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;


class ImageUtilities {
public:
	static void addToHistogram(Mat& image, Mat& histogram, int startValue = 1)
	{
		switch (image.type())
		{
		case CV_8U:  addToHistogram<unsigned char>(image, histogram, UCHAR_MAX+1, startValue);		break;
		case CV_8S:  addToHistogram<char>(image, histogram, UCHAR_MAX + 1, startValue);				break;
		case CV_16U: addToHistogram<unsigned short>(image, histogram, USHRT_MAX + 1, startValue);	break;
		case CV_16S: addToHistogram<short>(image, histogram, USHRT_MAX + 1, startValue);			break;
		}
	}

	template<typename T>
	static void addToHistogram(Mat& image, Mat& histogram, int histogramSize, int startValue)
	{
		if (histogram.empty() || histogram.cols != histogramSize) {
			histogram = Mat(1, histogramSize, CV_32SC1, Scalar(0));			
		}

		int* histogramData = (int*)histogram.data;
		for (int y = 0; y < image.rows; y++) {
			T* irow = (T*)(image.data + y * image.step);

			for (int x = 0; x < image.cols; x++) {
				const int pixelValue = (int)irow[x];

				if (pixelValue >= startValue) {
					histogramData[pixelValue]++;
				}
			}
		}
	}

	static Mat createHistogram(Mat& image, int startValue = 0)
	{
		Mat histogram;

		switch (image.type())
		{
		case CV_8U:  createHistogram<unsigned char>(image, histogram, startValue);		break;
		case CV_8S:  createHistogram<char>(image, histogram, startValue);				break;
		case CV_16U: createHistogram<unsigned short>(image, histogram, startValue);		break;
		case CV_16S: createHistogram<short>(image, histogram, startValue);				break;
		case CV_32S: createHistogram<int>(image, histogram, startValue);				break;
		case CV_32F: createHistogram<float>(image, histogram, startValue);				break;
		case CV_64F: createHistogram<double>(image, histogram, startValue);				break;
		}

		return histogram;
	}

	static Mat createHistogram(Mat& image, Mat& censorMap)
	{
		Mat histogram;

		switch (image.type())
		{
		case CV_8U:  createHistogram<unsigned char>(image, censorMap, histogram);		break;
		case CV_8S:  createHistogram<char>(image, censorMap, histogram);				break;
		case CV_16U: createHistogram<unsigned short>(image, censorMap, histogram);		break;
		case CV_16S: createHistogram<short>(image, censorMap, histogram);				break;
		case CV_32S: createHistogram<int>(image, censorMap, histogram);					break;
		case CV_32F: createHistogram<float>(image, censorMap, histogram);				break;
		case CV_64F: createHistogram<double>(image, censorMap, histogram);				break;
		}

		return histogram;
	}

	template<typename T>
	static void createHistogram(Mat& image, Mat& histogram, int startValue=1)
	{
		int maximumIntensity = 0;
		for (int y = 0; y < image.rows; y++) {
			T* irow = (T*)(image.data + y * image.step);

			for (int x = 0; x < image.cols; x++) {
				maximumIntensity = max(maximumIntensity, (int)irow[x]);
			}
		}

		const int histogramSize = (maximumIntensity + 1);
		histogram = Mat(1, histogramSize, CV_32SC1, Scalar(0));
		int* histogramData = (int*)histogram.data;

		if (maximumIntensity > 0) {
			for (int y = 0; y < image.rows; y++) {
				T* irow = (T*)(image.data + y * image.step);

				for (int x = 0; x < image.cols; x++) {
					const int pixelValue = (int)irow[x];

					if (pixelValue >= startValue) {
						histogramData[pixelValue]++;
					}
				}
			}
		}
	}

	template<typename T>
	static void createHistogram(Mat& image, Mat& censorMap, Mat& histogram, int startValue = 1)
	{
		int maximumIntensity = 0;
		for (int y = 0; y < image.rows; y++) {
			T* irow = (T*)(image.data + y * image.step);
			unsigned char* crow = (unsigned char*)(censorMap.data + y * censorMap.step);

			for (int x = 0; x < image.cols; x++) {
				if (crow[x] == 0) {
					maximumIntensity = max(maximumIntensity, (int)irow[x]);
				}
			}
		}

		const int histogramSize = (maximumIntensity + 1);
		histogram = Mat(1, histogramSize, CV_32SC1, Scalar(0));
		int* histogramData = (int*)histogram.data;

		if (maximumIntensity > 0) {
			for (int y = 0; y < image.rows; y++) {
				T* irow = (T*)(image.data + y * image.step);
				unsigned char* crow = (unsigned char*)(censorMap.data + y * censorMap.step);

				for (int x = 0; x < image.cols; x++) {
					if (crow[x] == 0) {
						const int pixelValue = (int)irow[x];

						if (pixelValue >= startValue) {
							histogramData[pixelValue]++;
						}
					}
				}
			}
		}
	}

	static Mat createPDFfromHistogram(Mat& histogram)
	{
		Mat pdf_Histogram(1, histogram.cols, CV_64FC1);

		int* histogramData = (int*)histogram.data;
		double histogramSum = 0.0;
		for (int i = 0; i < histogram.cols; i++) {
			histogramSum += histogramData[i];
		}

		const double histogramNormalizer = (histogramSum > 0 ? 1.0 / histogramSum : 0.0);
		
		double* pdf_HistogramData = (double*)pdf_Histogram.data;
		for (int i = 0; i < histogram.cols; i++) {
			pdf_HistogramData[i] = histogramData[i] * histogramNormalizer;
		}

		return pdf_Histogram;
	}

	static Mat createCDFfromHistogram(Mat& histogram)
	{
		Mat cdf_Histogram(1, histogram.cols, CV_64FC1);

		int* histogramData = (int*)histogram.data;
		double histogramSum = 0.0;
		for (int i = 0; i < histogram.cols; i++) {
			histogramSum += histogramData[i];
		}

		const double histogramNormalizer = (histogramSum > 0 ? 1.0 / histogramSum : 0.0);

		double* cdf_HistogramData = (double*)cdf_Histogram.data;
		double cumulativeSum = 0.0;
		for (int i = 0; i < histogram.cols; i++) {
			cumulativeSum += histogramData[i] * histogramNormalizer;
			cdf_HistogramData[i] = cumulativeSum;
		}

		return cdf_Histogram;
	}

	template<typename T>
	static Mat cumulativeSum(Mat& distribution)
	{
		Mat cumulative(1, distribution.cols, distribution.type());

		T* distributionData = (T*)distribution.data;
		T* cumulativeData = (T*)cumulative.data;

		T cumulativeSum = 0;
		for (int i = 0; i < distribution.cols; i++) {
			cumulativeSum += distributionData[i];
			cumulativeData[i] = cumulativeSum;
		}

		return cumulative;
	}

	template<typename T>
	static int getPercentileIndex(Mat& histogram, double percentile)
	{
		if (percentile <= 0) {
			return 0;
		}

		if (percentile >= 1) {
			return (histogram.cols-1);
		}

		T* histogramData = (T*)histogram.data;

		double histogramSum = 0.0;
		for (int i = 0; i < histogram.cols; i++) {
			histogramSum += (double)histogramData[i];
		}

		const double percentileSum = (histogramSum * percentile);

		double cumulativeSum = 0.0;
		for (int i = 0; i < histogram.cols; i++) {
			cumulativeSum += histogramData[i];
			if (cumulativeSum >= percentileSum) {
				return i;
			}
		}

		return (histogram.cols - 1);
	}

	template<typename T>
	static int getPercentileIndex(T* histogram, int histogramSize, double percentile)
	{
		if (percentile <= 0.0) {
			return 0;
		}

		if (percentile >= 1.0) {
			return (histogramSize - 1);
		}

		double histogramSum = 0.0;
		for (int i = 0; i < histogramSize; i++) {
			histogramSum += (double)histogram[i];
		}

		const double percentileSum = (histogramSum * percentile);

		double cumulativeSum = 0.0;
		for (int i = 0; i < histogramSize; i++) {
			cumulativeSum += histogram[i];
			if (cumulativeSum >= percentileSum) {
				return i;
			}
		}

		return (histogramSize - 1);
	}

	template<typename T>
	static Mat createHistogramImage(Mat& histogram, int histogramWidth, int histogramHeight, double upperPercentile = 1.0, int extraBand = 16)
	{
		const int numberOfBins = getPercentileIndex<T>(histogram, upperPercentile);
		histogramWidth = min(histogramWidth, numberOfBins);

		T* histogramData = (T*)histogram.data;
		T maxOccurance = 0;
		for (int i = 0; i < numberOfBins; i++) {
			maxOccurance = max(maxOccurance, histogramData[i]);
		}

		Mat histogramImage(histogramHeight + 2 * extraBand, histogramWidth + 2 * extraBand, CV_8UC1, Scalar(UCHAR_MAX));
		for (int i = 0; i < histogramWidth; i++) {
			const int hIndex = (int)((double)i / histogramWidth * numberOfBins);
			const int hOccurance = (int)((double)histogramData[hIndex] / maxOccurance * histogramHeight);

			if (histogramData[hIndex] > 0) {
				line(histogramImage, Point(i + extraBand, histogramHeight + extraBand - 1), Point(i + extraBand, histogramHeight - hOccurance + extraBand - 1), Scalar(0));
			}
		}

		return histogramImage;
	}

	template<typename T>
	static void showHistogram(Mat& histogram, int histogramWidth, int histogramHeight, string histogramCaption, double upperPercentile = 1.0, int extraBand = 16)
	{
		Mat histogramImage = createHistogramImage<T>(histogram, histogramWidth, histogramHeight, upperPercentile, extraBand);
		imshow(histogramCaption, histogramImage);
	}

	template<typename T>
	static void showImage(Mat& image, string histogramCaption, double upperPercentile = 0.999)
	{
		Mat histogram = createHistogram(image);
		const int upperIntensity = getPercentileIndex<int>(histogram, upperPercentile);

		const double multiplier = UCHAR_MAX / (double)upperIntensity;
		Mat imageStretched = image.mul(multiplier);
		imageStretched.convertTo(imageStretched, CV_8UC1);
		imshow(histogramCaption, imageStretched);
	}

private:

};
