#ifndef __astro_system_config_h
#define __astro_system_config_h

#include <string>
#include <map>

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
			operator long long() const { return atoll(c_str()); }
			operator int() const { return atoi(c_str()); }
			operator double() const { return atof(c_str()); }
			operator float() const { return atof(c_str()); }
			operator bool() const {
				if(*this == "true") return true;
				if(*this == "false") return false;
				return ((int)(*this)) != 0;
			}

			Variant(const std::string &s = "") : std::string(s) {}
		};
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

			@throws peyton::exceptions::EAny		Throws an EAny exception in case anything goes wrong
		*/
		Config(const std::string &filename = "", const std::string &defaults = "", bool expandVars = true);

		/**
			Loads configuration from file, optionally expanding the variables referenced in the values.

			@param filename		Path to the config file (eg. /home/joe/x.conf)
			@param expandVars		Should the variables in the file be automatically expanded
			
			@throws peyton::exceptions::EAny		Throws an EAny exception in case anything goes wrong
		*/
		void load(const std::string &filename, bool expandVars = true);

		/**
			Loads configuration from stream, optionally expanding the variables referenced in the values.

			@param in			Stream from which to load the configuration
			@param expandVars		Should the variables in the file be automatically expanded

			@throws peyton::exceptions::EAny		Throws an EAny exception in case anything goes wrong
		*/
		void load(std::istream &in, bool expandVars = true);

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
	};

}
}

#define __peyton_system peyton::system

#endif
