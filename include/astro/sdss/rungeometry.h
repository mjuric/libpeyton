#ifndef __rungeometry_h
#define __rungeometry_h

#include <astro/types.h>
#include <astro/constants.h>
#include <astro/system/config.h>
#include <astro/exceptions.h>

#include <map>

namespace peyton {

namespace exceptions {
	SIMPLE_EXCEPTION(ERunGeometry);
}

/// SDSS specific classes and functions
namespace sdss {

	struct RunGeometry {
		int run;
		Radians ra, dec, node, inc, muStart, muEnd, nu;
		MJD tstart, tend;

		inline Radians length() { return (muEnd < muStart ? (muEnd+peyton::ctn::pi2) : muEnd) - muStart; }
	};

	class RunGeometryDB {
	protected:
		peyton::system::Config conf;
	public:
		std::map<int, RunGeometry> db;
	public:
		RunGeometryDB();
		void getGeometry(int run, RunGeometry &geom) { geom = getGeometry(run); }
		const RunGeometry &getGeometry(int run);
	};

	class Mask {
	protected:
		static Radians nuCam[6][2];
	public:
		Radians nu[6][2], start, end;

		Mask(const RunGeometry &geom) { setAt(geom.muStart, geom.muEnd, geom.nu); }
		Mask(Radians s, Radians e, Radians nu0) { setAt(s, e, nu0); }
		Mask() { setAt(0, 0, 0); }

		void setAt(Radians s, Radians e, Radians nu0);
		void expand(Radians x, Radians y);

		inline Radians lo(const int col) const { return nu[col][0]; }
		inline Radians hi(const int col) const { return nu[col][1]; }

		bool contains(const Radians mu, const Radians nu, const int col = -1);

		inline Radians length() { return (end < start ? (end+peyton::ctn::pi2) : end) - start; }
		inline Radians width(int col) { return hi(col) - lo(col); }
	};

}
}

#define __peyton_sdss peyton::sdss

#endif
