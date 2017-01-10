#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#define ITMAX 100
#define EPS 3.0e-7
#define FPMIN 1.0e-30

inline double errorFunc(const double& x)
{
	//The error function is twice the integral of the Gaussian distribution with 0 mean and variance of 1/2.

	double t = 1.0 / (1.0 + 0.5*abs(x));

	double res = 1 - t*exp(-x*x - 1.26551223 + t*( 1.00002368 +
											   t*( 0.37409196 + 
											   t*( 0.09678418 +
											   t*(-0.18628806 +
											   t*( 0.27886807 +
											   t*(-1.13520398 +
											   t*( 1.48851587 + 
											   t*(-0.82215223 + 
											   t*( 0.17087277))))))))));

	if(x>= 0.0)
		return res;
	else
		return -res;
}

inline double inverseErrorFunc(const double& x)
{
	//It is an approximation. Has to be changed for preciese calculations for big x values
	double res, xSq, logXSq, exp1;
	double a = 0.140012;

	xSq = 1-x*x;
	logXSq = log(xSq);
	exp1 = 2/(M_PI*a) + logXSq/2;

	res = exp1*exp1;
	res -= logXSq/a;
	res = sqrt(res);
	res -= exp1;
	res = sqrt(res);

	if(x < 0)
		res = -res;

	return res;
}

inline double inverseCompErrorFunc(const double& x)
{
	//Inverse complementary error function
	return inverseErrorFunc(1-x);
}

inline double logGammaFunc(const double& xx)
{
	double x,y,tmp,ser;
	double cof[6]={76.18009172947146,-86.50532032941677, 24.01409824083091,-1.231739572450155, 0.1208650973866179e-2,-0.5395239384953e-5};
	int j;
	y=x=xx;
	tmp=x+5.5;
	tmp -= (x+0.5)*log(tmp);
	ser=1.000000000190015;
	for (j=0;j<=5;j++) 
		ser += cof[j]/++y;
	
	return -tmp+log(2.5066282746310005*ser/x);
}

inline double incompleteGammaFunc(const double& a, const double& x)
{
	//Upper incomplete gamma function: Integration is from x to Inf

	int i;
	double an,b,c,d,del,h;
	double gln=logGammaFunc(a);
	
	b=x+1.0-a;
	c=1.0/FPMIN;
	d=1.0/b;
	h=d;
	for (i=1;i<=ITMAX;i++) {
		an = -i*(i-a);
		b += 2.0;
		d=an*d+b;
		if (fabs(d) < FPMIN) 
			d=FPMIN;
		c=b+an/c;
		if (fabs(c) < FPMIN) 
			c=FPMIN;
		d=1.0/d;
		del=d*c;
		h *= del;
		if (fabs(del-1.0) < EPS) break;
	}
	
	return exp(-x+a*log(x)-(gln))*h;
}

inline double inverseIncompleteGammaFunc(const double& a, const double& y, double tolerance=-1)
{
	//Inverse Incomplete Gamma Function for Upper Integration (x to Inf)
	
	//Constants
	double growConstant = 1.5;
	double maxIter = 1e6;
	if(tolerance < 0)
		tolerance = y/1000;

	//Initializations
	int iter = 0;
	double x = a;
	double xPrev = a;
	double xNew = a;
	double yEst = 0.0;

	while(fabs(y-yEst) > tolerance && iter <= maxIter)
	{
		x = xNew;
		yEst = incompleteGammaFunc(a,x);
		xNew = 0.5*(x+xPrev);

		if(yEst < y && xPrev >= x)
		{
			xNew = x/growConstant;
		}
		else if(yEst > y && xPrev <= x)
		{
			xNew = x*growConstant;
		}

		xPrev = x;
		iter++;
	}

	return x;
}