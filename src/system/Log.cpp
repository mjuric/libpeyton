#include <astro/system/log.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

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
