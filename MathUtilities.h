#pragma once


class MathUtilities {
public:

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
