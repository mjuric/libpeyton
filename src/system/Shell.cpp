#include <astro/system/shell.h>
#include <astro/util.h>
#include <astro/exceptions.h>

#include <cstdlib>

using namespace std;
using namespace peyton::util;
using namespace peyton::exceptions;

void peyton::system::shell(const std::string &cmd) throw(EIOException)
{
	int status = ::system(cmd.c_str());
	if(status == -1 || WEXITSTATUS(status) != 0)
	{
		THROW(EIOException, string("Error executing '") + cmd + "' (system() returned " + str(status) + " - exit code " + str(WEXITSTATUS(status)));
	}
}
