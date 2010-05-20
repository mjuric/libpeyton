#ifndef _astro_log_h
#define _astro_log_h

#include <sstream>
#include <cassert>
#include <astro/assert.h>

namespace peyton {
namespace system {

// print stack backtrace (works on platforms that use glibc
// NOTE: for function names to be printed out you have to add -rdynamic to LDFLAGS
void print_trace();
std::string stacktrace();

/**
	\brief Class for event logging and debugging.
	
	\warning Do not use this class directly. Instead, use DEBUG(), ERRCHECK() and ASSERT() macros instead.
*/
class Log {
protected:
	char buf[1000];
	std::ostream *output;

	static int m_counter;
	static int counter();

	std::string name;
	int logLevel;
	bool loggingOn;
public:
	enum { terminate=-2, error, exception, verb1, verb2, verb3, verb4, verb5 };

public:
	class linestream : public std::ostringstream {
	protected:
		Log &parent;
		std::string prefix;
	public:
		linestream(Log &parent, int level, const char *subsys = NULL);
		~linestream();
		std::ostringstream &stream();
	};
	Log(const std::string &name, int level = exception, bool on = true);
	~Log();
	int level(int newlevel = -1);
	const std::string &identify() { return name; }
};

namespace logs
{
	extern Log peyton; ///< predefined log for libpeyton logging
	extern Log app; ///< predefined log for application messaging
	extern Log debug; ///< predefined log for application debugging -- to be used through DLOG macro
	extern Log message; ///< predefined log for general application messages -- to be used through MLOG macro
}

} // namespace system
} // namespace peyton

//	namespace peyton { namespace system { namespace logs { 

#define LOG_DECLARE(name) \
	namespace peyton { namespace system { namespace logs { \
	extern peyton::system::logs::Log peyton::system::logs::name; \
	} } }

#define LOG_DEFINE(name, lev, on) \
	::peyton::system::Log peyton::system::logs::name(#name, ::peyton::system::Log::lev, on);

#define LOG(log, lev) \
	if(peyton::system::logs::log.level() >= peyton::system::Log::lev) \
		peyton::system::Log::linestream(peyton::system::logs::log, peyton::system::Log::lev).stream()

#define DEBUG(lev) LOG(peyton, lev)

#define DLOG(lev) \
	if(peyton::system::logs::debug.level() >= peyton::system::Log::lev) \
		peyton::system::Log::linestream(peyton::system::logs::debug, peyton::system::Log::lev, __PRETTY_FUNCTION__).stream()

#define MLOG(lev) \
	if(peyton::system::logs::message.level() >= peyton::system::Log::lev) \
		peyton::system::Log::linestream(peyton::system::logs::message, peyton::system::Log::lev, __PRETTY_FUNCTION__).stream()


#define __peyton_system peyton::system

#endif
