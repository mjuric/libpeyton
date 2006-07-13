#include <astro/system/options.h>

#include <astro/util.h>
#include <astro/system/log.h>

#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <iterator>
#include <algorithm>
#include <memory>
#include <vector>
#include <list>
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

Option::Option(const Option &o)
	: variable(o.variable), mapkey(o.mapkey),
	name(o.name), optval(o.optval),
	parameter(o.parameter),
	defval(o.defval), description(o.description),
	deffromvar(o.deffromvar)
{ }

Option& Option::operator=(const Option &o)
{
	variable = o.variable;
	mapkey = o.mapkey; name = o.name; optval = o.optval;
	parameter = o.parameter; defval = o.defval; description = o.description;
	deffromvar = o.deffromvar;

	return *this;
}


Options::Options(const std::string &argv0, const std::string &description_, const Version &version_, const Authorship &author_ ) 
	: peyton::system::Config(), description(description_), ver(version_), author(author_), 
	ignore_unknown_opts(false), stop_after_final_arg(false), defoptsadded(false)
{
	//program = string(argv0 == NULL ? "program.x" : argv0);
	program = argv0;
	size_t pos = program.rfind('/');
	if(pos != string::npos) { program = program.substr(pos+1); }
	
	options.reserve(50);
	args.reserve(50);
}

// since we're using option as a name of a method in Option class, this
// little hack avoids the namespace clash
typedef option getopt_option;

bool Options::handle_argument(const std::string &arg, Option *&o)
{
	if(nargs == args.size())
	{
		if(ignore_unknown_opts) { return false; }

		THROW(EOptions, "Too many command line arguments specified");
	}

	// store
	o = &args[nargs++];
	ASSERT(o->name.size());
	store(*o, arg, o->name[0], true);

	return true;
}

void Options::store(Option &o, const std::string &value, const std::string &opt, bool is_argument)
{
	// store to string map
	insert(make_pair(o.mapkey, value));

	// parse and store into bound variable
	if(!o.notify(value))
	{
		std::string tmp = is_argument ? "argument" : "option";
		THROW(EOptions, std::string("Error parsing the value of ") + tmp + " " + opt + ", " + o.variable.type() + " expected.");
	}
}

bool Option::notify(const std::string &s)
{
	istringstream ss(s);
	return variable.setval(ss);
}

struct version_tag {} t_version;
struct help_tag {} t_help;

std::istream &opt::binding<version_tag >::setval(std::istream &in) { THROW(EOptionsVersion, ""); }
std::ostream &opt::binding<version_tag >::getval(std::ostream &out) const { out << "t_version"; }

std::istream &opt::binding<help_tag >::setval(std::istream &in) { THROW(EOptionsHelp, ""); }
std::ostream &opt::binding<help_tag >::getval(std::ostream &out) const { out << "t_help"; }

void Options::add_standard_options()
{
	if(!defoptsadded)
	{
		option("v").addname("version").bind(t_version, false).desc("Version and author information");
		option("h").addname("help").bind(t_help, false).desc("This help page");

		defoptsadded = true;
	}
}

void Options::parse(int argc, char **argv, option_list *unparsed)
{
	// create argument list
	option_list dummy;
	option_list &args = unparsed != NULL ? *unparsed : dummy;
	args.clear();
	FOR(1, argc) { args.push_back(argv[i]); }
	
	parse(*unparsed);
}

