#include <astro/system/fs.h>

#include <glob.h>
#include <astro/exceptions.h>
#include <astro/useall.h>

using namespace std;

dir::dir(const std::string &path)
{
	glob_t globbuf;
	int err;

	if(err = glob(path.c_str(), GLOB_TILDE, NULL, &globbuf))
	{
		switch(err) {
		case GLOB_NOSPACE: 	THROW(EIOException, string("Ran out of memory while reading [") + path + "]");
		case GLOB_ABORTED: 	THROW(EIOException, string("Error reading [") + path + "]");
		case GLOB_NOMATCH: 	return;
		default: 		THROW(EIOException, string("Unknown error while reading [") + path + "]");
		}
	}
	
	char **p = globbuf.gl_pathv;
	while(*p) { push_back(*p); p++; }
}


#if 0
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>


class dir
{
public:
	DIR *d;
public:
	dir(const std::string &path, const std::string &filter = "");

	bool next(std::string &dent);

	~dir();
};

dir::dir(const std::string &path, const std::string &filter)
 : d(NULL)
{
	d = opendir(path.c_str());
	if(d == NULL) { THROW(EIOException, string("Cannot open directory [") + path + "]"); }
}

bool dir::next(std::string &dent)
{
	ASSERT(d != NULL);

	dirent *ent = readdir(d);
	if(ent == NULL)
	{
		if(errno != 0) { THROW(EIOException, string("Error listing directory [") + path + "]"); }
		return false;
	}

	fnmatch(filter.c_str(), ent->d_name, 
}

dir::~dir()
{
	if(closedir(d) != 0) { THROW(EIOException, string("Error closing directory [") + path + "]"); }
}

#endif
