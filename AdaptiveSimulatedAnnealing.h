#pragma once

#include <iostream>
#include <algorithm>
#include "MathUtilities.h"
#include "AbstractCostFunction.h"
#include "MersenneTwister19937ar.h"

using namespace std;


// https://en.wikipedia.org/wiki/Simulated_annealing
// https://www.mathworks.com/help/gads/how-simulated-annealing-works.html

enum VariableDomain { ContinousDomain, IntegerDomain };								// default : ContinousDomain
enum AnnealingMechanism { AnnealFast, AnnealBoltzman };								// default : AnnealBoltzman
enum CoolingMechanism { CoolingExponential, CoolingFast, CoolingBoltzman };			// default : CoolingExponential
enum AcceptanceMechanism { AcceptanceBoltzman, AcceptanceAdaptive };				// default : AcceptanceAdaptive

struct SAOptimimumSolution {
	SAOptimimumSolution(int dimension, double initialTemperature)
	{
		optimumCostValue = numeric_limits<double>::infinity();
		x_optimum = new double[dimension];
		temperature = initialTemperature;
		iteration = 0;
	}

	double optimumCostValue;
	double* x_optimum;
	double temperature;
	int iteration;
};

class AdaptiveSimulatedAnnealing {
public:
	AdaptiveSimulatedAnnealing(double initialTemperature = 100.0, 
		int iterationPerDimension = 1000,
		double convergenceTolerance = 1e-4,
		VariableDomain variableDomain = ContinousDomain, 
		AnnealingMechanism annealingMechanism = AnnealBoltzman, 
		CoolingMechanism coolingMechanism = CoolingExponential, 
		AcceptanceMechanism acceptanceMechanism = AcceptanceAdaptive)
	{
		this->initialTemperature = initialTemperature;
		this->iterationPerDimension = iterationPerDimension;
		this->convergenceTolerance = convergenceTolerance;
		this->variableDomain = variableDomain;
		this->annealingMechanism = annealingMechanism;
		this->coolingMechanism = coolingMechanism;
		this->acceptanceMechanism = acceptanceMechanism;

		showInformationPeriod = 0;
	}

	void perturbInNeighbour(double* x_s, double* x_new, double* neighbourDirection, double temperature)
	{
		//@annealingfast(default)		—-> Step length equals the current temperature, and direction is uniformly random.
		//@annealingboltz				—-> Step length equals the square root of temperature, and direction is uniformly random.

		// find a Normal distributed random direction in unit  length
		double perturbTemparature = (annealingMechanism == AnnealFast ? temperature : sqrt(temperature));

		bool outOfFeasibleSet;
		do
		{
			outOfFeasibleSet = false;

			double directionSum = 0.0;
			for (int i = 0; i < dimension; i++) {
				neighbourDirection[i] = randomGenerator.genrand_real_NormalDistributed();
				directionSum += MathUtilities::sqr(neighbourDirection[i]);
			}
			const double vectorNormalizer = (1.0 / sqrt(directionSum));

			for (int i = 0; i < dimension; i++) {
				neighbourDirection[i] *= vectorNormalizer;
			}

			// find new point using unit Normal distributed direction
			for (int i = 0; i < dimension; i++) {
				x_new[i] = x_s[i] + perturbTemparature * neighbourDirection[i];

				if (variableDomain == IntegerDomain) {
					x_new[i] = (int)(x_new[i]);
				}

				if (x_new[i] < lowerBound || x_new[i] > upperBound) {
					outOfFeasibleSet = true;
					perturbTemparature /= 2;
					break;
				}
			}
		} 
		while (outOfFeasibleSet);
	}

	// k = 0 to maximumIteration
	double getTemperatureAtIteration(int iteration)
	{
		//@temperatureexp (default)		—-> T = T0 * 0.95^iteration
		//@temperaturefast				—-> T = T0 / (iteration + 1)
		//@temperatureboltz				—-> T = T0 / (log(iteration + 1) + 1)

		double temperature = 0.0;

		switch (coolingMechanism)
		{
		case CoolingExponential:	temperature = initialTemperature * pow(0.95, iteration);			break;
		case CoolingFast:			temperature = initialTemperature / (iteration + 1.0);				break;
		case CoolingBoltzman:		temperature = initialTemperature / (log(iteration + 1.0) + 1.0);	break;
		}

		const double minimumTemperature = 1e-8;
		temperature = max(temperature, minimumTemperature);

		return temperature;
	}

