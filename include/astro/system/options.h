#ifndef __astro_system_options
#define __astro_system_options

#include <astro/system/config.h>
#include <astro/util.h>
#include <astro/exceptions.h>

#include <vector>
#include <list>
#include <string>
#include <iostream>

#include <auto_ptr.h>

namespace peyton {

namespace exceptions {
	/// Exception thrown by %Options class
	SIMPLE_EXCEPTION(EOptions);
	DERIVE_EXCEPTION(EOptionsHelp, EOptions);
	DERIVE_EXCEPTION(EOptionsVersion, EOptions);
}

namespace system {

/**
	\brief	Program description and versioning classes
*/

struct Authorship
{
	std::string		authors;		///< Name of the program author(s)

	std::string		contact;		///< Who should be contacted for questions about the code
	std::string		contact_mail;	///< E-mail of the contact for the program
	
	Authorship(const std::string &authors_, const std::string &contact_ = "", const std::string &contact_mail_ = "")
		: authors(authors_), contact(contact_), contact_mail(contact_mail_)
	{}

	operator std::string()
	{
		return std::string("Author(s) : ") + authors + "\n"
			           + "Contact   : "  + contact + " <" + contact_mail + ">";
	}

	// predefined authors (send me a bottle of Coke (regular, please) and get your name listed)
	static Authorship majuric;
	static Authorship unspecified;
};

struct Version
{
	std::string		version;

	Version(const std::string &version_) : version(version_) {}

	operator std::string()
	{
		return std::string("Version   : ") + version;
	}
	
	static Version unspecified;
};
#define VERSION_DATETIME(ver, id) Version ver(std::string(id) + "\n            Built on " + std::string(__DATE__) + " " + __TIME__)

class Option;

namespace opt
{
	struct any
	{
	public:
		struct binding_data
		{
			virtual std::istream &setval(std::istream &in) = 0;
			virtual std::ostream &getval(std::ostream &out) const = 0;

			virtual binding_data *clone() const = 0;
			virtual std::string type() const = 0;

			virtual ~binding_data() {};
		};

	protected:
		std::auto_ptr<binding_data> data;

	public:
		// binder constructors
		template<typename T> any(T &t);
		template<typename T> void reset(T &t);

	public:
		// accessors
		std::string type() { return data.get() != NULL ? data->type() : ""; }
		operator bool() { return data.get() != NULL; }
		std::istream &setval(std::istream &in) { return data->setval(in); }
		std::ostream &getval(std::ostream &out) { return data->getval(out); }

	public:
		// default constructor
		any() {}

		// copy constructor
		any(const any &x)
			: data(x.data.get() ? x.data->clone() : NULL)
		{}
		any &operator=(const any &x)
		{
			data.reset(x.data.get() ? x.data->clone() : NULL);
			return *this;
		}
	};

	template<typename T>
	struct binding : public any::binding_data
	{
		T &var;
		bool isset;
		binding(T &v) : var(v), isset(false) { }

		virtual std::string type() const			{ return type_name(var); }
		virtual std::istream &setval(std::istream &in)		{ isset = in >> var; return in; }
		virtual std::ostream &getval(std::ostream &out) const	{ return out << var; }
		virtual binding *clone() const				{ return new binding<T>(*this); }

		virtual ~binding() {};
	};

	// specialize for integral types, so they may be given in exponential notation
	inline double readval(std::istream &in, bool &isset)
	{
		double tmp;
		isset = in >> tmp;
		return tmp;
	}
	template<> inline std::istream &binding<int>::setval(std::istream &in)			{ var = readval(in, isset); return in; }
	template<> inline std::istream &binding<unsigned int>::setval(std::istream &in)		{ var = readval(in, isset); return in; }
	template<> inline std::istream &binding<long>::setval(std::istream &in)			{ var = readval(in, isset); return in; }
	template<> inline std::istream &binding<unsigned long>::setval(std::istream &in)	{ var = readval(in, isset); return in; }
	template<> inline std::istream &binding<long long>::setval(std::istream &in)		{ var = readval(in, isset); return in; }
	template<> inline std::istream &binding<unsigned long long>::setval(std::istream &in)	{ var = readval(in, isset); return in; }

	template<typename T>
		void any::reset(T &t)
		{
			data.reset(new binding<T>(t));
		}

	template<typename T>
		any::any(T &t)
			: data(new binding<T>(t))
		{}

	// specialize for bool
	template<>
	inline std::istream &binding<bool>::setval(std::istream &in)
	{
		std::string tmp;
		if(!(in >> tmp)) { return in; }

		isset = true;
		if(tmp == "false" || tmp == "0") { var = 0; return in; }
		if(tmp == "true" || tmp == "1") { var = 1; return in; }

		isset = false;
		in.setstate(std::ios::failbit);
		return in;
	}
	template<>
	inline std::ostream &binding<bool>::getval(std::ostream &out) const
	{
		return out << (var ? "true" : "false");
	}

