#include <astro/io/compress.h>
#include <astro/util.h>
#include <astro/exceptions.h>

#include <cstdlib>

using namespace std;
using namespace peyton::io;
using namespace peyton::util;
using namespace peyton::exceptions;

void compress::gzip(const std::string &filename) throw(EIOException)
{
	int status = ::system((string("gzip -f ") + filename).c_str());
	if(status == -1 || WEXITSTATUS(status) != 0)
	{
		THROW(EIOException, string("Error gzipping file '") + filename + "' (system() returned " + str(status) + " - exit code " + str(WEXITSTATUS(status)));
	}
}
