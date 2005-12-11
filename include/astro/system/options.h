#ifndef __astro_system_options
#define __astro_system_options

#include <astro/system/config.h>
#include <astro/exceptions.h>

#include <vector>
#include <string>

namespace peyton {

namespace exceptions {
	/// Exception thrown by %Options class
	SIMPLE_EXCEPTION(EOptions);
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
#define VERSION_DATETIME(ver) Version ver(std::string(__DATE__) + " " + __TIME__)

class Option;
namespace opt
{
	struct base
	{
		std::string val;
		int priority;

		base(const std::string &s, int pri) : val(s), priority(pri) {}
		virtual void apply(Option &o) const = 0;
	};

	struct shortname : public base
	{
		// implies name = shortname (=> before name)
		// implies key = shortname (=> before key)
		// implies value = shortname (=> before value,name)
		shortname(const char c) : base(c == 0 ? std::string() : std::string(1, c), 10) {}
		virtual void apply(Option &o) const;
	};

	struct name : public base
	{
		// implies key = name (=> before key)
		// implies value = name (=> before value)
		name(const std::string &s) : base(s, 20) {}
		virtual void apply(Option &o) const;
	};

	struct binding : public base
	{
		union {
			double *v_double;
			float *v_float;
			int *v_int;
			short *v_short;
			bool *v_bool;
			std::string *v_string;
		} var;
		enum { unbound, t_double, t_float, t_int, t_short, t_bool, t_string } type;
		bool showdefault;
		#define CS(ty) binding(ty &v, bool sdef = true) : base("", 25), type(t_##ty), showdefault(sdef) { var.v_##ty = &v; }
		#define CSN(ns, ty) binding(ns::ty &v, bool sdef = true) : base("", 25), type(t_##ty), showdefault(sdef) { var.v_##ty = &v; }
		CS(double) CS(float) CS(short) CS(bool) CSN(std, string)

		binding() : type(unbound), base("", 25), showdefault(false) {}
		binding(const binding &b) : base("", 25), var(b.var), type(b.type), showdefault(b.showdefault) {}

		bool bound() { return type != unbound; }
		void clear() { type = unbound; }
		bool hasdefaultvalue() { return showdefault; }

		virtual void apply(Option &o) const;
		void setvalue(const std::string &s);
		std::string to_string() const;
	};

	struct key : public base
	{
		key(const std::string &s) : base(s, 30) {}
		virtual void apply(Option &o) const;
	};

	struct value : public base
	{
		value(const std::string &s) : base(s, 30) {}
		virtual void apply(Option &o) const;
	};

	struct def_value : public base
	{
		def_value(const std::string &s) : base(s, 30) {}
		virtual void apply(Option &o) const;
	};

	struct desc : public base
	{
		desc(const std::string &s) : base(s, 30) {}
		virtual void apply(Option &o) const;
	};

	struct arg : public base
	{
		arg(const std::string &s);
		virtual void apply(Option &o) const;
	};

	struct empty_tag : public base
	{
		empty_tag() : base("", 100) {}
		virtual void apply(Option &o) const;
	};

	extern arg arg_required;
	extern arg arg_optional;
	extern arg arg_none;
	extern empty_tag empty;
	static const bool hasdefault = true;

	#define USE_CMDLINE_OPTIONS using namespace peyton::system::opt;
};

/**
	\brief	Command line option specification class
	
	Used to describe permissable command line options. This class should not be used/constructed directly.
	Use peyton::system::Options::option() and peyton::system::Options::argument() instead.
*/
struct Option
{
public:
	/// Argument types for peyton::system::Option
	enum Argument
	{
		none = 0,		///< Option does not have a parameter
		required = 1,	///< Option must come with a parameter
		optional = 2	///< Option may come with a parameter
	};

	opt::binding				variable;	///< an option can be bound to a variable (preferred)
	std::string				key;		///< key to set in the map for this option (deprecated)

	std::string				name;		///< long option name
	char					shortname;	///< short option name
	std::string				value;	///< value to be returned if the option was given on the command line but
								///< either argument=no or argument=optional and no argument was specified
	Argument argument;				///< does the option have an argument

	std::string				defaultvalue;///< if it was not specified on the command line
	std::string				description;///< description of this option

	static std::string		nodefault;	///< pass this string as defaultvalue to have no default value

protected:
	friend class Options;
	void setvalue(const std::string &s)		///< set the value of a bound variable, if any. Called from Options::parse
		{ variable.setvalue(s); }
public:
	bool hasdefaultvalue;				///< does this option have a default value

public:
	Option(const Option &o)
		: variable(o.variable), key(o.key),
		name(o.name), shortname(o.shortname), value(o.value),
		argument(o.argument),
		defaultvalue(o.defaultvalue), description(o.description),
		hasdefaultvalue(o.hasdefaultvalue)
	{
	}

