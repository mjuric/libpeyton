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

int peyton::exceptions::exceptionRaised = 0;

void EAny::print() const throw()
{
	MLOG(exception) << (std::string)(*this);
}

EAny::operator std::string() const
{
	std::ostringstream s;
	s << io::format("Exception: %s\nInfo     : %s\nDetails  : %s:%d\n%s") << type_name(*this) << info << file << line << stacktrace;
	return s.str();
}

EErrno::EErrno(const std::string &inf, const std::string &fun, const std::string &f, int l, const std::string &st)
	: EAny(inf, fun, f, l, st)
{
	info += " [errno = " + str(errno) + " \"" + strerror(errno) + "\"]";
	err = errno;
}

EAny::~EAny() throw()
{
	if(thrown) { peyton::exceptions::exceptionRaised--; } // std::cerr << "Decremented exceptionRaised to " << peyton::exceptions::exceptionRaised << "\n"; }
}

EAny::EAny(const EAny &b)
	: info(b.info), line(b.line), func(b.func), file(b.file), stacktrace(b.stacktrace), thrown(b.thrown)
{
	if(thrown) { peyton::exceptions::exceptionRaised++; } //std::cerr << "Inc. er = " << exceptionRaised << "\n";}
}

void peyton::exceptions::log_exception(const EAny &e) throw()
{
	 DEBUG(exception) << "[throw suppressed]  " << (const std::string)e;
}
