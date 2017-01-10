#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include "AbstractCostFunction.h"


// https://en.wikipedia.org/wiki/Test_functions_for_optimization
enum TestCostFunction {
	RastriginFunction, AckleysFunction, SphereFunction, RosenbrockFunction, BealesFunction, GoldsteinPriceFunction, BoothsFunction, BukinFunctionNo6, 
	MatyasFunction, LeviFunctionNo13, ThreeHumpCamelFunction, EasomFunction, CrossInTrayFunction, EggholderFunction, HolderTableFunction, McCormickFunction, 
	SchafferFunctionNo2, SchafferFunctionNo4, StyblinskiTangFunction
};


class NonlinearTestCostFunctions : public AbstractCostFunction {
public:
	NonlinearTestCostFunctions(int dimension, TestCostFunction testCostFunction) : AbstractCostFunction(dimension)
	{
		this->testCostFunction = testCostFunction;

		switch (testCostFunction)
		{
		case RastriginFunction:
			setLowerBound(-5.12);
			setUpperBound(+5.12);
			break;

		case AckleysFunction:
			setLowerBound(-5.0);
			setUpperBound(+5.0);
			break;

		case SphereFunction:
			setLowerBound(-10.0);
			setUpperBound(+10.0);
			break;

		case RosenbrockFunction:
			setLowerBound(-10.0);
			setUpperBound(+10.0);
			break;

		case BealesFunction:
			setLowerBound(-4.5);
			setUpperBound(+4.5);
			break;

		case GoldsteinPriceFunction:
			setLowerBound(-2.0);
			setUpperBound(+2.0);
			break;

		case BoothsFunction:
			setLowerBound(-10.0);
			setUpperBound(+10.0);
			break;

		case BukinFunctionNo6:
			setLowerBound(-15.0);
			setUpperBound(+15.0);
			break;

		case MatyasFunction:
			setLowerBound(-10.0);
			setUpperBound(+10.0);
			break;

		case LeviFunctionNo13:
			setLowerBound(-10.0);
			setUpperBound(+10.0);
			break;

		case ThreeHumpCamelFunction:
			setLowerBound(-5.0);
			setUpperBound(+5.0);
			break;

		case EasomFunction:
			setLowerBound(-100.0);
			setUpperBound(+100.0);
			break;

		case CrossInTrayFunction:
			setLowerBound(-10.0);
			setUpperBound(+10.0);
			break;

		case EggholderFunction:
			setLowerBound(-512.0);
			setUpperBound(+512.0);
			break;

		case HolderTableFunction:
			setLowerBound(-10.0);
			setUpperBound(+10.0);
			break;

		case McCormickFunction:
			setLowerBound(-4.0);
			setUpperBound(+4.0);
			break;

		case SchafferFunctionNo2:
			setLowerBound(-100.0);
			setUpperBound(+100.0);
			break;

		case SchafferFunctionNo4:
			setLowerBound(-100.0);
			setUpperBound(+100.0);
			break;

		case StyblinskiTangFunction:
			setLowerBound(-5.0);
			setUpperBound(+5.0);
			break;
		}
	}