	Option(const std::string &key_, 
		
		const std::string &name_, const char shortname_ = 0,
		const std::string &value_ = "1",
		
		const Argument argument_ = none,
		
		const std::string &defaultvalue_ = nodefault, const std::string &description_ = "")
	: key(key_), name(name_), shortname(shortname_), value(value_), argument(argument_), defaultvalue(defaultvalue_), description(description_)
	{
		hasdefaultvalue = &defaultvalue_ != &nodefault;
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
	int nargs;
	std::map<std::string, bool> optionsGiven; ///< Options which appeared on the command line will have optionsGiven entry, mapped by \c Option::key
	std::vector<Option> options, args;

	std::string description;			///< Whole program description (will appear just above the generated usage())
	Version version;
	Authorship author;
public:
	Options(
		const std::string &description_ = "",
		const Version &version_ = Version::unspecified,
		const Authorship &author_ = Authorship::unspecified
		);

	/// describe how to parse a command line option
	void option(
		const std::string &key_, 					///< key which this option modifies
		const std::string &name_,					///< long command line name of this option (eg, if your option is --user, name would be "user")
		const char shortname_ = 0,					///< short command line name of this option (eg. if your option is -u, shortname would be 'u')
		const std::string &value_ = "1",				///< the value to assign to (*this)[key] if the option is specified on the command line, without arguments
		const Option::Argument argument_ = Option::none,	///< should the option accept arguments (possibilities are none, required and optional)
		const std::string &defaultvalue_ = Option::nodefault,	///< the value to assign to (*this)[key] if the option is \b not specified on the command line
		const std::string &description_ = ""			///< description of this option (used to construct the Options::usage() string
		)
		{
			options.push_back(Option(key_, name_, shortname_, value_, argument_, defaultvalue_, description_));
		}

	/// describe how to parse a command line option, using opt classes
	void option(
		const opt::base &spec1 = opt::empty,
		const opt::base &spec2 = opt::empty,
		const opt::base &spec3 = opt::empty,
		const opt::base &spec4 = opt::empty,
		const opt::base &spec5 = opt::empty,
		const opt::base &spec6 = opt::empty,
		const opt::base &spec7 = opt::empty,
		const opt::base &spec8 = opt::empty,
		const opt::base &spec9 = opt::empty
		);

	/// describe how to parse a command line option, using opt classes
	/// by default, requires an argument
	void option(
		const std::string &name,
		const opt::base &spec2 = opt::empty,
		const opt::base &spec3 = opt::empty,
		const opt::base &spec4 = opt::empty,
		const opt::base &spec5 = opt::empty,
		const opt::base &spec6 = opt::empty,
		const opt::base &spec7 = opt::empty
		)
		{ option(opt::name(name), opt::arg_required, spec2, spec3, spec4, spec5, spec6, spec7); }
	
	/// describe how to parse a command line option, using opt classes
	/// by default, _does not_ require an argument
	void option(
		const char shortname,
		const opt::base &spec2 = opt::empty,
		const opt::base &spec3 = opt::empty,
		const opt::base &spec4 = opt::empty,
		const opt::base &spec5 = opt::empty,
		const opt::base &spec6 = opt::empty,
		const opt::base &spec7 = opt::empty
		)
		{ option(opt::shortname(shortname), opt::arg_none, spec2, spec3, spec4, spec5, spec6, spec7); }

	/// convenience function for specifying options which require an argument
	void option_arg(
		const std::string &name, 				///< key and long name of the option
		const std::string &description = "",			///< description of this option (used to construct the Options::usage() string
		const std::string &defaultvalue = Option::nodefault	///< the value to assign to (*this)[key] if the option is \b not specified on the command line
		)
		{
			option(name, name, 0, "--", Option::required, defaultvalue, description);
		}

	/// convenience function for specifying switches (an option not requiring an argument)
	void option_switch(
		const std::string &name, 				///< key and long name of the option
		const std::string &description = "",			///< description of this option (used to construct the Options::usage() string
		const char shortname = 0,				///< short command line name of this option (eg. if your option is -u, shortname would be 'u')
		const std::string &value = "1"				///< the value to assign to (*this)[key] if the option is specified on the command line, without arguments
		)
		{
			char sn = // if name is single-character, set shortname switch as well (it's usually what you want)
				name.size() == 1 && shortname == 0 ? 
				sn = name[0] : shortname;
			option(name, name, sn, value, Option::none, Option::nodefault, description);
		}
	/**
		\brief describe how to parse a command line argument
		
		argument() should be called in order for each command line argument you need. Eg., for
\code
	./volume.x <arg1> <arg2> [arg3 [arg4]]
\endcode

		your calls to argument() would be:
		
		Options opts;
		opts.argument("arg1", "Description of arg1");
		opts.argument("arg2", "Description of arg1");
		opts.argument("arg3", "Description of arg1", Option::optional);
		opts.argument("arg4", "Description of arg1", Option::optional);
	*/
	void argument(
		const std::string &key_, 					///< key which this option modifies
		const std::string &description_ = "",			///< description of this option (used to construct the Options::usage() string
		const Option::Argument argument_ = Option::required,	///< Is this command line option required or optional? 
											///< After the first optional argument, all subsequent will be considered optional.
		const std::string &defaultvalue_ = Option::nodefault	///< the value to assign to (*this)[key] if the argument is \b not specified on the command line
		);
		
	/// Parses the command line and loads the options, as specified by \a options argument
	void parse(int argc, char **argv);

	/// Returns \c true if the option \a o was present on the command line
	bool found(const std::string &o) { return optionsGiven.count(o) != 0; }

	/// Returns the number of command line arguments that were found	
	int arguments_found() { return nargs; }

	/// Returns a human readable string describing the way to use the program, and the options avaliable
	std::string usage(char **argv = NULL);
};

} // namespace system
} // namespace peyton

#define __peyton_system peyton::system

#endif