	// specialize for string - ignore spaces
	template<>
	inline std::istream &binding<std::string>::setval(std::istream &in)
	{
		var.clear();
		while(in.good())
		{
			char buf[51] = {0};
			in.read(buf, 50);
			var += buf;
		}
		if(in.eof())
		{
			// clear EOF bit so that the result is a success
			in.clear();
		}
		return in;
	}

	// specialize input/output for vectors
	template<typename T>
	struct binding<std::vector<T> > : public any::binding_data
	{
		std::vector<T> &var;
		bool isset;
		binding(std::vector<T> &v) : var(v), isset(false) { }

		virtual std::string type() const			{ return type_name(var); }
		virtual std::istream &setval(std::istream &in)
		{
			T val;
			binding<T> tmp(val);
			if(!tmp.setval(in)) { return in; }
	
			if(!isset) { this->var.clear(); }
			this->var.push_back(val);
			this->isset = true;
	
			return in;
		}
		virtual std::ostream &getval(std::ostream &out) const
		{
			T val;
			binding<T> tmp(val);

			FOREACH(var)
			{
				if(i != var.begin()) { out << " "; }
				val = *i;
				tmp.getval(out);
			}
	
			return out;
		}
		virtual binding *clone() const
		{
			return new binding<std::vector<T> >(*this);
		}

		virtual ~binding() {};
	};

};

/**
	\brief	Command line option specification class
	
	Used to describe permissable command line options. This class should not be used/constructed directly.
	Use peyton::system::Options::option() and peyton::system::Options::argument() instead.
*/
struct Option
{
public:
	/// Parameter types for peyton::system::Option
	enum Parameter
	{
		None = 0,		///< Option does not have a parameter
		Required = 1,		///< Option must come with a parameter (when used arguments: argument is required)
		Optional = 2		///< Option may come with a parameter (when used arguments: argument is optional)
	};

	opt::any				variable;	///< an option can be bound to a variable (preferred)
	std::string				mapkey;		///< key to set in the map for this option (deprecated)

	std::vector<std::string>		name;		///< option name
	std::string				optval;		///< value to be returned if the option was given on the command line but
								///< either parameter=no or parameter=optional and no parameter was specified
	bool					optvalset;	///< has the option value been set
	Parameter parameter;					///< does the option take a parameter?
	bool					dogobble;	///< do we absorb all arguments after this one (valid only for last cmdline argument)
	std::string				description;	///< description of this option

	std::string				defval;		///< if it was not specified on the command line, set the hash map to this value.
	bool					defvalset;
	bool					deffromvar;

protected:
	friend class Options;
	bool notify(const std::string &s);			///< set the value of a bound variable, if any. Called from Options::parse. return false if parsing unsuccessful.

public:
	Option() : parameter(None), defvalset(false), optvalset(false), deffromvar(false), dogobble(false) {}

	Option(const Option &o);
	Option& operator=(const Option &o);

	//
	// property setters
	//
	Option &addname(const std::string &val) { name.push_back(val); return *this; }
	Option &key(const std::string &val) { mapkey = val; return *this; }
	Option &param_required()	{ parameter = Option::Required; return *this; }
	Option &param_optional()	{ parameter = Option::Optional; return *this; }
	Option &param_none()	{ parameter = Option::None; return *this; }
	Option &required()	{ parameter = Option::Required; return *this; }
	Option &optional()	{ parameter = Option::Optional; return *this; }
	Option &gobble()	{ dogobble = true; return *this; }
	Option &value(const std::string &val)
	{
		if(optvalset)
		{
			using namespace peyton::exceptions;
			THROW(EOptions, std::string("PROGRAM ERROR (please notify the author): Value already set for option ") + name[0]);
		}
		optval = val;
		optvalset = true;
		return *this;
	}
	Option &desc(const std::string &val)      { description = val; return *this; }

	Option &def_val(const std::string &val)
	{
		if(defvalset)
		{
			using namespace peyton::exceptions;
			THROW(EOptions, std::string("PROGRAM ERROR (please notify the author): Default value already set for option ") + name[0]);
		}
		defval = val;
		defvalset = true;
		return *this;
	}

