#include <astro/system/log.h>

#include <stdio.h>

#include <utility>

#include "NativeCatalog.h"
#include "MemoryCatalog.h"

using namespace peyton::asteroids;

typedef std::pair<char *, size_t> mcat;

bool MemoryCatalog::openCatalog(const char *filename, const char *mode)
{
	mcat *cat = (mcat *)filename;
	if(!strcmp(mode, "r")) {
		// read mode
		fp = fmemopen(cat->first, cat->second, "rb");
		if(fp == NULL) { DEBUG(basic, "Error opening memory catalog"); return false; }

		return initialize(cat->second);
	} else if(!strcmp(mode, "w")) {
		// write mode -- TODO: this should use open_memstream?
		fp = fmemopen(cat->first, cat->second, "wb");
		if(fp == NULL) { DEBUG(basic, "Error opening catalog"); return false; }
	}

	return true;
}

char *MemoryCatalog::identify(char *name) {
	strcpy(name, "NATIVE-MEMORY");
	return name;
}

