#ifndef __astro_system_config_h
#define __astro_system_config_h

#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <set>
#include <cstdlib>

#include "astro/util.h"

namespace peyton {
namespace system {

/**
@brief	A class for loading and accessing simple key-value configuration files

The class is derived from std::map<std::string, std::string> and includes methods
for loading key-value pairs from map.

The configuration file format is self-explanatory:

@verbatim
#
# Lines starting with # are ignored, as are whitespaces
#

var1 = string value
var2 = 12345
var3 = 1.223

home = /home/majuric
bin  = $home/bin
projects = $home/projects

key with spaces = This is a key with spaces
quoted value = "   This value is \"quoted\" to prevent stripping of trailing and leading spaces   "

var4 = To expand a key with spaces, enclose it in curly braces, eg ${key with spaces}
@endverbatim
*/
	class Config : public std::map<std::string, std::string>
	{
	protected:
		class Variant : public std::string
		{
		public:
			long long vlonglong() const 		{ return (long long)		atof(c_str()); } // Note: I use atof() to allow exponential notation for integers
			int vint() const 			{ return (int)			atof(c_str()); } // Note: I use atof() to allow exponential notation for integers
			unsigned vunsigned() const 		{ return (unsigned)		atof(c_str()); } // Note: I use atof() to allow exponential notation for integers
			unsigned long vulong() const 		{ return (unsigned long)	atof(c_str()); } // Note: I use atof() to allow exponential notation for integers
			unsigned long long vullong() const 	{ return (unsigned long long)	atof(c_str()); } // Note: I use atof() to allow exponential notation for integers
			double vdouble() const 			{ return atof(c_str()); }
			float vfloat() const 			{ return atof(c_str()); }
			bool vbool() const {
				if(*this == "true") return true;
				if(*this == "false") return false;
				return ((int)(*this)) != 0;
			}

			operator long long() const { return vlonglong(); }
			operator int() const { return vint(); }
			operator double() const { return vdouble(); }
			operator float() const { return vfloat(); }
			operator bool() const { return vbool(); }
			operator unsigned() const { return vunsigned(); }
			operator unsigned long() const { return vulong(); }
			operator unsigned long long() const { return vullong(); }

			template<typename A, typename B>
			operator std::pair<A, B>() const
			{
				std::pair<A, B> p;
				std::istringstream ss(*this);
				ss >> p.first >> p.second;
				return p;
			}

			template<typename T>
			int vvector(std::vector<T> &v) const;

			template<typename T>
			operator std::vector<T>() const { std::vector<T> v; vvector(v); return v; }

			Variant(const std::string &s = "") : std::string(s) {}
		};

	public:
		class filespec : public std::string
		{
		public:
			filespec() {}
			filespec(const std::string &s) : std::string(s) {}
			filespec(const char *s) : std::string(s) {}
			filespec(const filespec &s) : std::string(s) {}
			filespec& operator=(const filespec &a) { *this = (std::string&)a; return *this; }
		};
		//typedef std::string filespec;

	public:
		// Global definitions that will be present in _every_ (subsequently loaded) Config object
		static Config globals;

	public:
		/**
			Loads configuration from file, with the default configuration optionally specified
			as second parameter. Any keys not defined in the configuration file given will be
			set to values read from defaults parameter, and just then the variables will be expanded.
			This is usually what you want and expect.
			
			@param filename		Path to the config file (eg. /home/joe/x.conf)
			@param defaults		Text of the default configuration (not a filename!). This text
							will be loaded into sstringstream and passed to load().
			@param expandVars		Should variable expansion be performed
			@param allowEnvironmentVariables		If variable to be expanded cannot be found in the file, should it be searched in the environment.

			@throws peyton::exceptions::EAny		Throws an EAny exception in case anything goes wrong
		*/
		Config(const filespec &fspec = "", const std::string &defaults = "", bool expandVars = true, bool allowEnvironmentVariables = true);

