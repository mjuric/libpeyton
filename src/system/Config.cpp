#include "config.h"

#include <astro/system/config.h>
#include <astro/util.h>
#include <astro/util/varexpand.h>
#include <astro/exceptions.h>
#include <astro/system/fs.h>
#include <cstdarg>

#include <sstream>
#include <fstream>

#if HAVE_BOOST_REGEX
#include <boost/regex.hpp>
#endif

using namespace peyton;
using namespace peyton::system;
using namespace peyton::exceptions;

std::ostream &peyton::system::operator<<(std::ostream &out, const Config &cfg)
{
	FOREACH(cfg)
	{
		out << i->first << " = " << i->second << "\n";
	}
	return out;
}

std::istream &peyton::system::operator>>(std::istream &in, Config::filespec &fs)
{
	char c;
	bool eatwhite = true;
	bool done = false;
	int curly = 0;

	fs.clear();

	while(in.get(c))
	{
		if(done) { break; }

		if(isspace(c))
		{
			if(eatwhite) { continue; }
			if(!curly) { break; }
		}
		else
		{
			eatwhite = false;

			if(c == '{')
			{
				curly++;
			}
			else if(c == '}')
			{
				if(curly == 1) { done = true; }
				if(curly > 0) { curly--; }
			}
		}
		
		fs += c;
	}

	if(in.eof())
	{
		if(!fs.empty())
		{
			in.clear();
		}
	}
	else
	{
		in.unget();
	}
	return in;
}

#if HAVE_BOOST_REGEX
size_t Config::get_matching_keys(std::set<std::string> &matches, const std::string &pattern) const
{
	boost::regex pat(pattern);
	boost::cmatch what;
	size_t count = 0;
	FOREACH(*this)
	{
		const char *key = i->first.c_str();
		if(!boost::regex_match(key, what, pat)) { continue; }

		matches.insert(i->first);
		count++;
	}
	return count;
}
#endif

int Config::get_subset(Config &dest, const std::string &prefix, bool strip_prefix)
{
	size_t len = prefix.size();
	size_t found = 0;
	FOREACH(*this)
	{
		const char *key = i->first.c_str();
		if(strncmp(key, prefix.c_str(), len) != 0) { continue; }
		dest.insert(std::make_pair(std::string(key + (strip_prefix ? len : 0)), i->second));
		found++;
	}
	return found;
}

void Config::load(std::istream &in, bool expandVars, bool allowEnvironmentVariables)
{
	std::string key, value, line;
	int lnum = 0;
	std::map<std::string, int> keyind;

	//
	// config files consist of key-value pairs, separated by whitespace, each on a single line
	// comments begin with #
	// empty lines are ignored
	//

	do {
		// load a line, allowing for continuations if '\' is the last character
		line.clear();
		int last;
		do {
			std::string tmp;
			getline(in, tmp); lnum++;
			tmp = util::trim(tmp);

			if(!line.empty()) { line += " "; }
			line += tmp;

			if(line[0] == '#') { break; }					// comments can't have continuations

			if(line.empty() || *line.rbegin() != '\\') { break; }		// line does not end in '\'
			line.erase(line.size()-1);
			if(!line.empty() && *line.rbegin() == '\\') { break; }		// line ends in \\
			line.erase(line.size()-1);

			line = util::trim(line);
		} while(true);

		if(line[0] == '#') { continue;}			// remove comment
		if(!line.size()) { continue; }			// empty line

		std::stringstream ss(line);
		getline(ss, key, '=');
		key = Util::rtrim(key);

		if(key.find_first_not_of(" \t") == std::string::npos) continue;	// empty line

		// check if this is an "array push" key
		if(key.size() > 2 && key.substr(key.size()-2) == "[]")
		{
			// find the next available index for this key
			std::string key0 = key.substr(0, key.size()-2);
			int index = keyind[key0];

			do {
				key = key0 + "[" + util::str(index) + "]";
				index++;
			} while(count(key));

			keyind[key0] = index;
		}

		// get value and trim whitespaces
		getline(ss, value);
		value = Util::ltrim(value);
		//if(value.size() == 0) { THROW(EAny, "No value specified in config file at line " + util::str(lnum)) + "(key = " + key + ")"; }

		// check if the string is quoted
		if(value[0] == '"')
		{
			// unquote the string, without the delimiting quotes
			int len = value[value.size()-1] == '"' ? 2 : 1;
			value = util::unescape(value.substr(1, value.size()-2));
		}

		(static_cast<std::map<std::string, std::string> &>(*this))[key] = value;

		//std::cout << key << " -> " << value << "\n";
	} while(!in.eof());

	if(expandVars) { expandVariables(allowEnvironmentVariables); }
}

