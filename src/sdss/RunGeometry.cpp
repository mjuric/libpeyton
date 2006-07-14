#include <astro/system/config.h>
#include <astro/system.h>
#include <astro/system/fs.h>
#include <astro/system/log.h>
#include <astro/constants.h>
#include <astro/coordinates.h>

#include <fstream>
#include <stdio.h>
#include <unistd.h>

#include <astro/sdss/rungeometry.h>

using namespace peyton;
using namespace peyton::sdss;
using namespace peyton::system;
using namespace peyton::exceptions;

void Mask::setAt(Radians s, Radians e, Radians nu0)
{
	start = s; end = e;
	for(int i = 0; i != 6; i++) {
		nu[i][0] = nu0 + nuCam[i][0];
		nu[i][1] = nu0 + nuCam[i][1];
		
//		cout << nu[i][0]/ctn::d2r << " " << nu[i][1]/ctn::d2r << "\n";
	}
}

void Mask::expand(Radians x, Radians y)
{
	start -= x; coordinates::normalize(start);
	end += x; coordinates::normalize(end);
	for(int i = 0; i != 6; i++) {
		nu[i][0] -= y;
		nu[i][1] += y;
	}
}

bool Mask::contains(const Radians mu, const Radians nu, const int col)
{
	if(col != -1)
	{
		return coordinates::inBox(mu, nu, start, lo(col), end, hi(col));
	}
	
	for(int i = 0; i != 6; i++)
	{
		if(coordinates::inBox(mu, nu, start, lo(i), end, hi(i))) { return true; }
	}

	return false;
}

/**
	A word of caution about nuCam mask - it has been determined quasy-experimentaly
	and some pixels on the frame might actually fall out of the mask.
*/
Radians Mask::nuCam[6][2] = {
	{ 0.000000 * ctn::d2r, 0.225269 * ctn::d2r },
	{ 0.419741 * ctn::d2r, 0.645077 * ctn::d2r },
	{ 0.839260 * ctn::d2r, 1.064554 * ctn::d2r },
	{ 1.258912 * ctn::d2r, 1.484212 * ctn::d2r },
	{ 1.678480 * ctn::d2r, 1.903806 * ctn::d2r },
	{ 2.098364 * ctn::d2r, 2.323656 * ctn::d2r}
};

//extern Preferences pref;
RunGeometryDB::RunGeometryDB()
: conf(EnvVar("CONF_SDSS"))
{
	char buf[1001];
#if 0
	const char *ws = System::workspace();
	sprintf(buf, "%s/lib/%s", ws, "run_geometry.txt");
#else
	std::string runlist = conf["runlist file"];
#endif

	if(access(runlist.c_str(), 04)) {
		DEBUG(terminate) << "Cannot access run geometry file " << runlist;
		DEBUG(terminate) << "Is the 'runlist file' entry in " << EnvVar("CONF_SDSS") << " file incorrect?";
		exit(-1);
	}

	// length of half of exposure
	const double toffset = double(conf["time.toffset"])/(24.*3600.);

	std::ifstream f(runlist.c_str());

	RunGeometry geom;
	f.getline(buf, 1000);
	while(!f.eof()) {
		f.getline(buf, 1000);
		if(strlen(buf) < 3 || buf[0] == '#') continue;

		int n = sscanf(buf, "%d %lf %lf %lf %lf %lf %lf %lf %lf", 
			&geom.run, &geom.ra, &geom.dec, &geom.tstart, &geom.tend,
			&geom.node, &geom.inc, &geom.muStart, &geom.nu);
		if(n != 9) continue;

		// correct the time to time of center of the frame
		geom.tstart += toffset;
		geom.tend += toffset;

		geom.ra *= ctn::d2r;
		geom.dec *= ctn::d2r;
		geom.node *= ctn::d2r;
		geom.inc *= ctn::d2r;
		geom.muStart *= ctn::d2r;
		geom.nu *= ctn::d2r;
		
		geom.muEnd = geom.muStart + (geom.tend - geom.tstart)*ctn::pi2;
		geom.muEnd += 1361*ctn::d2r*(0.396 / 3600.); // width of last field (1361 pixels * 0.396" per pixel)
		coordinates::normalize(geom.muEnd);

		db[geom.run] = geom;

		
	}
}

const RunGeometry &RunGeometryDB::getGeometry(int run)
{
	std::map<int, RunGeometry>::iterator i = db.find(run);
	if(i == db.end())
	{
		THROW(ERunGeometry, "Run " + util::str(run) + " not found in run geometry database");
	}
	return (*i).second;
}
