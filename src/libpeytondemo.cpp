#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <astro/sdss/photometry.h>
#include <astro/exceptions.h>
#include <iostream>

#include <astro/useall.h>
using namespace std;

int main_diskmemorymodel(int argc, char *argv[]);

#if 0
int main(int argc, char *argv[])
{
	try
	{
		float f = 1.1;
		float f0 = 1e+9;
		float ferr = 0.1;
		float lupt, luptErr, mag, magErr;

		// test photometry routines
		phot::luptitude(f, ferr, phot::uband, lupt, luptErr, f0);
		phot::pogson(f, ferr, mag, magErr, f0);
		cout << lupt << " " << luptErr << " -- " << mag << " " << magErr << "\n";

		phot::fluxfromluptitude(lupt, luptErr, phot::uband, f, ferr, f0);
		cout << lupt << " " << luptErr << " == " << f << " " << ferr << "\n";
		phot::fluxfrompogson(mag, magErr, f, ferr, f0);
		cout << mag << " " << magErr << " == " << f << " " << ferr << "\n";
		
		return EXIT_SUCCESS;
	} catch(EAny &e) {
		e.print();
	}
}
#endif

int main(int argc, char *argv[])
{
	return main_diskmemorymodel(argc, argv);
}
