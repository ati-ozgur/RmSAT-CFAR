#pragma once

#include <omp.h>
#include <opencv2\opencv.hpp>

using namespace cv;
using namespace std;


class AbstractCFAR {
public:
	AbstractCFAR()
	{
		threadCount = INT_MAX;
	}

	virtual ~AbstractCFAR()
	{
	}

	virtual AbstractCFAR* clone() = 0;

	virtual Mat execute(Mat image, double probabilityOfFalseAlarm, map<string, double>& parameters) = 0;

	virtual int getClutterArea(map<string, double>& parameters) = 0;

	virtual int getBandWidth(map<string, double>& parameters) = 0;

	virtual bool isDeterministic() const { return true;  }

	virtual bool requiresGlobalHistogram() const { return false; }

	double getParameterValue(map<string, double>& parameters, string key, double defaultValue)
	{
		if (parameters.find(key) == parameters.end())
			return defaultValue;
		else
			return parameters[key];
	}	

	int getThreadCount() const
	{
		return max(min(omp_get_max_threads(), threadCount), 1);
	}

	void setThreadCount(int threadCount)
	{
		this->threadCount = threadCount;
	}

	template<typename T>
	static inline T sqr(T x)
	{
		return x * x;
	}
	
protected:

	int calculateBandSize(int windowRadius, int blockSize = 8)
	{
		const int overlapSize = ((2 * windowRadius + blockSize - 1) / blockSize) * blockSize;

		return overlapSize;
	}

private:
	int threadCount;

};
