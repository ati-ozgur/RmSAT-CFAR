#pragma once

#include <iostream>
#include "NonlinearTestCostFunctions.h"
#include "AdaptiveSimulatedAnnealing.h"

using namespace std;


class AdaptiveSimulatedAnnealingTest {
public:
	static void execute()
	{
		const int dimension = 2;
		const TestCostFunction testCostFunction = SphereFunction;
		NonlinearTestCostFunctions costFunction(dimension, testCostFunction);

		double* x_initial = new double[dimension];
		set_x_initial(costFunction, x_initial);

		AdaptiveSimulatedAnnealing asa;
		//asa.setShowInformationPeriod(1);
		SAOptimimumSolution optimimumSolution = asa.minimize(costFunction, x_initial);

		show_x_optimum(costFunction, optimimumSolution);

		delete[] optimimumSolution.x_optimum;
	}

private:
	static void set_x_initial(AbstractCostFunction& costFunction, double* x_initial)
	{
		const int dimension = costFunction.Dimension();

		cout << std::fixed << "X initial = [ ";
		const double lowerBound = costFunction.getLowerBound();
		const double upperBound = costFunction.getUpperBound();

		MersenneTwister19937ar randomGenerator;

		for (int i = 0; i < dimension; i++) {
			x_initial[i] = randomGenerator.genrand_real_inInterval(lowerBound / 2.0, upperBound / 2.0);
			cout << x_initial[i] << " ";
		}
		cout << "]" << endl;
		const double initialCostValue = costFunction.evaluate(x_initial);
		cout << "X initial cost value = " << initialCostValue << endl << endl;
	}

	static void show_x_optimum(AbstractCostFunction& costFunction, SAOptimimumSolution& optimimumSolution)
	{
		const int dimension = costFunction.Dimension();

		cout << "Iteration count = " << optimimumSolution.iteration << endl;
		cout << "Temperature = " << optimimumSolution.temperature << endl;
		cout << "X* = [ ";
		for (int i = 0; i < dimension; i++) {
			cout << optimimumSolution.x_optimum[i] << " ";
		}
		cout << "]" << endl;
		cout << "X* optimum cost value = " << optimimumSolution.optimumCostValue << endl << endl;
	}

};
