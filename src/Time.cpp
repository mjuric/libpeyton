#include <astro/time.h>
#include <stdio.h>

void peyton::time::MJD2cal(double J, int &year, int &month, double &dd)
{
	// convert to JD
	J = J + 2400000.5;

	int day;
	long a, c, d, x, y, jd;
	int BC;
	
	if( J < 1721425.5 ) /* January 1.0, 1 A.D. */
		BC = 1;
	else
		BC = 0;
	
	jd = J + 0.5; /* round Julian date up to integer */
	
				  /* Find the number of Gregorian centuries
				  * since March 1, 4801 B.C.
	*/
	a = (100*jd + 3204500L)/3652425L;
	
	/* Transform to Julian calendar by adding in Gregorian century years
	* that are not leap years.
	* Subtract 97 days to shift origin of JD to March 1.
	* Add 122 days for magic arithmetic algorithm.
	* Add four years to ensure the first leap year is detected.
	*/
	c = jd + 1486;
	if( jd >= 2299160.5 )
		c += a - a/4;
	else
		c += 38;
		/* Offset 122 days, which is where the magic arithmetic
		* month formula sequence starts (March 1 = 4 * 30.6 = 122.4).
	*/
	d = (100*c - 12210L)/36525L;
	/* Days in that many whole Julian years */
	x = (36525L * d)/100L;
	
	/* Find month and day. */
	y = ((c-x)*100L)/3061L;
	day = c - x - ((306L*y)/10L);
	month = y - 1;
	if( y > 13 )
		month -= 12;
	
	/* Get the year right. */
	year = d - 4715;
	if( month > 2 )
		year -= 1;
	
	/* Day of the week. */
	a = (jd + 1) % 7;
	
	/* Fractional part of day. */
	dd = day + J - jd + 0.5;
	
	if( BC )
	{
		year = -year + 1;
	}
	
	day = dd;
}



double peyton::time::calToMJD(int y, int m, int d, double h)
{
	return calToJD(y, m, d, h) - 2400000.5;
}

double peyton::time::calToJD(int yX, int mX, int dX, double h)
{
	long year = yX, month = mX;
	double day = dX + h/24.0;
	
	long y, a, b, c, e, m;
	double J;
	
	
	/* The origin should be chosen to be a century year
	* that is also a leap year.  We pick 4801 B.C.
	*/
	y = year + 4800;
	if( year < 0 )
	{
		y += 1;
	}
	
	/* The following magic arithmetic calculates a sequence
	* whose successive terms differ by the correct number of
	* days per calendar month.  It starts at 122 = March; January
	* and February come after December.
	*/
	m = month;
	if( m <= 2 )
	{
		m += 12;
		y -= 1;
	}
	e = (306 * (m+1))/10;
	
	a = y/100;	/* number of centuries */
	if( year <= 1582L )
	{
		if( year == 1582L )
		{
			if( month < 10 )
				goto julius;
			if( month > 10)
				goto gregor;
			if( day >= 15 )
				goto gregor;
		}
julius:
		b = -38;
	}
	else
	{ /* -number of century years that are not leap years */
gregor:
	b = (a/4) - a;
	}
	
	c = (36525 * y)/100; /* Julian calendar years and leap years */
	
						 /* Add up these terms, plus offset from J 0 to 1 Jan 4801 B.C.
						 * Also fudge for the 122 days from the month algorithm.
	*/
	J = b + c + e + day - 32167.5;
	return( J );
}

double peyton::time::dayfrac(int h, int m, int s)
{
	return h / 24. + m / (24.*60.) + s / (24.*3600.);
}

char *peyton::time::toMPECTimeFormat(char *str, MJD time)
{
	int y, m; double d;
	Time::MJD2cal(time, y, m, d);

	sprintf(str, "%d %02d %8.5f", y, m, d);

	return str;
}

