#ifndef __astro_math_h
#define __astro_math_h

#include <astro/constants.h>
#include <astro/types.h>
#include <cmath>
#include <cstdlib>

namespace peyton {
namespace math {

	/// sine of \a x, where \a x is in degrees
	inline double dsin(double x) { return std::sin(x*ctn::d2r); }
	/// cosine of \a x, where \a x is in degrees
	inline double dcos(double x) { return std::cos(x*ctn::d2r); }

	/// convert radians to degrees
	inline double deg(const peyton::Radians r) { return r/peyton::ctn::d2r; }
	/// convert degrees to radians
	inline Radians rad(const double d) { return d*peyton::ctn::d2r; }
	/// convert radians to arc minutes
	inline Radians arcmin(const double r) { return 60.*deg(r); }
	/// convert radians to arc seconds
	inline Radians arcsec(const double r) { return 3600.*deg(r); }

	/// return a random number between x0 and x1 (default range is [0, 1])
	inline float rnd(float x0 = 0, float x1 = 1) { return x0 + (float(std::rand())/float(RAND_MAX)) * (x1 - x0); }

	/// return <x> modulo <mod>
	inline double modulo(double x, double mod) { return x - mod*std::floor(x/mod); }

} // namespace math
} // namespace peyton

#define __peyton_math peyton::math

#endif
