#ifndef __astro_system_system_h
#define __astro_system_system_h

#include <astro/exceptions.h>

namespace peyton {

namespace exceptions {
	SIMPLE_EXCEPTION(EWorkspaceNotSet);
}

/// classes/functions related to the OS
namespace system {

	const char *workspace_internal(const char *envvar);

	/**
	*	\brief 	Returns a path to workspace dir.
	*	
	*	A "workspace" is simply a path relative to which the program which uses libpeyton
	*	works (stores it's files, etc..). Workspace is usually read from an environment
	*	variable, which name must be set at compile time, by defining the LIBPEYTON_WORKSPACE
	*	environment variable. Eg, to have the workspace be red from SDSSAST_WORKSPACE variable,
	*	you'd compile your file using:
	*	
	*		\c gcc \c -DLIBPEYTON_WORKSPACE=\"SDSSAST_WORKSPACE\"
	*	
	*	If no compile time workspace is specified, the value of HOME environment variable will
	*	be used.
	*	
	*	\throws peyton::exceptions::EWorkspaceNotSet The workspace environment variable is not set
	*	\warning THIS IS NOT IMPLEMENTED YET - THE WORKSPACE IS HARDCODED TO SDSSAST_WORKSPACE
	*	\deprecated This method of locating workspaces is deprecated. Use peyton::system::Config class instead
	*/
	inline const char *workspace() throw(peyton::exceptions::EWorkspaceNotSet)
	{
		return workspace_internal("SDSSAST_WORKSPACE");

#if 0
		#ifdef LIBPEYTON_WORKSPACE
			return workspace_internal(LIBPEYTON_WORKSPACE);
		#else
			#error If you are going to use workspace() function, please define LIBPEYTON_WORKSPACE
			return workspace_internal("HOME");
		#endif
#endif
	}
}
namespace System = system;

}

#define __peyton_exceptions peyton::exceptions
#define __peyton_system peyton::system

#endif
