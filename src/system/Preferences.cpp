#include <astro/compat/compat.h>

#include <astro/system/preferences.h>
#include <astro/system.h>
#include <astro/system/error.h>

#include <fstream>
#include <stdio.h>
#include <unistd.h>

using namespace peyton;
using namespace peyton::system;

Preferences::Preferences()
{
	const char *ws = peyton::system::workspace();
	char buf[1001];
	sprintf(buf, "%s/lib/%s", ws, "prefs.txt");

	if(access(buf, 04)) {
		DEBUG(terminate, "Cannot access preferences file " << buf << " (SDSSAST_WORKSPACE environment variable incorrect ?)");
		exit(-1);
	}

	std::ifstream f(buf);

	char type[100], kw[100];
	int valBeg;
	while(!f.eof()) {
		f.getline(buf, 1000);
		if(strlen(buf) < 3 || buf[0] == '#') continue;

		sscanf(buf, "%s %s %n", type, kw, &valBeg);

		char *val = &buf[valBeg];

		Mixed &m = (*this)[kw];
		if(!stricmp(type, "int")) {
			m = atoi(val);
		} else if(!stricmp(type, "double")) {
			m = atof(val);
		} else if(!stricmp(type, "bool")) {
			if(!stricmp(type, "true")) m = true;
			else m = false;
		} else if(!stricmp(type, "string")) {
			m = val;
		} else {
			DEBUG(basic, "Unknown type in preferences file");
		}
	}
}

Preferences::~Preferences() {
	while(db.size()) {
		char *c = (*db.begin()).first;
		db.erase(db.begin());
		free(c);
	}
}
