#ifndef _astro_skypoint_h
#define _astro_skypoint_h

#include <astro/types.h>
#include <astro/constants.h>

#include <iosfwd>

namespace peyton {
namespace coordinates {

/**
	\brief A point in spherical coordinates having (lon, lat)

	This is here only because the SATOOLS package uses it.

	\deprecated Use peyton::math::V2 instead
*/
class SkyPoint {
public:
	Radians ra, dec;
public:
	SkyPoint() { ra = dec = 0; }
	SkyPoint(double raX, double decX) {ra = raX; dec = decX;}

	Radians distance(const SkyPoint &p);

	void toRad() { ra *= peyton::ctn::d2r; dec *= peyton::ctn::d2r; }
	void toDeg() { ra /= peyton::ctn::d2r; dec /= peyton::ctn::d2r; }
};

std::ostream& operator << (std::ostream& os, const SkyPoint& s);
std::istream& operator >> (std::istream& is, SkyPoint& s);

}
}

#define __peyton_coordinates peyton::coordinates

#endif