	// override
	virtual double evaluate(double* x)
	{
		const double x1 = x[0];
		const double x2 = x[1];

		switch (testCostFunction)
		{
		case RastriginFunction:
			return 0.0;	//TODO
			// optimum at P(0, 0) = 0

		case AckleysFunction:
			return ( -20.0 * exp(-0.2 * sqrt(0.5 * (sqr(x1) + sqr(x2)))) - exp(0.5 * (cos(2.0 * M_PI * x1) + cos(2.0 * M_PI * x2))) + exp(1.0) + 20.0 );
			// optimum at P(0, 0) = 0

		case SphereFunction:
			return ( sqr(x1) + sqr(x2) );
			// optimum at P(0, 0) = 0

		case RosenbrockFunction:
			return ( 100.0 * sqr(x2 - sqr(x1)) + sqr(x1- 1) );
			// optimum at P(1, 1) = 0

		case BealesFunction:
			return ( sqr(1.5 - x1 + x1 * x2) + sqr(2.25 - x1 + x1 * sqr(x2)) + sqr(2.625 - x1 + x1 * cube(x2)) );
			// optimum at P(3, 0.5) = 0

		case GoldsteinPriceFunction:
			return ( (1.0 + sqr(x1 + x2 + 1) * (19.0 - 14 * x1 + 3.0 * cube(x1) - 14.0 * x2 + 6.0 * x1 * x2 + 3.0 * sqr(x2))) + (30.0 + sqr(2.0 * x1 - 3.0 * x2) * (18.0 - 32.0 * x1 + 12.0 * sqr(x1) + 48.0 * x2 - 36.0 * x1 * x2 + 27.0 * sqr(x2))) );
			// optimum at P(0, -1) = 4

		case BoothsFunction:
			return ( sqr(x1 + 2.0 * x2 - 7.0) + sqr(2.0 * x1 + x2 - 5.0) );
			// optimum at P(1, 3) = 0

		case BukinFunctionNo6:
			return ( 100.0 * sqrt(abs(x2 - 0.01 * sqr(x1))) + 0.01 * abs(x1 + 10.0) );
			// optimum at P(-10, 1) = 0

		case MatyasFunction:
			return ( 0.26 * (sqr(x1) + sqr(x2)) - 0.48 * x1 * x2 );
			// optimum at P(0, 0) = 0

		case LeviFunctionNo13:
			return ( sqr(sin(3.0 * M_PI * x1)) + sqr(x1 - 1.0) * (1.0 + sqr(sin(3.0 * M_PI * x2))) + sqr(x2 - 1.0) * (1.0 + sqr(sin(2.0 * M_PI * x2))) );
			// optimum at P(1, 1) = 0

		case ThreeHumpCamelFunction:
			return ( 2.0 * sqr(x1) - 1.05 * pow(x1, 4.0) + pow(x1, 6.0) / 6.0 + x1 * x2 + sqr(x2) );
			// optimum at P(0, 0) = 0

		case EasomFunction:
			return ( -cos(x1) * cos(x2) * exp(-(sqr(x1 - M_PI) + sqr(x2 - M_PI))) );
			// optimum at P(pi, pi) = -1

		case CrossInTrayFunction:
			return ( -0.0001 * pow(abs(sin(x1) * sin(x2) * exp(abs(100.0 - sqrt(sqr(x1) + sqr(x2)) / M_PI))) + 1.0, 0.1) );
			// optimum at P(+-1.34941, +-1.34941) = -2.06261

		case EggholderFunction:
			return ( -(x2 + 47) * sin(sqrt(abs(0.5 * x1 + (x2 + 47.0)))) - x1 * sin(sqrt(abs(x1 - (x2 + 47.0)))) );
			// optimum at P(512, 404.2319) = -959.6407

		case HolderTableFunction:
			return ( -abs(sin(x1) * cos(x2) * exp(abs(1.0 - sqrt(sqr(x1) + sqr(x2)) / M_PI))) );
			// optimum at P(+-8.05502, +-9.66459) = -19.2085

		case McCormickFunction:
			return ( sin(x1 + x2) + sqr(x1 - x2) -1.5 * x1 + 2.5 * x2 + 1.0 );
			// optimum at P(-0.54719, -1.54719) = -1.9133

		case SchafferFunctionNo2:
			return ( 0.5 + (sqr(sin(sqr(x1) - sqr(x2))) - 0.5) / sqr(1.0 + 0.001 * (sqr(x1) + sqr(x2))) );
			// optimum at P(0, 0) = 0

		case SchafferFunctionNo4:
			return ( 0.5 + (sqr(cos(sin(sqr(x1) - sqr(x2)))) - 0.5) / sqr(1.0 + 0.001 * (sqr(x1) + sqr(x2))) );
			// optimum at P(0, 1.25313) = 0.292579

		case StyblinskiTangFunction:
			return ( (pow(x1, 4.0) - 16.0 * sqr(x1) + 5.0 * x1 + pow(x2, 4.0) - 16.0 * sqr(x2) + 5.0 * x2) / 2.0 );
			// optimum at P(-2.903534, -2.903534) = -78,33233

		}

		return 0.0;
	}

private:
	TestCostFunction testCostFunction;

	template<typename T>
	static inline T sqr(T x)
	{
		return x * x;
	}

	template<typename T>
	static inline T cube(T x)
	{
		return x * x * x;
	}

};
