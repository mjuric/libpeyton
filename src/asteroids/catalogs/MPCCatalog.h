#ifndef _astro_mpccatalog_h
#define _astro_mpccatalog_h

#include <astro/asteroids/catalog.h>

namespace peyton {
namespace asteroids {

class MPCCatalog : public Catalog {
protected:
	enum {recordByteLen = 266 + 1 };
	int recordCnt;
	virtual bool openCatalog(const char *filename, const char *mode);
	FILE *fp;

public:
	MPCCatalog() { fp = NULL; recordCnt = -1; }

	virtual int read(Asteroid &obj, const char *name);
	virtual int read(std::vector<Asteroid> &obj, const int from, const int to);
	virtual int read(Asteroid &obj, const int id);
	virtual int recordCount();

	virtual int write(Asteroid &obj, int at);

	virtual char *identify(char *);

	virtual ~MPCCatalog();
};

}
}

#endif
