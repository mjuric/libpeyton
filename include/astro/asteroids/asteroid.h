#ifndef _astro_asteroid_h
#define _astro_asteroid_h

#include <astro/types.h>
#include <memory.h>

namespace peyton {
/// %Asteroid research related classes and functions
namespace asteroids {

/**
	\brief Physical, dynamical and other data of an asteroid
	
	This is basically a simple struct which holds data usually given in asteroid catalogs.
*/
class Asteroid {
public:
	enum {maxNameLen = 18};
public:
	int numeration;
	int type;		///< 0 - asteroid, 1 - comet
	char orbitType;	///< type of comet orbit - see MPC reporting format documentation for explanation
	char name[19];	///< provisional designation or name of the asteroid
	int id;		///< catalog specific ID of the asteroid (eg., the line number in ASTORB)

	double h, g;	///< (H, G) absolute magnitudes
	MJD t0;		///< Epoch of osculation, MJD
	double elements[6]; ///< a, e, i, longitude of ascending node, argument of perihelion, M. Angular units are radians.

	int arc;		///< Arc of observation
public:
	Asteroid() { memset(this, 0, sizeof (Asteroid)); name[0] = 0; id = -1; }
	Asteroid &operator=(const Asteroid &obj) { memcpy(this, &obj, sizeof (Asteroid)); return *this; }
};

} // namespace asteroids
} // namespace peyton

#endif
