#ifndef __astro_io_printf
#define __astro_io_printf

#include <astro/exceptions.h>
#include "bits/auto_ptr.h"

// cout << io::format("%2.3f %d %o") << a << b << c;
//
// format f("%2.3f %d %o");

#include <iostream>
#include <cstdio>

namespace peyton {

namespace exceptions {
	SIMPLE_EXCEPTION(EIOFormat);
}

namespace io {

/**
	\brief Internal struct. Not to be used directly.

	This struct stores the information parsed from printf formatted format
	strings, if the format is traditional printf formatting (that is, not %@
	or %%(...)) formats. Look to eg. fprintf manpage for the meaning of
	each field.
	
	This format can be used by format_type procedures for deciding on the
	format of character output.
*/
struct trad_format
{
	char flag;
	int width;
	int prec;
	char length[2];
	char conversion;
};

/**
	\brief Auxilliary class for string formatting.
	
	See Formatter class documentation.
*/

class format : public std::string
{
public:
	bool autorewind;
public:
	format(const char *c, bool autorewind_ = false) : autorewind(autorewind_), std::string(c) {}
	format(const std::string &s, bool autorewind_ = false) : autorewind(autorewind_),  std::string(s) {}
};

//    cout << io::format("abs(v) = %(abs) %f %2.1f\n") << V3(1, 2, 3) << r << r;

/**
	\brief A printf-like, typesafe, character output class

	This class gives you the convenience of printf format strings, while
	maintaining typesafty. All of the basic printf formatting codes have
	been implemented, while some advanced features (eg., positional
	referencing using $m, where m is an integer) have not yet been
	implemented.
	
	Typesafty is achieved by defining 'output handlers' through
	specializing the format_type template for each type you want to output.
	All of C/C++ basic types have already been specialized.
	
	This class is intendend to be used in three most common patterns:
	-# implicitly through the format() class and directly outputing to a
	stream
	-# implicitly through the format() class and automatically converted to a string
	-# explicitly instantiated

	The code to follow illustrates each pattern of usage. V3 is a vector
	class defined in peyton::math namespace.

	\code
	// pattern 1
	cout << io::format("abs(v) = %(abs) %f %2.1f\n") << V3(1, 2, 3) << r << r;

	// pattern 2	
	std::string s = io::format("abs(v) = %(abs) %f %2.1f\n") << V3(1, 2, 3) << r << r;

	// pattern 3
	io::Formatter fs(cout, "abs(v) = %2.1f %(polar) %3.2f %f %@\n", true);
	fs << r << V3(1, 2, 3) << r << r << V3(1, 1, 1);
	fs << r << V3(1, 2, 3) << r << r << V3(1, 1, 1);
	\endcode

*/

class Formatter
{
public:
	std::string formatt;	///< Format string
	std::ostream &out;	///< Stream to output to
	bool sstrm;		///< Internal flag - true if out is an owned (allocated) ostringstream
public:
	int	at,		///< Current location in the format string
		to;		///< Location to which the current format code extends
				///< (i.e. the format code is in [at, to) interval
	trad_format tf;	///< Parsed formatting codes for 'traditional' sprintf formats (built-in types)
	bool autorewind;	///< After the end of the format string is reached,
				///< if there's more output to be done, should we start from the beginning of
				///< the format string, or just pass the data directly to the stream
public:
	/**
		Output to a private ostringstream. This constructor should _never_ be directly used.
		
		Formatted output is accessible through std::string() operator.
	*/
	explicit Formatter(const std::string &fmt, bool arw = true);

	/**
		Construct a Formatter object outputing to stream \a out_, with format \fmt.
		
		If the stream should automatically rewind (see rewind()), set \a autorewind_ to true.
	*/
	Formatter(std::ostream &out_, const std::string &fmt, bool autorewind_ = true)
	: formatt(fmt), out(out_), sstrm(false), at(0), to(0), autorewind(autorewind_)
	{
		pop();
	}

	~Formatter();

	/**
		Returns the contents of private ostringstream, or an empty string if
		the object has no private stringstream. Should not be used directly, but
		through the <code>std::string = format("blabla %d %d") << a << b;</code> idiom.
	*/
	operator std::string() const;

	/**
		\brief Returns the currently relevant piece of format.
		
		Use this for your own format codes when writing format_type specializations
		of your own types.
	*/
	std::string front() { return formatt.substr(at, to-at); }

	void pop();
	bool empty() { return at == to; }
	void rewind() { at = 0; to = 0; pop(); }
};

typedef bits::auto_ptr<Formatter> PFormatter;

//
// Operator which creates transient parser object from format object
//
inline PFormatter operator <<(std::ostream &out, const format &fmt)
{
	return PFormatter(new Formatter(out, fmt, fmt.autorewind));
}

template<typename T>
inline PFormatter operator<<(const format &fmt, const T &x)
{
	PFormatter f(new Formatter(fmt, fmt.autorewind));
	return f << x;
}

#if 0
inline std::ostream &operator <<(std::ostream &out, const Formatter &f)
{
	return out << (std::string)f;
}
#endif

//
// Default output template for formatted output
// Calls format_type(ostream out, string fmt, const T &x) which is expected to be
// specialized for a type being streamed.
// Note that PFormatter is the type of f, not PFormatter &
//
template<typename T>
PFormatter operator <<(PFormatter f, const T &x)
{
	*f << x;
	return f;
}

template<typename T>
Formatter &operator <<(Formatter &f, const T &x)
{
	if(f.empty()) {
		if(f.autorewind) { f.rewind(); }
		if(f.empty()) { f.out << x; return f; }
	}

	format_type(f, x);
	f.pop();
	return f;
}

//
// Default, unspecialized format_type template
// PROBLEM: Should this throw an exception, or even not be here at all?
// RESOLUTION: It should be here - you sometimes want to use the format string just for
//			positioning, and leave the output in the default format. Just nuke if
//			the user attempts anything but %@ formatting
//
template<typename T>
inline void format_type(Formatter &f, const T &v)
{
	if(f.formatt[f.at+1] == '(' ||
		(f.tf.conversion != 0 && (f.tf.width != -1 || f.tf.prec != -1 || f.tf.length[0]))
	) {
//		std::cerr << f.tf.conversion << " " << f.tf.width << " " << f.tf.prec << " " << (int)f.tf.length[0] << "\n";
		THROW(peyton::exceptions::EIOFormat,
			std::string("Trying to format a datatype with no formatting rules defined, using a format specification [")
				 + f.front() + "]. This is not allowed - define a format_type() specialization for your type first");
	}

	f.out << v;
}

//
// specializations for built in types
// - we directly specialize operator << in order to use information already obtained
// from the parser in Formatter::tf structure
//
void format_type(Formatter &f, const int &x);
void format_type(Formatter &f, const unsigned &x);
void format_type(Formatter &f, const double &x);
void format_type(Formatter &f, const char &x);
void format_type(Formatter &f, const bool &x);
void format_type(Formatter &f, const char *x);

inline void format_type(Formatter &f, const std::string &x) { format_type(f, x.c_str()); }
inline void format_type(Formatter &f, const float &x)       { format_type(f, (double)x); }
inline void format_type(Formatter &f, const char &x)        { format_type(f, (int)x); }

} // namespace io
} // namespace peyton

#define __peyton peyton
#define __peyton_io peyton::io
#define __peyton_exceptions peyton::exceptions

#endif
