#include <astro/skypoint.h>
#include <astro/coordinates.h>

#include <iostream>

using namespace peyton;
using namespace peyton::coordinates;

Radians SkyPoint::distance(const SkyPoint &p)
{
	return peyton::coordinates::distance(ra, dec, p.ra, p.dec);
}

inline std::ostream& operator << (std::ostream& os, const SkyPoint& s)
{
	os << s.ra/peyton::ctn::d2r << " " << s.dec/peyton::ctn::d2r;
	return os;
}

inline std::istream& operator >> (std::istream& is, SkyPoint& s)
{
	is >> s.ra >> s.dec; s.toRad(); return is;
}