	template<typename T>
		Option &bind(T &var, bool sdef = true)
		{
			this->variable.reset(var);
			//std::cerr << "Setting " << name[0] << " "; this->variable.getval(std::cerr) << " (" << this->variable.type() << ")\n";
			if(sdef)
			{
				std::ostringstream ss;
				this->variable.getval(ss);
				def_val(ss.str());
				deffromvar = true;
			}
			return *this;
		}
};

/**
	\brief	Command line options parsing
	
	Use this class, in conjunction with peyton::system::Option class to parse command line options.

	Options class is a subclass of peyton::system::Config (that is, it's a string to string key-value map). Command
	line options specify what value to assign to a given key. You specify the mapping by calling Options::add() for
	each command line option you need. After you've specified all valid command line options, parse the command
	line by calling the Options::parse() method. Here's a typical code snippet demonstrating how to use the Options
	class.
	
	\sa Options::add()
\code
int main(int argc, char **argv)
{
	Options options;
	//           key      longname   short value  argument          default              description
	options.add("remote", "noremote",  0,  "0",   Option::none,     Option::nodefault,   "Do not start remote servers");
	options.add("remote", "remote",   'r', "1",   Option::none,     Option::nodefault,   "Start remote servers (default)");
	options.add("test",   "test",     't', "1",   Option::none,     "0",                 "Just do testing, no actual running");
	options.add("usage",  "usage",    'u', "all", Option::optional, "",                  "Do not start remote servers");

	try
	{
		options.parse(argc, argv);
	}
	catch(EOptions &e)
	{
		cerr << "\n" << options.usage(argv);
		e.print();

		exit(-1);
	}
	
	... your code ...
}
\endcode
*/
class Options : public peyton::system::Config
{
protected:
	std::map<std::string, bool> options_found; ///< Options which appeared on the command line will have optionsGiven entry, mapped by \c Option::key
	std::map<std::string, Option*> longOptMap;
	std::map<char, Option*> shortOptMap;

	bool ignore_unknown_opts;		///< ignore unknown options and extra arguments
	int nargs;				///< number of arguments found (filled in in parse())
	int nreqargs;				///< number of required arguments (filled in in parse())
	std::vector<Option> args, options;
	std::string program;

	std::string description;		///< Short program description (will appear just above the generated usage())
	Version ver;
	Authorship author;

	bool defoptsadded;			///< Default options have been added
public:
	bool stop_after_final_arg;		///< do not parse anything beyond the final positional argument
	std::string prolog;			///< text which will appear on the bottom of help() text

	typedef std::list<const char *> option_list;

	Options(
		const std::string &argv0,
		const std::string &description_ = "",
		const Version &version_ = Version::unspecified,
		const Authorship &author_ = Authorship::unspecified
		);

	/// describe how to parse a command line option, using opt classes
	/// by default, does not require a parameter
	Option &option(const std::string &name, const std::string &desc = "")
	{
		options.push_back(Option());
		return options.back()
			.key(name)
			.addname(name)
			.desc(desc)
			.param_none();
	}

	/// add a command line argument
	/// by default, it's a required argument
	Option &argument(const std::string &key, const std::string &desc = "")
	{
		args.push_back(Option());
		return args.back()
			.key(key)
			.addname(key)
			.desc(desc)
			.param_required();
	}

	/// Ignore unknown options and extra command line arguments
	void ignore_unknown(bool ignore) { ignore_unknown_opts = ignore; }

	/// Parses the command line and loads the options, as specified by \a options argument
	void parse(int argc, char **argv, option_list *remainder = NULL);
	void parse(option_list &options);

	/// Returns \c true if the option \a o was present on the command line
	bool found(const std::string &o) { return options_found.count(o) != 0; }

	/// Returns the number of command line arguments that were found	
	int arguments_found() { return nargs; }

	/// Returns a human readable string describing the way to use the program, and the options avaliable
	std::ostream &help(std::ostream &out);

	/// Returns a short human readable summary describing the program, and instructions on how to get more help
	std::ostream &usage(std::ostream &out);
	
	/// Returns a short human readable version string and author summary
	std::ostream &version(std::ostream &out);

	void add_standard_options();
protected:
	bool handle_argument(const std::string &arg, Option *&o);
	void store(Option &o, const std::string &value, const std::string &opt, bool is_argument);
	std::ostream &summary(std::ostream &out);
};

/// parse options opt from (argc, argv), writing any errors to out
void parse_options(Options &opts, int argc, char **argv, std::ostream &out = std::cerr);
/// parse options opt from (argc, argv), writing any errors to out, returning possibly unparsed options in optlist (if that has been enabled through opts.ignore_unknown())
void parse_options(Options::option_list &optlist, Options &opts, int argc, char **argv, std::ostream &out = std::cerr);
/// parse options opt from optlist, writing any errors to out. Successfully parsed options are removed from optlist
void parse_options(Options &opts, Options::option_list &optlist, std::ostream &out = std::cerr);

/// prints an option-parsing induced error in a standardized format
void print_options_error(const std::string &err, Options &opts, std::ostream &out = std::cerr);

} // namespace system
} // namespace peyton

#define __peyton_system peyton::system

#endif
