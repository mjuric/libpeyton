#ifndef __astro_asteroids_h
#define __astro_asteroids_h

namespace peyton {
namespace asteroids {

	/// convert old style provisional designation \a _prov to packet MPC format \a dest
	char *prov2packed(char *dest, const char *_prov, char *orbitType = 0);

}
}

#define __peyton_asteroids peyton::asteroids

#endif
