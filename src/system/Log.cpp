#include <astro/system/log.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <iostream>

using namespace peyton::system;

char Log::buf[1000];
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

int Log::debugLevel = 10;

int Log::level(int newlevel)
{
	if(newlevel < 0) { return debugLevel; }

	std::swap(newlevel, debugLevel);
	return newlevel;
}


Log::linestream::linestream(int level)
: std::ostringstream()
{
	(*this) << " [ debug" << level << " ] ";
}

Log::linestream::~linestream()
{
	(*this) << "\n"/* << std::ends*/; std::cerr << str();
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
