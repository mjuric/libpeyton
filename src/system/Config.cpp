#include <astro/system/config.h>
#include <astro/util.h>
#include <astro/util/varexpand.h>
#include <astro/exceptions.h>
#include <astro/system/fs.h>

#include <sstream>
#include <fstream>

using namespace peyton;
using namespace peyton::system;
using namespace peyton::exceptions;

void Config::load(std::istream &in, bool expandVars)
{
	std::string key, value, line;
	int lnum = 0;

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