		/**
			Loads configuration from file, optionally expanding the variables referenced in the values.

			@param fspec			Path to the config file (eg. /home/joe/x.conf)
			@param expandVars		Should the variables in the file be automatically expanded
			@param allowEnvironmentVariables		If variable to be expanded cannot be found in the file, should it be searched in the environment.
			
			@throws peyton::exceptions::EAny		Throws an EAny exception in case anything goes wrong
		*/
		void load(const filespec &filename, bool expandVars = true, bool allowEnvironmentVariables = true);

		/**
			Loads configuration from stream, optionally expanding the variables referenced in the values.

			@param in			Stream from which to load the configuration
			@param expandVars		Should the variables in the file be automatically expanded
			@param allowEnvironmentVariables		If variable to be expanded cannot be found in the file, should it be searched in the environment.

			@throws peyton::exceptions::EAny		Throws an EAny exception in case anything goes wrong
		*/
		void load(std::istream &in, bool expandVars = true, bool allowEnvironmentVariables = true);

		/**
			Variable-expands the loaded configuration file. Typically you would use
			this to explicitly ask for variables to be expanded after you've loaded
			them without expansion (e.g., if you're loading from multiple files, and
			want the expansion to occur after all the files have been loaded).

			@param allowEnvironmentVariables		If variable to be expanded cannot be found in the file, should it be searched in the environment.
		*/
		void expandVariables(bool allowEnvironmentVariables = true);

		/**
			Accessor for easy casting of configuration values to different types. Configuration
			values are stored as strings, but sometimes they might be integers or floating point
			numbers. Variant class has cast operators defined which allow a Variant to be automatically
			converted to any of these. That's why we defile operator[] which returns a Variant and
			allows expressions like this:
			
			@code
peyton::system::Config config("x.conf");
				
int x = config["int_value"];
double y = config["double_value"];
string z = config["string_value"];
			@endcode
		*/
		const Variant operator[](const std::string &key) const throw();
		
		/**
			If the configuration value name exists, set var = cfg[name], 
			else set var to dflt.
		*/
		template <typename T>
		bool get(T &var, const std::string &name, const T &dflt) const
		{
			std::map<std::string, std::string>::const_iterator it = find(name);
			var = it != this->end() ? Variant((*it).second) : dflt;
			return it != end();
		}

		// useful specialization for std::string when the default parameter is const char*
		bool get(std::string &var, const std::string &name, const char *dflt) const
		{
			return get<std::string>(var, name, dflt);
		}

		/**
			If the configuration value name exists, return cfg[name],
			else throw exception.
		*/
		const Variant get(const std::string &name) const;

		/**
			If any of the configuration keys exist, return
			their value.
		*/
		const Variant get_any_of(int nargs, ...) const;
		const Variant get_any_of(const std::string &s1, const std::string &s2) const { return get_any_of(2, s1.c_str(), s2.c_str()); }
		const Variant get_any_of(const std::string &s1, const std::string &s2, const std::string &s3) const { return get_any_of(3, s1.c_str(), s2.c_str(), s3.c_str()); }
		const Variant get_any_of(const std::string &s1, const std::string &s2, const std::string &s3, const std::string &s4) const { return get_any_of(4, s1.c_str(), s2.c_str(), s3.c_str(), s4.c_str()); }

#if HAVE_BOOST_REGEX
		// return all keys matching a given regular expression pattern pat. Note that the method
		// does _not_ clear the matches set before populating it. Returns the number of matches
		// found.
		size_t get_matching_keys(std::set<std::string> &matches, const std::string &pat) const;
#endif
		// copies all keyvals with keys beginning with prefix to dest, possibly stripping
		// the prefix if requested (default=yes). Returns the number of keyvals copied.
		int get_subset(Config &dest, const std::string &prefix, bool strip_prefix = true);
	};
	
	template<typename T>
	int Config::Variant::vvector(std::vector<T> &v) const
	{
		std::istringstream s(*this);
		T val;
		v.clear();
		while(s >> val)
		{
			v.push_back(val);
		}
		return v.size();
	}

	ISTREAM(Config::filespec &fs);
	OSTREAM(const Config &fs);
}
}

#define __peyton_system peyton::system

#endif
