#ifndef _astro_error_h
#define _astro_error_h

/**
	\file

	\deprecated This header file is obsolete. Use log.h instead.
*/

#if 0

class Error {
public:
	enum {err, warn};
	static int type;
	static int code;
	static char description[1000];

	static void error(int c, const char *d, ...);
	static void warning(int c, const char *d, ...);
};

#define ERRCHECK(condition) if(condition)
#define ASSERT(cond) if(!(cond)) { Error::error(-1, "Assertion [" #cond "] failed at %s, %s:%d", __PRETTY_FUNCTION__, __FILE__, __LINE__); abort(); }

#else

#include <astro/system/log.h>

#endif

#endif
