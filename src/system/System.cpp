#include <astro/system.h>

#include <stdlib.h>

using namespace peyton::exceptions;

const char *peyton::system::workspace_internal(const char *envvar)
{
	char *ws = getenv(envvar);
	if(ws == NULL)
	{
		THROW(EWorkspaceNotSet, std::string(envvar) + " variable not set. Aborting.");
	}
	return ws;
}