	bool isAccepted(double E_s, double E_s_new, double temperature, double optimumTemperature)
	{
		// if E(snew) < E(s) then accept, otherwise if P(E(s), E(snew), T) <= random(0, 1) then accept : then s ← snew

		if (E_s_new < E_s) 
			return true;
		else {
			const double acceptanceTemperature = (acceptanceMechanism == AcceptanceBoltzman ? temperature : optimumTemperature);

			const double delta = (E_s_new - E_s);
			//const double H = (1.0 / (1.0 + exp(delta / acceptanceTemperature)));
			const double H = exp(-delta / acceptanceTemperature);
			const double R = randomGenerator.genrand_real1();

			return (H >= R);
		}
	}

	SAOptimimumSolution minimize(AbstractCostFunction& costFunction, double* x_initial)
	{
		/*
		Let s = s0
		For k = 0 through kmax(exclusive) :
			T ← temperature(k ∕ kmax)
			Pick a random neighbour, snew ← neighbour(s)
			If P(E(s), E(snew), T) ≥ random(0, 1), move to the new state:
				s ← snew
		Output : the final state s
		*/

		// initialize local variables
		dimension = costFunction.Dimension();
		lowerBound = costFunction.getLowerBound();
		upperBound = costFunction.getUpperBound();

		SAOptimimumSolution optimimumSolution(dimension, initialTemperature);

		double* x_s = new double[dimension];
		double* x_s_new = new double[dimension];
		double* neighbourDirection = new double[dimension];

		// Let s = s0
		for (int i = 0; i < dimension; i++) {
			x_s[i] = x_initial[i];

			if (variableDomain == IntegerDomain) {
				x_s[i] = (int)(x_s[i]);
			}
		}

		const double initialError = costFunction.evaluate(x_s);

		double E_s = initialError;

		const int maximumIteration = (dimension * iterationPerDimension);
		const int stallIterationLimit = (maximumIteration / 5);
		int stallIterationCounter = 0;

		int iteration;
		for (iteration = 0; iteration <= maximumIteration; iteration++) {
			// temperature cooling
			const double temperature = getTemperatureAtIteration(optimimumSolution.iteration);
		
			// pick a random neighbour, snew ← neighbour(s)
			perturbInNeighbour(x_s, x_s_new, neighbourDirection, temperature);

			// if energy decreases then always accept, if energy increases accept with a chance (higher temperature and lower increase in energy means more chance to accept)
			const double E_s_new = costFunction.evaluate(x_s_new);

			// keep best
			if (E_s_new < optimimumSolution.optimumCostValue) {
				optimimumSolution.optimumCostValue = E_s_new;
				optimimumSolution.temperature = temperature;
				optimimumSolution.iteration = iteration;

				for (int i = 0; i < dimension; i++) {
					optimimumSolution.x_optimum[i] = x_s_new[i];
				}
			}

			// show iteration information
			if (showInformationPeriod > 0) {
				if (iteration % showInformationPeriod == 0) {
					if (iteration == 0) {
						cout << "E_s      " << "\t\t" << "E_s_new  " << "\t\t" << "optimum cost" << endl;
						cout << "---------" << "\t\t" << "---------" << "\t\t" << "------------" << endl;
					}
					cout << E_s << "\t\t" << E_s_new << "\t\t" << optimimumSolution.optimumCostValue << endl;
				}
			}

			// check if SA is stalled (if so break the iteration)
			if (abs(E_s_new - E_s) < convergenceTolerance) {
				stallIterationCounter++;

				if (stallIterationCounter > stallIterationLimit) {
					break;
				}
			}
			else
				stallIterationCounter = 0;

			// accept with probability
			if (isAccepted(E_s, E_s_new, temperature, optimimumSolution.temperature)) {
				E_s = E_s_new;

				for (int i = 0; i < dimension; i++) {
					x_s[i] = x_s_new[i];
				}
			}
		}
		optimimumSolution.iteration = iteration;

		delete[] x_s;
		delete[] x_s_new;
		delete[] neighbourDirection;

		if (showInformationPeriod > 0) {
			cout << endl;
		}

		return optimimumSolution;
	}

	void setShowInformationPeriod(int showInformationPeriod)
	{
		this->showInformationPeriod = showInformationPeriod;
	}

private:
	double initialTemperature;
	int iterationPerDimension;
	double convergenceTolerance;
	VariableDomain variableDomain;
	AnnealingMechanism annealingMechanism;
	CoolingMechanism coolingMechanism;
	AcceptanceMechanism acceptanceMechanism;

	MersenneTwister19937ar randomGenerator;

	int dimension;
	double lowerBound;
	double upperBound;

	int showInformationPeriod;

};
