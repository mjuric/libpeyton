#if 0

#include <astro/system/error.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>

int Error::code;
char Error::description[1000];
int Error::type;

void Error::error(int c, const char *text, ...)
{
	type = Error::err;
	code = c;

	va_list marker;
	va_start(marker, text);

	vsprintf(description, text, marker);

	va_end(marker);

	std::cerr << "Error: " << description << "\n";
}

void Error::warning(int c, const char *text, ...)
{
	type = Error::warn;
	code = c;

	va_list marker;
	va_start(marker, text);

	vsprintf(description, text, marker);

	va_end(marker);

	std::cerr << "Warning: " << description << "\n";
}

#endif
