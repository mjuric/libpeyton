#ifndef __astro_util_varexpand_h
#define __astro_util_varexpand_h

#include <astro/util.h>
#include <map>

namespace peyton {
namespace util {

	//
	// Expands variables, PERL style, given a hash table of variables
	// eg. "Blah blah $var1 and expand with dots ${complicated var} blah blah"
	//

	//
	// This recursively expands the given hashtable
	// Usefull for eg. loading config files and expanding the names
	//
	bool expand_dict(std::map<std::string, std::string> &dictionary);

	//	
	// This expands the text, given a hashtable
	//
	std::string expand_text(std::string text, std::map<std::string, std::string> &dictionary);

}
}

#define __peyton_util peyton::util

#endif
