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

/**
	\brief	Command line option specification class
	
	Used to describe permissable command line options. This class should not be used/constructed directly.
	Use peyton::system::Options::option() and peyton::system::Options::argument() instead.
*/
struct Option
{
	/// Argument types for peyton::system::Option
	enum Argument
	{
		none = 0,		///< Option does not have a parameter
		required = 1,	///< Option must come with a parameter
		optional = 2	///< Option may come with a parameter
	};

	std::string				key;		///< key to set in the map for this option

	std::string				name;		///< long option name
	char					shortname;	///< short option name
	std::string				value;	///< value to be returned if the option was given on the command line but
								///< either argument=no or argument=optional and no argument was specified
	Argument argument;				///< does the option have an argument

	std::string				defaultvalue;///< if it was not specified on the command line
	std::string				description;///< description of this option

	static std::string		nodefault;	///< pass this string as defaultvalue to have no default value

public:
	bool hasdefaultvalue;				///< does this option have a default value

public:
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
class Options : public peyton::system::Config {
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