void Options::parse(option_list &args)
{
	// add standard options, if they were not already added
	add_standard_options();

	// create option lookup maps and set default values
	shortOptMap.clear(); longOptMap.clear();
	FOREACH(options)
	{
		Option &o = *i;
		ASSERT(o.name.size());
		if(o.defval.size())
		{
			if(o.deffromvar)
			{
				// if the default value was extracted from bound variable,
				// just store it into the map
				insert(make_pair(o.mapkey, o.defval));
			}
			else
			{
				store(o, o.defval, o.name[0], false);
			}
		}
		FOREACHj(j, o.name)
		{
			std::string &name = *j;
			ASSERT(name.size());
			if(name.size() == 1) { shortOptMap[name[0]] = &*i; }
			else { longOptMap[name] = &*i; }
		}
		// some courtesy sanity checking
		ASSERT(!(o.optval.size() && o.parameter == Option::Required))
		{
			std::cerr << o.mapkey << " has a nonempty value '" << o.optval << "', but requires a parameter? This makes no sense because the parameter will always overwrite the default value.\n";
		}
	}
	bool inoptargs = false;
	nreqargs = 0;
	FOREACH(this->args)
	{
		Option &o = *i;
		if(o.deffromvar)
		{
			// if the default value was extracted from bound variable,
			// just store it into the map
			insert(make_pair(o.mapkey, o.defval));
		}
		else
		{
			store(o, o.defval, o.name[0], true);
		}

		switch(o.parameter)
		{
		case Option::Required:
		case Option::None:
			if(inoptargs) THROW(EOptions, "PROGRAM ERROR (please notify the author): required command line arguments not allowed after an optional argument has been specified.");
			nreqargs++;
		break;
		case Option::Optional:
			inoptargs = true;
		break;
		}

		// some courtesy sanity checking
		ASSERT(!(o.defval.size() && o.parameter == Option::Required))
		{
			std::cerr << o.mapkey << " has a default value '" << o.defval << "', but requires a parameter? This makes no sense because the parameter will always overwrite the default value.\n";
		}
	}

	// parse the arguments in the list
	nargs = 0;
	bool options_ended = false;
	std::list<const char *>::iterator i = args.begin(), k;
	while(i != args.end())
	{
		if(nargs == this->args.size() && stop_after_final_arg) { break; }

		std::string arg = *i;
		k = i;
		bool succ = false;
		
		// option handling
		Option *o = NULL;
		if(arg.size() > 1 && arg[0] == '-' && !options_ended)
		{
			size_t argpos = 0;

			// forced end to option parsing
			if(arg == "--")
			{
				options_ended = true;
				succ = false; // leave the marker in array
			}
			else
			{
				// number?
				std::istringstream ss(arg);
				double d;
				if(ss >> d)
				{
					succ = handle_argument(arg, o);
				}
				else
				{
					std::string opt, pfxopt;
					if(arg.size() > 1 && arg[1] == '-') // long option?
					{
						size_t eqpos = arg.find('=');
						if(eqpos != string::npos)
						{
							argpos = eqpos + 1;
						}
						else
						{
							eqpos = arg.size();
						}
						opt = arg.substr(2, eqpos-2);
						pfxopt = arg.substr(0, eqpos);
				
						if(longOptMap.count(opt)) o = longOptMap[opt];
					}
					else	// short option
					{
						if(shortOptMap.count(arg[1]))
						{
							o = shortOptMap[arg[1]];
							opt = arg[1];
							pfxopt = "-";
							pfxopt += opt;
						}
						if(arg.size() > 2) { argpos = 2; }
					}
	
					// unknown option
					if(o == NULL)
					{
						if(ignore_unknown_opts) { ++i; continue; }
	
						THROW(EOptions, std::string("Unrecognized option `") + arg + "'");
					}
	
					// parameter not required but supplied
					if(argpos && o->parameter == Option::None)
					{
						THROW(EOptions, string("Command line option `") + pfxopt + "' does not take an argument, but one was suplied.");
					}
				
					// deduce option value
					std::string value = argpos ? arg.substr(argpos) : o->optval;
	
					// parameter required but not supplied with option
					if(!argpos && o->parameter == Option::Required)
					{
						// see if the next cmdline parameter is a parameter
						++i;
						if(i != args.end() && strlen(*i) && (*i)[0] != '-')
						{
							value = *i;
						}
						else
						{
							THROW(EOptions, string("Argument to `") + arg + "' is missing");
						}
					}
	
					// store
					store(*o, value, opt, false);
					succ = true;
				}
			}
		}
		else
		{
			// argument handling
			succ = handle_argument(arg, o);
		}

		// verify arg/opt handling success
		i++;
		if(succ)
		{
			options_found[o->mapkey] = true;
			args.erase(k, i);
		}
	}

	// verify that we have a sufficient number of command line arguments read
	if(nargs < nreqargs)
	{
		THROW(EOptions, string("Insufficient number of command line arguments (") + str(nargs) + " found, " + str(nreqargs) + " required)");
	}
}

