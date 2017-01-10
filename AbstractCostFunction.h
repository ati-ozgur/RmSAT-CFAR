#pragma once

#include <limits>

using namespace std;


class AbstractCostFunction {
public:
	AbstractCostFunction(int dimension)
	{
		this->dimension = dimension;

		lowerBound = -numeric_limits<double>::infinity();
		upperBound = numeric_limits<double>::infinity();
	}

	// abstract
	virtual double evaluate(double* x) = 0;

	inline int Dimension() const { return dimension;  }

	double getLowerBound() const { return lowerBound; }

	double getUpperBound() const { return upperBound; }

	void setLowerBound(double lowerBound)
	{
		this->lowerBound = lowerBound;
	}

	void setUpperBound(double upperBound)
	{
		this->upperBound = upperBound;
	}

protected:
	int dimension;
	double lowerBound;
	double upperBound;

private:

};
