#include <astro/exceptions.h>
#include <astro/util.h>
#include <astro/system/log.h>
#include <astro/io/format.h>
#include <sstream>

#include <typeinfo>
#include <errno.h>

using namespace peyton;
using namespace peyton::system;
using namespace peyton::util;
using namespace peyton::exceptions;

void EAny::print() throw()
{
	Log::debug(Log::exception, "[%s] : %s (at %s:%d)", typeid(*this).name(), info.c_str(), file.c_str(), line);
}

EAny::operator std::string()
{
	std::ostringstream s;
	s << io::format("[%s] : %s (at %s:%d)") << typeid(*this).name() << info.c_str() << file.c_str() << line;
	return s.str();
}

EErrno::EErrno(std::string inf, std::string fun, std::string f, int l)
	: EAny(inf, fun, f, l)
{
	info += " [errno = " + str(errno) + " \"" + strerror(errno) + "\"]";
	err = errno;
}

