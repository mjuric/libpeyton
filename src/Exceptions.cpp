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
	DEBUG(exception) << (std::string)(*this);
}

EAny::operator std::string() const
{
	std::ostringstream s;
	s << io::format("[%s] : %s (at %s:%d)") << type_name(*this) << info << file << line;
	return s.str();
}

EErrno::EErrno(std::string inf, std::string fun, std::string f, int l)
	: EAny(inf, fun, f, l)
{
	info += " [errno = " + str(errno) + " \"" + strerror(errno) + "\"]";
	err = errno;
}

EAny::~EAny() throw()
{
	if(thrown) { peyton::exceptions::exceptionRaised--; } // std::cerr << "Decremented exceptionRaised to " << peyton::exceptions::exceptionRaised << "\n"; }
}

EAny::EAny(const EAny &b)
{
	// std::cerr << "Copy constructor.\n";

	info = b.info;
	line = b.line;
	func = b.func;
	file = b.file;
	thrown = b.thrown;
	
	if(thrown) { peyton::exceptions::exceptionRaised++; } //std::cerr << "Inc. er = " << exceptionRaised << "\n";}
}

void peyton::exceptions::log_exception(const EAny &e) throw()
{
	 DEBUG(exception) << "[throw suppressed]  " << (const std::string)e;
}
