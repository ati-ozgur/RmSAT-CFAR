#pragma once

#include <iostream>
#include "TimeMeasurer.h"
#include "RayleighMixtureData.h"

using namespace std;


class TargetDetectorBaseLogger {
public:
	TargetDetectorBaseLogger()
	{
		totalTime = 0.0;
	}

	void startTimer()
	{
		timeMeasurer.resetTimer();
	}

	void endTimer(string message)
	{
		measuredTime = timeMeasurer.getTimeNanosecond();
		totalTime += measuredTime;
		timeMeasurer.resetTimer();

		showTime(message, measuredTime);
	}

	void startTotalTimer()
	{
		totalTime = 0.0;
	}

	double endTotalTimer(string message)
	{
		showTime(message, totalTime);
		totalTime = 0.0;
	}

	virtual void showTime(string message, double measuredTime)
	{
	}

	double getTotalTime() const
	{
		return totalTime;
	}

	virtual void printText(string text) 
	{

	}

	virtual void showFitting(RayleighMixtureData& rayleighMixtureData)
	{

	}

private:
	TimeMeasurer timeMeasurer;
	double measuredTime;
	double totalTime;

};
