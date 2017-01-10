#pragma once

#include <windows.h>

using namespace std;


class TimeMeasurer {
private:
	__int64 freq, start, end;
public:
	TimeMeasurer()
	{
		resetTimer();
	}

	void resetTimer()
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		QueryPerformanceCounter((LARGE_INTEGER*)&start);
	}

	__int64 getTime()
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&end);

		return ((end - start) * 1000) / freq;
	}

	double getTimeMicrosecond()
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&end);

		const double microScaling = 1e3;

		return ((end - start) * 1000 * microScaling) / (freq * microScaling);
	}

	double getTimeNanosecond()
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&end);

		const double nanoScaling = 1e6;

		return ((end - start) * 1000 * nanoScaling) / (freq * nanoScaling);
	}
};
