#include <astro/system/options.h>

#include <astro/util.h>
#include <astro/system/log.h>

#include <sstream>
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <getopt.h>
//#include <iostream>

using namespace peyton::system;
using namespace std;
using namespace peyton;
using namespace peyton::exceptions;
using namespace peyton::util;


Authorship Authorship::majuric("Mario Juric", "Mario Juric", "mjuric@astro.princeton.edu");
Authorship Authorship::unspecified("U. N. Specified", "U. N. Specified", "e-mail unspecified");
Version Version::unspecified("version unspecified");

std::string Option::nodefault;

Options::Options(const std::string &description_, const Version &version_, const Authorship &author_ ) 
	: peyton::system::Config(), description(description_), version(version_), author(author_)
{
	// This hack fixes the problem when one wants a negative number as an argument
	// without this, the number with a '-' in front is interpreted as an option.
	// To avoid this behavior, we define -0, -1, ... -9 and -. options, and handle them separately
	// in Options::parse()
	
	FOR(0, 10)
	{
		string s(string("-") + str(i));
		option(s, s, '0'+i, "", Option::optional);
	}
	option(".", ".", '.', "", Option::optional);
}

// since we're using option as a name of a method in Option class, this
// little hack avoids the namespace clash
typedef option getopt_option;

void Options::parse(int argc, char **argv)
{
	// Lookup table for looking up short->long names
	std::map<char, int> shortlong;
	std::map<std::string, std::string> &result = (std::map<std::string, std::string> &)(*this);

	// Construct the short option format string for getopt

	// From man getopt(3):
	// "If the  first character  of optstring is '-', then each non-option
	// argv-element is handled as if it were the argument of an option
	// with character code 1."
	std::string optstring("-");

	std::vector<getopt_option> opts;
	FOREACH(options) {
		const Option &o = *i;

		if(o.key == "") { THROW(peyton::exceptions::EOptions, "Option key cannot be an empty string"); }

		if(o.shortname != 0) // if this option has short option as well
		{
			optstring += o.shortname;
			switch(o.argument)
			{
				case Option::optional: optstring += "::"; break;
				case Option::required: optstring += ":"; break;
			}
			shortlong[o.shortname] = opts.size();
		}

		if(o.hasdefaultvalue)
		{
			result[o.key] = o.defaultvalue; // set the default value for this option
		}

		// construct options struct
		struct option oo = { o.name.c_str(), o.argument, NULL, o.shortname };
		opts.push_back(oo);
	}
	static struct option null_option = { NULL, 0, false, 0};
	opts.push_back(null_option);

	FOREACH(args) {
		const Option &o = *i;
		
		if(o.hasdefaultvalue)
		{
			result[o.key] = o.defaultvalue; // set the default value for this argument
		}
	}

//	FOREACH(short2long) { cout << (*i).first << " -> " << (*i).second << "\n"; }

	int index = -1;
	char c;
	nargs = 0; // number of command line arguments read
	string numhackarg;
	while((c = getopt_long(argc, argv, optstring.c_str(), &(*opts.begin()), &index)) != -1) {
		if(c == '?') { THROW(EOptions, std::string("Option [") + (char)optopt + "] unknown!"); }
		if(c == ':') { THROW(EOptions, std::string("Option [") + (char)optopt + "] must be specified with a parameter!"); }

		// negative numbers as arguments hack
		if(c == '.' || (c >= '0' && c <= '9'))
		{
			numhackarg = "-"; numhackarg += c;
			if(optarg) { numhackarg += optarg; }
			optarg = const_cast<char *>(numhackarg.c_str());
			c = 1;
//			cout << "NUMHACK ACTIVE: [" << numhackarg << "] [" << optarg << "]\n";
		}

		// command line arguments
		if(c == 1) {
			if(args.size() == nargs) { THROW(EOptions, "Too many arguments specified on the command line"); }
			nargs++;
		}

		// if a short option is found, getopt_long returns the option character, and not the long option index
		if(c != 0) { index = shortlong[c]; }

		const Option &o = ((c == 1) ? args[nargs-1] : options[index]);
		optionsGiven[o.key] = true;				// Note that the option was found
		result[o.key] = optarg ? optarg : o.value;	// If the option has an argument, set the map value to that argument ...
										// ... and set it to o.value, otherwise
		// DEBUG(verbose, "Setting option " << o.key << " to '" << o.value << "'");
	}

	if(nargs < args.size() && args[nargs].argument != Option::optional)
	{
		THROW(EOptions, string("To few command line arguments found (") + str(nargs) + ')');
	}
}

std::string Options::usage(char **argv)
{
	string program(argv == NULL ? "program.x" : argv[0]);

	std::ostringstream out;
	out << "- - - - - - - - - - - - - - - \n";
	
	if(argv != NULL) {
		out << "Program   : " << argv[0] << "\n";
	} else {
		out << "Short instructions" << "\n";
	}

	out << std::string(version) << "\n";
	out << "\n";

	if(!description.empty()) {
		out << description << "\n\n";
	}

	if(!args.empty()) {
		out << "Usage : ";
		out  << program << " ";
		FOREACH(args) {
			Option &o = *i;

			if(i != args.begin())              { out << " "; }

			if(o.argument == Option::required) { out << "<" + o.key << ">"; }
			else                               { out << "[" + o.key << "]"; }
		}
		out << "\n\n";
		FOREACH(args) {
			Option &o = *i;
			out << "      - " << o.key << " -- " << o.description << "\n";
		}
		out << "\n";
	}

	if(options.size() > 11) { // taking into account negative number argument hack
		out << "Options available:\n\n";
		FOREACH(options) {
			Option &o = *i;
			if(o.shortname == '.' || (o.shortname >= '0' && o.shortname <= '9')) continue;
			out << "\t--" << o.name;
			switch(o.argument)
			{
				case Option::optional: out << "[=argument]"; break;
				case Option::required: out << "=argument"; break;
			}

			if(o.hasdefaultvalue)
			{
				out << " (default '" << o.defaultvalue << "')";
			}
			out << "\n";

			out << "\t\t" << o.description << "\n";
		}
		out << "\n";
	}

	out << std::string(author) << "\n";

	out << "- - - - - - - - - - - - - - - \n";
	return out.str();
}


void Options::argument(const std::string &key, const std::string &description, const Option::Argument argument, const std::string &defaultvalue)
{
	// after the first optional argument, all subsequent ones are optional as well
	Option::Argument a_ = (!args.empty() && args.back().argument == Option::optional) ? Option::optional : argument;

	args.push_back(Option(key, "$", 0, "", a_, defaultvalue, description));
}
