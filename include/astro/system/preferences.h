#ifndef _astro_preferences_h
#define _astro_preferences_h

#include <astro/system/mixed.h>
#include <functional>
#include <map>

namespace peyton {
namespace system {

struct less_str : public std::binary_function<char *, char *, bool> {
	bool operator()(const char *x, const char *y) const {
		return strcmp(x, y) < 0;
	}
};

/**
	\brief Class for loading/accessing preferences in form of a flat key-value map
	
	The preferences class looks for file $WORKSPACE/lib/prefs.txt where $WORKSPACE is
	the path returned by peyton::system::workspace() function. The format of this file
	will not be documented since the \c Preferences class is deprecated.
	
	\deprecated This class has been depricated in favor of peyton::system::Config class
*/
class Preferences {
protected:
	typedef std::map<char *, Mixed, less_str> Map;
	Map db;
public:
	Preferences();
	Mixed &operator [](const char *pref) {
		Map::iterator i = db.find((char *)pref);
		if(i != db.end()) return (*i).second;
		return db[strdup(pref)];
	}
	~Preferences();
};

}
}

#define __peyton_system peyton::system

#endif
