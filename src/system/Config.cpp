#include "config.h"

#include <astro/system/config.h>
#include <astro/util.h>
#include <astro/util/varexpand.h>
#include <astro/exceptions.h>
#include <astro/system/fs.h>

#include <sstream>
#include <fstream>

#if HAVE_BOOST_REGEX
#include <boost/regex.hpp>
#endif

using namespace peyton;
using namespace peyton::system;
using namespace peyton::exceptions;

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

void Config::load(std::istream &in, bool expandVars)
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
		getline(in, line); lnum++;



		line = Util::trim(line);
		if(!line.size()) { continue; }

		std::stringstream ss(line);
		getline(ss, key, '=');
		key = Util::rtrim(key);

		if(key.find_first_not_of(" \t") == std::string::npos) continue;	// empty line
		if(key[0] == '#') continue;							// comment

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
		if(value.size() == 0) { THROW(EAny, "No value specified in config file at line " + util::str(lnum)); }

		// check if the string is quoted
		if(value[0] == '"')
		{
			// unquote the string, without the delimiting quotes
			int len = value[value.size()-1] == '"' ? 2 : 1;
			value = util::unescape(value.substr(1, value.size()-2));
		}

		insert(value_type(key, value));

		//std::cout << key << " -> " << value << "\n";
	} while(!in.eof());

	if(expandVars) { util::expand_dict(*this); }
}

void Config::load(const std::string &filename, bool expandVars)
{
	std::ifstream f(filename.c_str());
	if(!f.good()) { THROW(EFile, "Could not open [" + filename + "] file"); }

	load(f, expandVars);
}

Config::Config(const std::string &filename, const std::string &defaults, const bool expandVars)
{
	if(defaults.size())
	{
		std::stringstream ss(defaults);
		load(ss, false);
	}

	if(filename.size())
	{
		load(filename, false);
	}

	if(expandVars) { util::expand_dict(*this); }
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
