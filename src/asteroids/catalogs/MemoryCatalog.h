#ifndef _astro_memorycatalog_h
#define _astro_memorycatalog_h

#include <astro/asteroids/catalog.h>

/**
	\brief Memory-based catalog in NATIVE format

	A quick hack to allow reading memory based NATIVE catalogs.
	In filename parameter, pass a pointer to the following structure:

		struct { int length; char *contents };

	where length is the length of the contents which contains the file
	data. fmemopen is then used to emulate FILE* access to that location.
*/

namespace peyton {
namespace asteroids {

class MemoryCatalog : public NativeCatalog {
protected:
	virtual bool openCatalog(const char *filename, const char *mode);

public:
	MemoryCatalog() : NativeCatalog() {}
	virtual char *identify(char *);
};

}
}

#endif
