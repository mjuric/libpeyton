#ifndef __astro_system_shell_h
#define __astro_system_shell_h

#include <astro/exceptions.h>

namespace peyton {
namespace system {

	/// execute a given command line with the standard shell (a proxy for ::system call)
	void shell(const std::string &cmd) throw(peyton::exceptions::EIOException);

}
}

#define __peyton_system peyton::system

#endif
