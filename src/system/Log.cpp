#include <astro/system/log.h>
#include <astro/system/fs.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <iostream>

using namespace peyton::system;

LOG_DEFINE(peyton, exception, true);
LOG_DEFINE(app, verb1, true);

#if 0
void Log::write(const char *text, ...)
{
	va_list marker;
	va_start(marker, text);

	vsprintf(buf, text, marker);

	va_end(marker);

	std::cerr << buf << "\n";
}

void Log::debug(int level, const char *text, ...)
{
	va_list marker;
	va_start(marker, text);

	vsprintf(buf, text, marker);

	va_end(marker);

	std::cerr << "L" << level << ": " << buf << "\n";
}
#endif

int Log::level(int newlevel)
{
	if(newlevel < 0) { return debugLevel; }

	std::swap(newlevel, debugLevel);
	return newlevel;
}

Log::Log(const std::string &n, int level, bool on)
: name(n), debugLevel(level), debuggingOn(on)
{
	// specific to this object
	{
	EnvVar e_on(name + "_debug_on");
	if(e_on) { debuggingOn = atoi(e_on.c_str()) != 0; }
	EnvVar e_l(name + "_debug_level");
	if(e_l) { debugLevel = atoi(e_l.c_str()); }
	}
	
	// global commands
	{
	EnvVar e_on("all_debug_on");
	if(e_on) { debuggingOn = atoi(e_on.c_str()) != 0; }
	EnvVar e_l("all_debug_level");
	if(e_l) { debugLevel = atoi(e_l.c_str()); }
	}
	
	// make a sound if called for
	{
	EnvVar e_on("all_debug_list");
	EnvVar e_on1(name + "_debug_list");
	if(e_on || e_on1)
	{
		linestream(*this, -3).stream() << "Log '" << identify()
			<< "', debug_on = " << debuggingOn << ", "
			<< "debug_level = " << debugLevel;
	}
	}
}

Log::linestream::linestream(Log &p, int level)
: parent(p), std::ostringstream()
{
	(*this) << " [ " << parent.identify() << ", " << level << " ]   ";
}

Log::linestream::~linestream()
{
	(*this) << "\n"; std::cerr << str();
}

std::ostringstream &Log::linestream::stream()
{
	return *this;
}

bool peyton::system::assert_msg_abort()
{
	std::cerr << "\n";
	std::cerr << "##############################################################################\n";
	std::cerr << "# Dump complete.\n";
	std::cerr << "##############################################################################\n";
	std::cerr << "\n\n\n";
	abort();
}

bool peyton::system::assert_msg_message(const char *assertion, const char *func, const char *file, int line)
{
	std::cerr << "\n\n\n";
	std::cerr << "############################## ASSERTION FAILED ##############################\n";
	std::cerr << "# Condition : " << assertion << "\n";
	std::cerr << "#\n";
	std::cerr << "# Function  : " << func << "\n";
	std::cerr << "# Source    : " << file << "\n";
	std::cerr << "# Line      : " << line << "\n";
	std::cerr << "##############################################################################\n";
	std::cerr << "# Possible auxilliary information to follow:\n";
	std::cerr << "##############################################################################\n";
	std::cerr << "\n";
	return true;
}
