#ifndef _astro_observation_h
#define _astro_observation_h

#include <astro/types.h>
#include <memory.h>

namespace peyton {
namespace asteroids {

/**
	\brief Class encapsulating SDSS observation of a moving object

	\todo Should this class reside in peyton::sdss namespace?
*/
class Observation {
public:
	enum {maxNameLen = 18};
public:
	MJD t0;
	int flags;

	char name[19];
	int id;			///< catalog ID (catalog dependent!)

	Radians ra, dec;
	Radians dra, ddec;

	double mag, umag;	///< magnitude and magnitude without phase angle correction

	double R, dist;	///< helio/geocentric distance
	double phase;	///< asteroid phase
public:
	Observation() { id = -1; t0 = 0; flags = 0; mag = 0; ra = dec = dra = ddec = 0; name[0] = 0; R = 0; phase = -1; }
	Observation & operator=(const Observation &o)
	{
		memcpy(this, &o, sizeof(Observation));
		return *this;
	}
};

} // namespace asteroids
} // namespace peyton

#endif
