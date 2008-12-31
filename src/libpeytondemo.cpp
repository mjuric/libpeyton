#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <astro/sdss/photometry.h>
#include <astro/exceptions.h>
#include <astro/system/options.h>
#include <iostream>

#include <astro/math.h>
#include <astro/util.h>
#include <astro/useall.h>
using namespace std;

int test_options(int argc, char **argv);
int demo_binarystream();
int main_diskmemorymodel(int argc, char *argv[]);
int main_fpnumber(int argc, char *argv[]);

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

// check modulo code
void moduloTest()
{
	double v[] = {-1.4, -3.8, -41, -.1, .1, 3.3, 5.7, 6, 8};
	FOR(0, 9)
	{
		cerr << v[i] << " -> " << math::modulo(v[i], 3) << "\n";
	}
}

int config_test()
{
	Config cfg;
	cfg.load("observe.conf");

	std::set<std::string> keys;
	cfg.get_matching_keys(keys, "module\\[.+\\]");

	FOREACH(keys) { std::cout << *i << " "; }
	std::cout << "\n";

	return 0;
}

int main(int argc, char *argv[])
{
	//return 0;
	//moduloTest(); return 0;

	return config_test();
	return test_options(argc, argv);
	return demo_binarystream();
	return main_fpnumber(argc, argv);
	return main_diskmemorymodel(argc, argv);
}
