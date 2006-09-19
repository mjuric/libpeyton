#ifndef _astro_util_h
#define _astro_util_h

#define FOREACH2j(C, i_, x) for(C i_ = (x).begin(); i_ != (x).end(); ++i_)
#define FOREACH2(C, x) FOREACH2j(C, i, x)

#define REVEACH2j(C, i_, x) for(C i_ = (x).rbegin(); i_ != (x).rend(); ++i_)
#define REVEACH2(C, x) REVEACH2j(C, i, x)

#define FOREACHj(i_, x) for(typeof((x).begin()) i_ = (x).begin(); i_ != (x).end(); ++i_)
#define FOREACH(x) FOREACHj(i, x)

#define REVEACHj(i_, x) for(typeof((x).begin()) i_ = (x).rbegin(); i_ != (x).rend(); ++i_)
#define REVEACH(x) REVEACHj(i, x)

#define FORj(i, i0, i1) for(int i = i0; i != i1; ++i)
#define FOR(i0, i1) FORj(i, i0, i1)

#define REVj(i, i0, i1) for(int i = i0; i != i1; --i)
#define REV(i0, i1) REVj(i, i0, i1)

#define OSTREAM(T) std::ostream &operator <<(std::ostream &out, T)
#define ISTREAM(T) std::istream &operator >>(std::istream &in, T)

#include <astro/types.h>
#include <astro/constants.h>
#include <cmath>
#include <string>
#include <memory.h>
#include <ctype.h>
#include <stdio.h>
#include <cxxabi.h>

#ifndef NULL
#define NULL 0
#endif

namespace peyton {
/// everything else
namespace util {

	/// return approximate longitude of the Sun for a given \a time
	Radians approxSunLongitude(peyton::MJD t);

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
	inline std::string tolower(const std::string &s) { std::string o(s); FOREACH2(std::string::iterator, o) { *i = ::tolower(*i); }; return o; }
	/// convert string to uppercase
	inline std::string toupper(const std::string &s) { std::string o(s); FOREACH2(std::string::iterator, o) { *i = ::toupper(*i); }; return o; }

	/// convert size_t to std::string
	inline std::string str(size_t n) { char buf[20]; sprintf(buf, "%d", n); return buf; }
	/// convert int to std::string
	inline std::string str(int n) { char buf[20]; sprintf(buf, "%d", n); return buf; }
	/// convert double to std::string
	inline std::string str(double n, const char *fmt = "%f") { char buf[20]; sprintf(buf, fmt, n); return buf; }

	/// type-name demangler
	template<typename T>
		std::string type_name()
		{
			std::string tmp;
			int status;
	
			char *name = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
			if(status != 0)
			{
				// TODO: thrown an exception here? If so, make sure to free(name) first!
				tmp = "demangling error (status code = ";
				tmp += str(status);
			}
			else
			{
				tmp = name;
			}
			free(name);

			return tmp;
		}

	/// type-name demangler
	template<typename T>
		std::string type_name(const T& t)
		{
			return type_name<T>();
		}
}
namespace Util = util; // backwards compatibility hack

}

#define __peyton_util peyton::util

#endif
