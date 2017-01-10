#pragma once

template <typename T>
inline T minValue(T* vect, const int& numElems)
{
	T minVal = vect[0];
	for(int i=1; i<numElems; i++)
	{
		if(vect[i] < minVal)
		{
			minVal = vect[i];
		}
	}

	return minVal;
}

template <typename T>
inline T maxValue(T* vect, const int& numElems)
{
	T maxVal = vect[0];
	for(int i=1; i<numElems; i++)
	{
		if(vect[i] > maxVal)
		{
			maxVal = vect[i];
		}
	}

	return maxVal;
}

template <typename T>
inline double mean(T* vect, const int& numElems)
{
	double meanVal = 0.0;

	for(int i=0; i<numElems; i++)
	{
		meanVal += vect[i];
	}

	meanVal /= numElems;

	return meanVal;
}

template <typename T>
inline T median(T* vect, const int& numElems)
{
	int medianIndex = (numElems / 2);

	return vect[T];
}

template <typename T>
inline double stdDev(T* vect, const int& numElems)
{
	double var = variance(vect, numElems);
	return sqrt(var);
}

template <typename T>
inline double variance(T* vect, const int& numElems)
{
	double var = 0.0;
	double mean = 0.0;

	for(int i=0; i<numElems; i++)
	{
		mean += vect[i];
		var += (vect[i] * vect[i]);
	}

	mean = mean*mean/(numElems*(numElems-1));
	var /= (numElems-1);

	var = var - mean;

	return abs(var);
}

template <typename T>
inline void meanAndVariance(T* vect, const int& numElems, double& mean, double& var, bool excludeZeros)
{
	var = 0.0;
	mean = 0.0;
	int effectiveNumElems = 0;

	if(!excludeZeros)
	{
		for(int i=0; i<numElems; i++)
		{
			mean += vect[i];
			var += (vect[i] * vect[i]);
		}

		effectiveNumElems = numElems;
	}
	else
	{
		for(int i=0; i<numElems; i++)
		{
			if(vect[i] != 0)
			{
				mean += vect[i];
				var += (vect[i] * vect[i]);
				effectiveNumElems++;
			}
		}
	}

	mean /= effectiveNumElems;
	var /= (effectiveNumElems-1);

	var = abs(var - mean*mean);
}

template <typename T>
inline void meanAndStd(T* vect, const int& numElems, double& mean, double& std, bool excludeZeros)
{
	meanAndVariance<T>(vect, numElems, mean, std, excludeZeros);
	std = sqrt(std);
}

template<typename T>
inline void meanAndStd(T* imageData, int width, int height, int xc, int yc, int h, double& mean, double& std, bool excludeZeros)
{
	//Evaluates mean and standard deviation of row-wised scanned 2D data, for a hxh block centered at (xc,yc).
	meanAndVar<T>(imageData, width, height, xc, yc, h, mean, std, excludeZeros);
	std = sqrt(std);
}

template<typename T>
inline void meanAndVar(T* imageData, int width, int height, int xc, int yc, int h, double& mean, double& var, bool excludeZeros)
{
	//Evaluates mean and variance of row-wised scanned 2D data, for a hxh block centered at (xc,yc).
	int minX = max(0, xc-h);
	int maxX = min(xc+h, width-1);
	int minY = max(0, yc-h);
	int maxY = min(yc+h, height-1);

	T* imageDataRow;
	int numOfElements = 0;
	double sum = 0.0;
	double squareSum = 0.0;
	if(!excludeZeros)
	{
		for(int y=minY; y<=maxY; y++)
		{
			imageDataRow = imageData + y*width;
			for(int x=minX; x<=maxX; x++)
			{
				sum += imageDataRow[x];
				squareSum += (imageDataRow[x] * imageDataRow[x]);
				numOfElements++;
			}
		}
	}
	else
	{
		for(int y=minY; y<=maxY; y++)
		{
			imageDataRow = imageData + y*width;
			for(int x=minX; x<=maxX; x++)
			{
				if(imageDataRow[x] != 0)
				{
					sum += imageDataRow[x];
					squareSum += (imageDataRow[x] * imageDataRow[x]);
					numOfElements++;
				}
			}
		}
	}

	mean = sum/numOfElements;
	var = squareSum/numOfElements;
	var = abs(var - mean*mean);
}