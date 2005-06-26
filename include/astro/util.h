#ifndef _astro_util_h
#define _astro_util_h

#define FOREACHj(i_, x) for(typeof((x).begin()) i_ = (x).begin(); i_ != (x).end(); ++i_)
#define FOREACH(x) FOREACHj(i, x)

#define REVEACHj(i_, x) for(typeof((x).rbegin()) i_ = (x).rbegin(); i_ != (x).rend(); ++i_)
#define REVEACH(x) REVEACHj(i, x)

#define FORj(i, i0, i1) for(int i = i0; i != i1; ++i)
#define FOR(i0, i1) FORj(i, i0, i1)

#define REVj(i, i0, i1) for(int i = i0; i != i1; --i)
#define REV(i0, i1) REVj(i, i0, i1)

#define OSTREAM(T...) std::ostream &operator <<(std::ostream &out, T)
#define ISTREAM(T...) std::istream &operator >>(std::istream &in, T)

#include <astro/types.h>
#include <astro/constants.h>
#include <cmath>
#include <string>
#include <memory.h>
#include <ctype.h>
#include <stdio.h>

#ifndef NULL
#define NULL 0
#endif

namespace peyton {
/// everything else
namespace util {

	/// return approximate longitude of the Sun for a given \a time
	Radians approxSunLongitude(MJD time);

	/// remove whitespace from the begining of the string \a str
	std::string ltrim(const std::string &str, const std::string &whitespace = "\t ");
	/// remove whitespace from the end of the string \a str
	std::string rtrim(const std::string &str, const std::string &whitespace = "\t ");
	/// remove whitespace from the begining and the end of the string \a str
	std::string  trim(const std::string &str, const std::string &whitespace = "\t ");
	/** \deprecated Use the version which takes std::string as an argument */
	char *trim(char *txt);
	/** \deprecated Use the version which takes std::string as an argument */
	char *trim(char *dest, const char *src);

	/// Convert all occurences of \\" and \\' in a string to " and '
	std::string unescape(const std::string &str);

	/// convert string to lowercase
	inline std::string tolower(const std::string &s) { std::string o(s); FOREACH(o) { *i = ::tolower(*i); }; return o; }
	/// convert string to uppercase
	inline std::string toupper(const std::string &s) { std::string o(s); FOREACH(o) { *i = ::toupper(*i); }; return o; }

	/// convert size_t to std::string
	inline std::string str(size_t n) { char buf[20]; sprintf(buf, "%d", n); return buf; }
	/// convert int to std::string
	inline std::string str(int n) { char buf[20]; sprintf(buf, "%d", n); return buf; }
	/// convert double to std::string
	inline std::string str(double n, const char *fmt = "%f") { char buf[20]; sprintf(buf, fmt, n); return buf; }
}
namespace Util = util; // backwards compatibility hack

}

#define __peyton_util peyton::util

#endif
