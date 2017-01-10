#pragma once

class Detector {
public:

	virtual bool detect(const double valueToTest, double* clutterValues, const int numberOfClutterValues, const double probabilityOfFalseAlarm)
	{
		estimatePdfParameters(clutterValues, numberOfClutterValues);

		const double threshold = estimateThreshold(probabilityOfFalseAlarm);
		
		return testValue(valueToTest, threshold);
	}

	inline void clearData()
	{
		if (data != NULL) {
			delete [] data;

			data = NULL;
			dataSize = 0;
		}
	}

	inline double* setDataSize(const int size)
	{
		if (size > dataSize) {
			delete [] data;

			this->dataSize = size;
			data = new double[size];
		}

		return data;
	}

	inline double* getData() const { return data; }
		
protected:
	virtual void estimatePdfParameters(double* clutterValues, const int numberOfClutterValues) = NULL;
	
	virtual double estimateThreshold(const double probabilityOfFalseAlarm) = NULL;

	virtual bool testValue(const double valueToTest, const double threshold)
	{
		return (valueToTest > threshold);
	}

private:
	double* data;
	int dataSize;
};
