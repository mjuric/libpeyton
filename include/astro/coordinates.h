#ifndef _astro_coordinates_h
#define _astro_coordinates_h

#include <astro/types.h>
#include <astro/constants.h>

namespace peyton {
/// Functions for manipulating and converting spherical coordinates
namespace coordinates {

	void eclequ(double lambda, double beta, double &ra, double &dec);
	void equecl(double ra, double dec, double &lambda, double &beta);

	/// conversion from Equatorial coordinate system to SDSS Great Circles .
	void equgcs(Radians node, Radians inc, Radians ra, Radians dec, Radians &mu, Radians &nu);
	/// conversion from SDSS Great Circles coordinate system to Equatorial c.s.
	void gcsequ(Radians node, Radians inc, Radians mu, Radians nu, Radians &ra, Radians &dec);

	void equgal(Radians ra, Radians dec, Radians &l, Radians &b);
	void galequ(Radians l, Radians b, Radians &ra, Radians &dec);

	/// SDSS GCS to Galactic coordinates
	void gcsgal(Radians node, Radians inc, Radians mu, Radians nu, Radians &l, Radians &b);
	/// galgcs - Galactic to SDSS GCS coordinates
	void galgcs(Radians node, Radians inc, Radians l, Radians b, Radians &mu, Radians &nu);

	/// angular distance between two points on a sphere
	Radians distance(Radians xr1, Radians yr1, Radians xr2, Radians yr2);

	/// angular distance between two angles on a circle, going from phi0 to phi1
	/// Note that distance from phi0 = 5deg to phi0=4deg is 359deg, not 1deg
	inline Radians distance(Radians phi0, Radians phi1)
	{
		return phi0 < phi1 ? (phi1 - phi0) : ((phi1 + ctn::pi2) - phi0);
	}

	/// is a point in box. Takes into consideration the compactness of spherical coordinates.
	bool inBox(Radians ra, Radians dec, Radians ralo, Radians declo, Radians rahi, Radians dechi);

	/// rotate velocity vector from Great circle coord. sys. to equatorial c.s.
	void rot_vel(Radians node, Radians inc, Radians mu, Radians nu, Radians vmu, Radians vnu, Radians &vra, Radians &vdec);

	/// reduce the angle to [0, 2*pi) range
	inline void normalize(Radians &l) { while(l >= ctn::pi2) l -= ctn::pi2; while(l < 0) l += ctn::pi2; }



	/// constant for formatCoord() mpec_forward parameter
	const int FC_MPEC_FORMAT	= 1;
	/// constant for formatCoord() mpec_forward parameter
	const int FC_NO_FRAC_SECONDS	= 2;
	/// make a textual representation of coordinates based on format string fmt
	void formatCoord(const char *fmt, char *out, const double v);
	/// make a textual representation of coordinates, suitable for MPEC inclusion
	void formatCoord(char *ras, char *decs, const double ra, const double dec, const int mpec_format = 0);
}

// backwards compatibility
namespace Coordinates = coordinates;

}

#define __peyton_coordinates peyton::coordinates

#endif
