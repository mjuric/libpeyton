#include <astro/compat/compat.h>

#include <astro/asteroids/catalog.h>

#include <stdio.h>
#include <string.h>

#include "catalogs/BowellCatalog.h"
#include "catalogs/NativeCatalog.h"
#include "catalogs/MemoryCatalog.h"

#pragma warning(disable: 4786)

using namespace peyton::exceptions;
using namespace peyton::asteroids;

Catalog *Catalog::open(const char *filename, const char *type, const char *mode)
{
	if(type == NULL) { THROW(ENotImplemented, "Catalog autodetection not implemented yet"); }

	// create catalog
	Catalog *cat = NULL;

	if(!stricmp(type, "ASTORB2")) {
		cat = new BowellCatalog;
	} else if(!stricmp(type, "NATIVE")) {
		cat = new NativeCatalog;
	} else if(!stricmp(type, "NATIVE-MEMORY")) {
		cat = new MemoryCatalog;
	} else {
		THROW(ECatalogTypeUnknown, "Catalog type unknown");
		return NULL;
	}

	// open the catalog
	if(!cat->openCatalog(filename, mode)) {
		delete cat;
		return NULL;
	} else {
		return cat;
	}
}

int Catalog::read(std::vector<Asteroid> &o, int *ids, int num)
{
	o.erase(o.begin(), o.end());
	o.resize(num);
	for(int i = 0; i != num; i++) {
		read(o[i], ids[i]);
	}

	return num;
}