std::string argapp(const Option &o, const std::string &sep = "=")
{
	switch(o.parameter)
	{
		case Option::Optional: return std::string("[") + sep + "param]";
		case Option::Required: return sep + "<param>";
	}
	return "";
}

    namespace parformat {

    	int split(std::list<std::string> &chunks, const std::string &text, const std::string &sep = "\n")
	{
		chunks.clear();
		size_t pos0 = 0, pos;
		do
		{
			pos = text.find(sep, pos0);
			size_t len = (pos == string::npos ? text.size() : pos) - pos0;
			chunks.push_back(text.substr(pos0, len));
			pos0 = pos + 1;
		} while(pos != string::npos);
		return chunks.size();
	}

        void format_paragraph(std::ostream& os,
                              std::string par,
                              unsigned first_column_width,
                              unsigned line_length,
			      std::list<string> *leftpars = NULL)
        {
            // index of tab (if present) is used as additional indent relative
            // to first_column_width if paragrapth is spanned over multiple
            // lines if tab is not on first line it is ignored
            string::size_type par_indent = par.find('\t');
	    using namespace std;

            if (par_indent == string::npos)
            {
                par_indent = 0;
            }
            else
            {
                // only one tab per paragraph allowed
                if (count(par.begin(), par.end(), '\t') > 1)
                {
                    ASSERT(0) { std::cerr << "Only one tab per paragraph is allowed"; };
                }
          
                // erase tab from string
                par.erase(par_indent, 1);

                // this assert may fail due to user error or 
                // environment conditions!
                assert(par_indent < (line_length - first_column_width));

                // ignore tab if not on first line
                if (par_indent >= (line_length - first_column_width))
                {
                    par_indent = 0;
                }            
            }
          
            if (par.size() < (line_length - first_column_width))
            {
                os << par;
            }
            else
            {
                string::const_iterator       line_begin = par.begin();
                const string::const_iterator par_end = par.end();

                bool first_line = true; // of current paragraph!
            
                unsigned indent = first_column_width;
            
                while (line_begin < par_end)  // paragraph lines
                {
                    if (!first_line)
                    {
                        // trimm leading single spaces
                        // if (firstchar == ' ') &&
                        //    ((exists(firstchar + 1) && (firstchar + 1 != ' '))
                        if ((*line_begin == ' ') &&
                            ((line_begin + 1 < par_end) &&
                             (*(line_begin + 1) != ' ')))
                        {
                            line_begin += 1;  // line_begin != line_end
                        }
                    }

                    string::const_iterator line_end;
                
                    if (line_begin + (line_length - indent) > par_end)
                    {
                        line_end = par_end;
                    }
                    else
                    {
                        line_end = line_begin + (line_length - indent);
                    }
            
                    // prevent chopped words
                    // if (lastchar != ' ') &&
                    //    ((exists(lastchar + 1) && (lastchar + 1 != ' '))
                    if ((*(line_end - 1) != ' ') &&
                        ((line_end < par_end) && (*line_end != ' ')))
                    {
                        // find last ' ' in the second half of the current paragraph line
                        string::const_iterator last_space =
                            find(reverse_iterator<string::const_iterator>(line_end - 1),
                                 reverse_iterator<string::const_iterator>(line_begin - 1),
                                 ' ')
                            .base();
                
                        if (last_space != line_begin - 1)
                        {                 
                            // is last_space within the second half ot the 
                            // current line
                            if (unsigned(distance(last_space, line_end)) < 
                                (line_length - indent) / 2)
                            {
                                line_end = last_space;
                            }
                        }                                                
                    } // prevent chopped words
             
                    // write line to stream
                    copy(line_begin, line_end, ostream_iterator<char>(os));
              
                    if (first_line)
                    {
                        indent = first_column_width + par_indent;
                        first_line = false;
                    }

                    // more lines to follow?
                    if (line_end != par_end)
                    {
                        os << '\n';

			unsigned pad = indent;
			if(leftpars && !leftpars->empty())
			{
				os << leftpars->front();
				pad -= leftpars->front().size();
				leftpars->erase(leftpars->begin());
			}
			for(; pad > 0; --pad)
			{
				os.put(' ');
			}                    
                    }
              
                    // next line starts after of this line
                    line_begin = line_end;              
                } // paragraph lines
            }          
        }                              
        
        void format_description(std::ostream& os,
                                const std::string& desc, 
                                unsigned first_column_width,
                                unsigned line_length,
				const std::string& lefttext)
        {
            // we need to use one char less per line to work correctly if actual
            // console has longer lines
            assert(line_length > 1);
            if (line_length > 1)
            {
                --line_length;
            }

            // line_length must be larger than first_column_width
            // this assert may fail due to user error or environment conditions!
            assert(line_length > first_column_width);

	    list<string> paragraphs, leftpars;
	    split(paragraphs, desc);
	    bool indent = true;
	    if(lefttext.size())
	    {
		split(leftpars, lefttext);
		indent = false;
	    }
            list<string>::iterator par_iter = paragraphs.begin();                
            list<string>::iterator par_end = paragraphs.end();
            while (par_iter != par_end)  // paragraphs
            {
                // print the first line of left column
		unsigned pad = first_column_width;
		if(!leftpars.empty())
		{
			os << leftpars.front();
			pad -= leftpars.front().size();
			leftpars.erase(leftpars.begin());
		}
		if(!indent)
		{
			for(; pad > 0; --pad)
			{
				os.put(' ');
			}
		}
		indent = false;

		// rest of the paragraph
                format_paragraph(os, *par_iter, first_column_width, 
                                 line_length, &leftpars);
                ++par_iter;

		// next line
                if (par_iter != par_end)
                {
                    os << "\n";
		}
            }  // paragraphs

	    // write remaining left column text
	    FOREACH(leftpars)
	    {
	    	os << "\n" << *i;
	    }
        }
    
/*        void format_one(std::ostream& os, const option_description& opt, 
                        unsigned first_column_width, unsigned line_length)
        {
            stringstream ss;
            ss << "  " << opt.format_name() << ' ' << opt.format_parameter();
            
            // Don't use ss.rdbuf() since g++ 2.96 is buggy on it.
            os << ss.str();

            if (!opt.description().empty())
            {
                for(unsigned pad = first_column_width - ss.str().size(); 
                    pad > 0; 
                    --pad)
                {
                    os.put(' ');
                }
            
                format_description(os, opt.description(),
                                   first_column_width, line_length);
            }*/
        }