void Config::expandVariables(bool allowEnvironmentVariables)
{
	util::expand_dict(*this, allowEnvironmentVariables);
}

void Config::load(const filespec &fspec, bool expandVars, bool allowEnvironmentVariables)
{
	// split fspec to filename + overrides
	std::string filename = util::trim(fspec.substr(0, fspec.find('{')));
	bool have_overrides = fspec.find('{') != std::string::npos;

	if(filename.size())
	{
		std::ifstream f(filename.c_str());
		if(!f.good()) { THROW(EFile, "Could not open [" + filename + "] file"); }

		// note: if overrides are present, delay variable expansion
		load(f, !have_overrides & expandVars, allowEnvironmentVariables);
	}

	// add overrides, if any
	if(have_overrides)
	{
		std::string overrides = util::trim(fspec.substr(filename.size()));	// get the '{ .... }' part
		if(overrides.size() < 2 || overrides[0] != '{' || overrides[overrides.size()-1] != '}')
		{
			THROW(EAny, "Error loading '" + fspec + "': The variables to be overridden must be enclosed in {}");
		}
		overrides = overrides.substr(1, overrides.size()-2); 	// remove {}
		FOREACH(overrides) { if(*i == ';') { *i = '\n'; } }	// replace ; with \n

		// load & expand variables
		std::istringstream f(overrides);
		load(f, expandVars, allowEnvironmentVariables);
	}
}

Config peyton::system::Config::globals;

Config::Config(const filespec &filename, const std::string &defaults, const bool expandVars, bool allowEnvironmentVariables)
{
	// source globals, unless this is the globals object itself.
	if(this != &globals)
	{
		*this = globals;
	}

	// load defaults
	if(defaults.size())
	{
		std::stringstream ss(defaults);
		load(ss, false);
	}

	if(filename.size())
	{
		load(filename, false);
	}

	if(expandVars) { expandVariables(allowEnvironmentVariables); }
}

const Config::Variant Config::operator[](const std::string &s) const throw()
{
	const_iterator it = find(s);

	if(it == end())
	{
		return Variant();
	}

	return Variant((*it).second);
}

const Config::Variant Config::get(const std::string &s) const
{
	const_iterator it = find(s);

	if(it == end())
	{
		THROW(EAny, "Configuration key '" + s + "' was not set.");
	}

	return Variant((*it).second);
}

const Config::Variant Config::get_any_of(int nargs, ...) const
{
	va_list aq;
	va_start(aq, nargs);

	for(int i=0; i != nargs; i++)
	{
		char *name = va_arg(aq, char *);
		const_iterator it = find(name);
		if(it != end())
		{
			va_end(aq);
			return Variant((*it).second);
		}
	}
	va_end(aq);

	va_start(aq, nargs);
	std::ostringstream out;
	for(int i=0; i != nargs; i++)
	{
		char *name = va_arg(aq, char *);
		if(i) { out << ", "; }
		out << name;
	}
	va_end(aq);
	THROW(EAny, "None of the requested configuration keys '" + out.str() + "'were set.");
}
