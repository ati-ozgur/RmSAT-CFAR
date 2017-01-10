#pragma once

#include <algorithm>

using namespace std;

template<typename T>
static int partition(T* list, int startI, int endI, int pivotI)
{
	T pivotValue = list[pivotI];
	list[pivotI] = list[startI];
	list[startI] = pivotValue;

	int storeI = startI + 1;//no need to store @ pivot item, it's good already.
	//Invariant: startI < storeI <= endI

	while (storeI < endI && list[storeI] <= pivotValue) ++storeI; //fast if sorted

	//now storeI == endI || list[storeI] > pivotValue
	//so elem @storeI is either irrelevant or too large.
	for (int i = storeI + 1; i < endI; ++i) {
		if (list[i] <= pivotValue) {
			//swap_elems(list, i, storeI);
			swap(list[i], list[storeI]);
			++storeI;
		}
	}

	int newPivotI = storeI - 1;
	list[startI] = list[newPivotI];
	list[newPivotI] = pivotValue;

	//now [startI, newPivotI] are <= to pivotValue && list[newPivotI] == pivotValue.
	return newPivotI;
}

template<typename T>
static T quickSelect(T* list, int k, int startI, int endI)
{
	while (true) {
		// Assume startI <= k < endI
		int pivotI = (startI + endI) / 2; //arbitrary, but good if sorted
		int splitI = partition<T>(list, startI, endI, pivotI);

		if (k < splitI)
			endI = splitI;
		else if (k > splitI)
			startI = splitI + 1;
		else //if (k == splitI)
			return list[k];
	}
	//when this returns, all elements of list[i] <= list[k] iif i <= k
}

template<typename T>
T quickSelect(T* list, int n, int k)
{
	return quickSelect<T>(list, k, 0, n);
}
