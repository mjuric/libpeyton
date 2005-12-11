#include <astro/system/options.h>

#include <astro/util.h>
#include <astro/system/log.h>

#include <sstream>
#include <iostream>
#include <map>
#include <set>
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
	FOREACH2(std::vector<Option>::iterator, options) {
		Option &o = *i;

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
			o.setvalue(result[o.key] = o.defaultvalue); // set the default value for this option
		}

		// construct options struct
		struct option oo = { o.name.c_str(), o.argument, NULL, o.shortname };
		opts.push_back(oo);
	}
	static struct option null_option = { NULL, 0, false, 0};
	opts.push_back(null_option);

	FOREACH2(std::vector<Option>::iterator, args) {
		Option &o = *i;
		
		if(o.hasdefaultvalue)
		{
			o.setvalue(result[o.key] = o.defaultvalue); // set the default value for this argument
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

		Option &o = ((c == 1) ? args[nargs-1] : options[index]);
		optionsGiven[o.key] = true;				// Note that the option was found
		o.setvalue(result[o.key] = optarg ? optarg : o.value);	// If the option has an argument, set the map value to that argument ...
									// ... and set it to o.value, otherwise
		// DEBUG(verbose, "Setting option " << o.key << " to '" << o.value << "'");
	}

	if(nargs < args.size() && args[nargs].argument != Option::optional)
	{
		THROW(EOptions, string("To few command line arguments found (") + str(nargs) + ')');
	}
}

std::string argapp(const Option &o, const std::string &sep = "=")
{
	switch(o.argument)
	{
		case Option::optional: return std::string("[") + sep + "[argument]";
		case Option::required: return sep + "<argument>";
	}
	return "";
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
		FOREACH2(std::vector<Option>::iterator, args) {
			Option &o = *i;

			if(i != args.begin())              { out << " "; }

			if(o.argument == Option::required) { out << "<" + o.key << ">"; }
			else                               { out << "[" + o.key << "]"; }
		}
		out << "\n\n";
		FOREACH2(std::vector<Option>::iterator, args) {
			Option &o = *i;
			out << "    *  " << o.key << " -- " << o.description << "\n";
		}
		out << "\n";
	}

	if(options.size() > 11) { // taking into account negative number argument hack
		out << "Available options:\n\n";
		FOREACH2(std::vector<Option>::iterator, options) {
			Option &o = *i;
			if(o.shortname == '.' || (o.shortname >= '0' && o.shortname <= '9')) continue;

			out << "    * ";
			if(o.shortname != 0)
			{
				out << "-" << o.shortname << argapp(o, "");
			}
			if(o.name != std::string(1, o.shortname))
			{
				out << (o.shortname != 0 ? ", " : "") << "--" << o.name << argapp(o, "=");
			}

			if(o.argument != Option::none)
			{
				if(o.hasdefaultvalue)
				{
					out << " (default '" << o.defaultvalue << "')";
				}
				else if(o.variable.bound() && o.variable.hasdefaultvalue())
				{
					out << " (default '" << o.variable.to_string() << "')";
				}
			}
			out << "\n";

			out << "        " << o.description << "\n";
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

opt::arg peyton::system::opt::arg_required("required");
opt::arg peyton::system::opt::arg_optional("optional");
opt::arg peyton::system::opt::arg_none("none");
opt::empty_tag peyton::system::opt::empty;

void opt::binding::setvalue(const std::string &s)
{
	switch(type)
	{
		case t_double: *var.v_double = atof(s.c_str()); break;
		case t_float: *var.v_float = atof(s.c_str()); break;
		case t_int: *var.v_int = atoi(s.c_str()); break;
		case t_short: *var.v_short = atoi(s.c_str()); break;
		case t_bool:
			*var.v_bool = s == "1" || tolower(s) == "true";
			break;
		case t_string: *var.v_string = s; break;
	};
}

std::string opt::binding::to_string() const
{
	std::ostringstream ss;
	switch(type)
	{
		case t_double: ss << *var.v_double; break;
		case t_float: ss << *var.v_float; break;
		case t_int: ss << *var.v_int; break;
		case t_short: ss << *var.v_short; break;
		case t_bool: ss << *var.v_bool; break;
		case t_string: ss << *var.v_string; break;
	};
	ss.flush();
	return ss.str();
}

void opt::shortname::apply(Option &o) const
{
	o.shortname = val.size() ? val[0] : 0;
	if(o.shortname != 0)
	{
		o.name = o.shortname;
		o.value = o.shortname;
	}
};

void opt::name::apply(Option &o) const
{
	o.name = val;
	if(o.name.size())
	{
		o.key = o.name;
		o.value = o.name;
	}
}

void opt::binding::apply(Option &o) const
{
	o.variable = *this;
	if(type == t_bool) { o.value = "true"; }
}

void opt::key::apply(Option &o) const
{
	if(val.size())
	{
		o.key = val;
	}
}

void opt::arg::apply(Option &o) const
{
	switch(val[0])
	{
	case 'r': o.argument = Option::required; break;
	case 'o': o.argument = Option::optional; break;
	case 'n': o.argument = Option::none; break;
	}
}

void opt::value::apply(Option &o)     const { o.value = val; }
void opt::def_value::apply(Option &o) const { o.defaultvalue = val; o.hasdefaultvalue = true; }
void opt::desc::apply(Option &o)      const { o.description = val; }
void opt::empty_tag::apply(Option &o) const { /* noop */ }

opt::arg::arg(const std::string &s)
: base(s, 30)
{
	if(s != "required" && s != "optional" && s != "none")
	{
		THROW(peyton::exceptions::EOptions, "Option argument must be one of 'required', 'optional' or 'none'. PS: It's better to use arg::required, arg::optional, arg::none.");
	}
}

struct lt_o_spec
{
	bool operator()(const opt::base *a, const opt::base *b) const { return a->priority < b->priority; }
};

void Options::option(
	const opt::base &spec1,
	const opt::base &spec2,
	const opt::base &spec3,
	const opt::base &spec4,
	const opt::base &spec5,
	const opt::base &spec6,
	const opt::base &spec7,
	const opt::base &spec8,
	const opt::base &spec9
	)
{
	// note: the ability of conflicting options to override each other
	// (eg., for arg_required given _after_ arg_none to be the dominant option)
	// will break badly if order of elements in stl::multimap is implementation
	// dependent (coudn't figure out if it is).
	std::multiset<const opt::base *, lt_o_spec> specset;
	specset.insert(&spec1);
	specset.insert(&spec2);
	specset.insert(&spec3);
	specset.insert(&spec4);
	specset.insert(&spec5);
	specset.insert(&spec6);
	specset.insert(&spec7);
	specset.insert(&spec8);
	specset.insert(&spec9);

	Option o("", "");
	FOREACH(specset)
	{
		(*i)->apply(o);
	}

	options.push_back(o);
}
