#ifndef _astro_time_h
#define _astro_time_h

#include <astro/types.h>

namespace peyton {
/// time and date related functions and classes
namespace time {

	double calToJD(int y, int m, int d, double h);
	double calToMJD(int y, int m, int d, double h);
	void MJD2cal(double mjd, int &year, int &month, double &dd);
	double dayfrac(int h, int m, int s);

	char *toMPECTimeFormat(char *str, MJD time);
}
namespace Time = time;
}

#define __peyton_time peyton::time

#endif