std::ostream &Options::summary(std::ostream &out)
{
	out << "Usage: " << program << " ";
	if(!args.empty())
	{
		FOREACH(args)
		{
			Option &o = *i;

			if(i != args.begin())              { out << " "; }

			if(o.parameter == Option::Required) { out << "<" + o.name[0] << ">"; }
			else                               { out << "[" + o.name[0] << "]"; }
		}
	}

	return out;
}

std::ostream &Options::usage(std::ostream &out)
{
	return summary(out) << "\n\nTry `" << program << " --help' for more options.";
}

std::ostream &Options::version(std::ostream &out)
{
	out << "Program   : " << program << "\n";
	if(!description.empty()) { out << "Summary   : " << description << "\n"; }
	out << std::string(ver) << "\n";
	out << std::string(author) << "\n";
	return out;
}

std::ostream &Options::help(std::ostream &out)
{
	out << "\n";
	
	if(!description.empty())
	{
		out << description << "\n";
	}

	summary(out) << "\n\n";

	if(!args.empty())
	{
		out << "Arguments:\n";
		size_t maxlen = 0; vector<std::string> fmtted;
		FOREACH(args)
		{
			Option &o = *i;
			std::string l("   "); l += o.name[0];
			maxlen = std::max(maxlen, l.size());
			fmtted.push_back(l);
		}
		maxlen += 3;
		FORj(j, 0, fmtted.size())
		{
			std::string &l = fmtted[j];
			out << l;
			if(l.size() < maxlen) { out << string(maxlen - l.size(), ' '); }

			ostringstream desc;
			desc << args[j].description;

			bool opened = false;
			if(args[j].defval.size())
			{
				if(!opened) { desc << " ("; opened = true; } else { desc << ", "; }
				desc << "'" << args[j].defval << "' if unspecified";
			}
			if(opened) { desc << ")"; }

			parformat::format_description(out, desc.str(), maxlen, 78, "");
			out << "\n";
		}
	}
	out << "\n";

	if(options.size())
	{
		out << "Options:\n";

		// figure out the maximum width of left column
		size_t maxlen = 0; vector<pair<std::string, int> > fmtted;
		FOREACH(options)
		{
			Option &o = *i;

			size_t lastlen;
			ostringstream sfmt;
			bool prevshort = false;
			FORj(j, 0, o.name.size())
			{
				std::string &name = o.name[j];

				ostringstream line;
				line << (prevshort ? " " : "   ");
				line << (name.size() == 1 ? "-" : "--") << name;
				line << argapp(o, (name.size() == 1 ? "" : "="));
				if(j+1 != o.name.size()) { line << ","; }

				std::string l = line.str();
				lastlen = (prevshort ? lastlen : 0) + l.size();
				maxlen = std::max(maxlen, lastlen);

				if(j && !prevshort) { sfmt << "\n"; }
				sfmt << l;

				prevshort = name.size() == 1 && o.parameter == Option::None;
			}

			fmtted.push_back(make_pair(sfmt.str(), lastlen));
		}

		maxlen += 3;
		FORj(j, 0, fmtted.size())
		{
			pair<std::string, int> &l = fmtted[j];
			Option &o = options[j];

			// left column
			//out << l.first;
			//if(l.second < maxlen) { out << string(maxlen - l.second, ' '); }

			// right column
			ostringstream desc;
			desc << o.description;
			bool opened = false;
			if(o.defval.size())
			{
				if(!opened) { desc << " ("; opened = true; } else { desc << ", "; }
				desc << "'" << o.defval << "' if unspecified";
			}
			if(o.parameter == Option::Optional)
			{
				if(!opened) { desc << " ("; opened = true; } else { desc << ", "; }
				desc << "'" << o.optval << "' if param not present";
			}
			if(opened) { desc << ")"; }
			parformat::format_description(out, desc.str(), maxlen, 78, l.first);
			out << "\n";
		}
	}

	out << "\n";
	
	if(prolog.size())
	{
		out << prolog << "\n\n";
	}

	return out;
}

