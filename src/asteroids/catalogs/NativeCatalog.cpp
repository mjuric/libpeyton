#include <astro/system/log.h>

#include <stdio.h>

#include "NativeCatalog.h"

using namespace peyton::asteroids;

inline char * underscore(char *c) {
	int l = strlen(c);
	while(l) { if(c[l] == ' ') c[l] = '_'; l--; }
	return c;
}

bool NativeCatalog::initialize(size_t size)
{
	if(size % recordByteLen != 0) {
		fclose(fp); fp = NULL;
		DEBUG(basic) << "Size of catalog not a multiple of record length";
		return false;
	}

	recordCnt = size / recordByteLen;

	// construct a name/id map
	Asteroid a;
	for(int i = 0; i != recordCnt; i++) {
		read(a, -1);
		names[underscore(strdup(a.name))] = a.id;
	}
	return true;
}

bool NativeCatalog::openCatalog(const char *filename, const char *mode)
{
	if(!strcmp(mode, "r")) {
		// read mode
		fp = fopen(filename, "rb");
		if(fp == NULL) { DEBUG(basic) << "Error opening catalog"; return false; }

		fseek(fp, 0, SEEK_END);
		size_t size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		return initialize(size);
	} else if(!strcmp(mode, "w")) {
		// write mode
		fp = fopen(filename, "wb");
		if(fp == NULL) { DEBUG(basic) << "Error opening catalog"; return false; }
	}
	
	return true;
}

int NativeCatalog::write(Asteroid &obj, int at)
{
	if(at != -1) { fseek(fp, at*recordByteLen, SEEK_SET); }

	return fwrite(&obj, recordByteLen, 1, fp);
}

char *NativeCatalog::identify(char *name) {
	strcpy(name, "NATIVE");
	return name;
}


int NativeCatalog::read(Asteroid &obj, const char *name)
{
	NameMap::iterator i = names.find(name);
	if(i == names.end()) return -1;
	
	return read(obj, (*i).second);
}

int NativeCatalog::read(Asteroid &obj, const int id)
{
	if(id != -1) {
		fseek(fp, id*recordByteLen, SEEK_SET);
	}

	fread(&obj, recordByteLen, 1, fp);

	return 0;
}

int NativeCatalog::read(std::vector<Asteroid> &aobj, const int from_in, const int to_in)
{
	int from = from_in, to = to_in;
	if(to > recordCnt) {
		to = recordCnt;
	}

	aobj.erase(aobj.begin(), aobj.end());
	aobj.resize(to - from);
	int cnt = 0;

	// locate first record
	fseek(fp, from*recordByteLen, SEEK_SET);

	while(cnt != to - from) {
		Asteroid &obj = aobj[cnt];

		read(obj, -1);

		cnt++;
	}

	return cnt;
}

int NativeCatalog::recordCount() { return recordCnt; }

NativeCatalog::~NativeCatalog() {
	if(fp != NULL) fclose(fp);
	while(names.size()) {
		const char *c = (*names.begin()).first;
		names.erase(names.begin());
		free((char *)c);
	}
}
