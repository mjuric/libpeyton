#ifndef _astro_nativecatalog_h
#define _astro_nativecatalog_h

#include <astro/asteroids/catalog.h>

#include <map>

#pragma warning(disable: 4786)

namespace peyton {
namespace asteroids {

struct less_str : public std::binary_function<char *, char *, bool> {
	bool operator()(const char *x, const char *y) const {
		return strcmp(x, y) < 0;
	}
};

class NativeCatalog : public Catalog {
protected:
	enum {recordByteLen = sizeof(Asteroid)};
	int recordCnt;
	virtual bool openCatalog(const char *filename, const char *mode);
	FILE *fp;

	typedef std::map<const char *, int, less_str> NameMap;
	NameMap names;

	friend class Catalog;
	
	bool initialize(size_t size);
public:
	NativeCatalog() { fp = NULL; }

	virtual int read(Asteroid &obj, const char *name);
	virtual int read(std::vector<Asteroid> &obj, const int from, const int to);
	virtual int read(Asteroid &obj, const int id);
	virtual int recordCount();

	virtual int write(Asteroid &obj, int at);

	virtual char *identify(char *);

	virtual ~NativeCatalog();
};

}
}

#endif