namespace peyton {
namespace system {
	void print_options_error(const std::string &err, Options &opts, std::ostream &out)
	{
		out << ">>>  " << err << "\n";
		opts.usage(out) << "\n";
	}
	
	void parse_options(Options::option_list &optlist, Options &opts, int argc, char **argv, ostream &out)
	{
		try {
			opts.parse(argc, argv, &optlist);
		}
		catch(EOptionsHelp &e) {
			opts.help(out);
			exit(0);
		}
		catch(EOptionsVersion &e)
		{
			opts.version(out);
			exit(0);
		}
		catch(EOptions &e)
		{
			print_options_error(e.info, opts, out);
			exit(-1);
		}
	}
	
	void parse_options(Options &opts, Options::option_list &optlist, ostream &out)
	{
		try {
			opts.parse(optlist);
		}
		catch(EOptionsHelp &e) {
			opts.help(out);
			exit(0);
		}
		catch(EOptionsVersion &e)
		{
			opts.version(out);
			exit(0);
		}
		catch(EOptions &e)
		{
			print_options_error(e.info, opts, out);
			exit(-1);
		}
	}
	
	void parse_options(Options &opts, int argc, char **argv, ostream &out)
	{
		Options::option_list optlist;
		return parse_options(optlist, opts, argc, argv, out);
	}
}
}

#define PRINT(v) \
	std::cout << #v" = " << v << " (found = " << sopts[cmd]->found(#v) << ") " << " (*sopts[cmd].count = " << sopts[cmd]->count(#v) << ") ((*sopts[cmd])[" #v "]) = " << (*sopts[cmd])[#v] << "\n";

