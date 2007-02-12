#ifndef _astro_log_h
#define _astro_log_h

#include <sstream>
#include <assert.h>

namespace peyton {
namespace system {

/**
	\brief Class for event logging and debugging.
	
	\warning Do not use this class directly. Instead, use DEBUG(), ERRCHECK() and ASSERT() macros instead.
*/
class Log {
protected:
	char buf[1000];

	std::string name;
	int debugLevel;
	bool debuggingOn;
public:
#if 0
	void write(const char *text, ...);
	void debug(int level, const char *text, ...);

	std::ostream &stream_begin();
	void stream_end();
#endif
	enum { terminate=-2, error, exception, verb1, verb2, verb3, verb4, verb5 };

public:
	class linestream : public std::ostringstream {
	protected:
		Log &parent;
	public:
		linestream(Log &parent, int level);
		~linestream();
		std::ostringstream &stream();
	};
	Log(const std::string &name, int level = exception, bool on = true);
	int level(int newlevel = -1);
	const std::string &identify() { return name; }
};

namespace logs
{
	extern Log peyton; ///< predefined log for libpeyton debugging
	extern Log app; ///< predefined log for application messaging
}

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

/**
	\brief		This function is only to be used through ASSERT() macro
*/
bool assert_msg_message(const char *assertion, const char *func, const char *file, int line);
bool assert_msg_abort();

}
}

#if defined(__GNUG__) || defined (__INTEL_COMPILER)
#define ASSERT(cond) \
	for(bool __nuke = !(cond); __nuke && peyton::system::assert_msg_message(#cond, __PRETTY_FUNCTION__, __FILE__, __LINE__); peyton::system::assert_msg_abort())
#else
#define ASSERT(cond) \
	for(bool __nuke = !(cond); __nuke && peyton::system::assert_msg_message(#cond, "<function_name_unsupported>"_, __FILE__, __LINE__); peyton::system::assert_msg_abort())
#endif

#define ERRCHECK(condition) if(condition)
//#define ASSERT(cond) assert(cond)

//	if(!(cond)) { DEBUG(terminate, "Assertion [" #cond "] failed at " << __PRETTY_FUNCTION__ << ", " << __FILE__ << ":" << __LINE__); abort(); }

#define __peyton_system peyton::system

#endif
