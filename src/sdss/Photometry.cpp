#include <astro/sdss/photometry.h>
#include <astro/constants.h>
#include <astro/math.h>

#include <cmath>

using namespace peyton;
using namespace std;

void sdss::phot::johnson(double &V, double &B, double r, double g) throw()
{
#if 0
	// Fukugita
	r -= .11; g += .12;

	V = .4666667*g + .533333*r;
	B = 1.419048*g - .419048*r;
#endif

#if 0
	// Krisciunas
	r -= .10; g += .08;

	V = .5625*r + .4375*g;
	B = -.479166666*r + 1.47916666*g;
#endif

#if 1
	V = r + 0.44*(g-r) - 0.02;
	B = 1.04*(g-r) + 0.19 + V;
#endif

//	cout << r+.11 << " " << g-.12 << " " << V << " " << B << "\n";
}

// u g r i z
static const double BPRIME[] = { 1.4e-10, 0.9e-10, 1.2e-10, 1.8e-10, 7.4e-10 };
static const double POGSON = 2.5/ctn::ln10;

// ## given flux and zeropoint $1 and its error $2 in a given band $3,
// ## set $4 and $5 to luptitude and its error
void sdss::phot::luptitude(float f, float fluxErr, int band, float &lupt, float &luptErr, float f0) throw()
{
	const double bp = BPRIME[band];

	const double x = (f/f0) / (2*bp);
	const double mu = log(x+sqrt(sqr(x)+1));
	lupt = -POGSON*(mu+log(bp));

	const double aux = sqrt(sqr(f)+sqr(2*bp*f0));
	luptErr = POGSON*fluxErr/aux;
}

void sdss::phot::pogson(float f, float fErr, float &mag, float &magErr, float f0) throw()
{
	mag = -2.5 * (log10(f/f0));
	magErr = POGSON * (fErr/f);
}

void sdss::phot::fluxfromluptitude(float lupt, float luptErr, int band, float &f, float &fErr, float f0) throw()
{
	const double bp = BPRIME[band];

	f = 2*f0*bp*sinh(-lupt/POGSON-log(bp));
	const double aux = sqrt(sqr(f)+sqr(2*bp*f0));
	fErr = aux/POGSON*luptErr;
}

void sdss::phot::fluxfrompogson(float mag, float magErr, float &f, float &fErr, float f0) throw()
{
	f = f0*pow(10., -mag/2.5);
	fErr = f/POGSON * magErr;
}