int test_options(int argc, char **argv)
{
try {
	std::string argv0 = argv[0];
	VERSION_DATETIME(version, "$Id: Options.cpp,v 1.8 2006/07/13 23:26:55 mjuric Exp $");
	std::string progdesc = "libpeytondemo, a mock star catalog generator.";

	//
	// Declare option variables here
	//
	std::string cmd, conf, output = "xx.txt";
	int wparam = 3;
	bool d = true;
	std::string strval = "nostring";

	//
	// Option definitions
	//
	std::map<std::string, Options *> sopts;
	Options opts(argv[0], progdesc, version, Authorship::majuric);
	opts.argument("cmd").bind(cmd).desc(
		"What to make. Can be one of:\n"
		"  footprint - \tcalculate footprint of a set of runs on the sky\n"
		"    pskymap - \tconstruct a partitioned sky map given a set of runs on the sky\n"
		"       beam - \tcalculate footprint of a single conical beam\n"
		"        pdf - \tcalculate cumulative probability density functions (CPDF) for a given model and footprint\n"
		"    catalog - \tcreate a mock catalog given a set of CPDFs\n"
		);
	opts.stop_after_final_arg = true;
	opts.prolog = "For detailed help on a particular subcommand, do `libpeytondemo <cmd> -h'";
	opts.add_standard_options();

	sopts["footprint"] = new Options(argv0 + " footprint", progdesc + " Footprint generation subcommand.", version, Authorship::majuric);
	sopts["footprint"]->argument("conf").bind(conf).desc("Configuration file for the Bahcall-Soneira model, or if cmd=footprint, the file with the set of runs for which to calculate footprint.");
	sopts["footprint"]->argument("output").bind(output).optional().desc("Name of the output file (needed for cmd='pdf')");
	sopts["footprint"]->option("p")
		.addname("xparam").addname("wparam")
		.bind(wparam)
		.param_required()
		.desc("An integer option requiring a parameter. This is intentionaly longer than it should be.\nAnd here is a new paragraph now. Blabla new paragraph that is long.");
	sopts["footprint"]->option("d").addname("ddlong").addname("ddlong2").bind(d).value("false").desc("A boolean switch");
	sopts["footprint"]->option("strval").bind(strval).param_required().desc("A string with optional value");
	sopts["footprint"]->add_standard_options();

	//
	// Parse
	//
	Options::option_list optlist;
	parse_options(optlist, opts, argc, argv);
	if(sopts.count(cmd))
	{
		parse_options(*sopts[cmd], optlist);
	}
	else
	{
		ostringstream ss;
		ss << "Unrecognized subcommand `" << cmd << "'";
		print_options_error(ss.str(), opts);
		return -1;
	}

	//
	// Your code starts here
	//
	PRINT(cmd);
	PRINT(conf);
	PRINT(output);
	PRINT(wparam);
	PRINT(d);
	PRINT(strval);

} catch(EAny &e) {
	e.print();
	return -1;
}
}

#undef PRINT
#define PRINT(v) \
	std::cout << #v" = " << v << " (found = " << opts.found(#v) << ") " << " (opts.count = " << opts.count(#v) << ") ((opts)[" #v "]) = " << (opts)[#v] << "\n";
int test_options_simple(int argc, char **argv)
{
try {
	//
	// Program version information
	//
	VERSION_DATETIME(version, "$Id: Options.cpp,v 1.8 2006/07/13 23:26:55 mjuric Exp $");
	std::string progdesc = "libpeytondemo, a mock star catalog generator.";

	//
	// Declare option variables here
	//
	std::string cmd, conf, output = "xx.txt";
	int wparam = 3;
	bool d = true;
	std::string strval = "nostring";

	//
	// Define options
	//
	Options opts(argv[0], progdesc, version, Authorship::majuric);
	opts.argument("cmd").bind(cmd).desc(
		"What to make. Can be one of:\n"
		"  footprint - \tcalculate footprint of a set of runs on the sky\n"
		"    pskymap - \tconstruct a partitioned sky map given a set of runs on the sky\n"
		"       beam - \tcalculate footprint of a single conical beam\n"
		"        pdf - \tcalculate cumulative probability density functions (CPDF) for a given model and footprint\n"
		"    catalog - \tcreate a mock catalog given a set of CPDFs\n"
		);
	opts.add_standard_options();
	opts.argument("conf").bind(conf).desc("Configuration file for the Bahcall-Soneira model, or if cmd=footprint, the file with the set of runs for which to calculate footprint.");
	opts.argument("output").bind(output).optional().desc("Name of the output file (needed for cmd='pdf')");
	opts.option("wparam").bind(wparam).param_required().desc("An integer option requiring a parameter. This is intentionaly longer than it should be.\nAnd here is a new paragraph now. Blabla new paragraph that is long.");
	opts.option("d").addname("ddlong").addname("ddlong2").bind(d).value("false").desc("A boolean switch");
	opts.option("strval").bind(strval).param_required().desc("A string with optional value");

	//
	// Parse
	//
	parse_options(opts, argc, argv);

	//
	// Your code starts here
	//
	PRINT(cmd);
	PRINT(conf);
	PRINT(output);
	PRINT(wparam);
	PRINT(d);
	PRINT(strval);

} catch(EAny &e) {
	e.print();
	return -1;
}
}
