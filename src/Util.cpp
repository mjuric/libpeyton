#include <astro/util.h>
#include <astro/util/varexpand.h>

#include <astro/constants.h>
#include <astro/time.h>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <string>
#include <map>
#include <iostream>

using namespace peyton;

Radians
util::approxSunLongitude(MJD time)
{
	double t = (time + 0.5 - 51545) / 36525;
	double m = (357.528+35999.050*t) * ctn::d2r;
	double l = 280.460+36000.772*t + (1.915 - 0.0048*t)*sin(m)
		+0.020*sin(2*m);
	return l*ctn::d2r;
}

char *util::trim(char *dest, const char *src)
{
	int cnt = 0;
	while(isspace(*src) && *src) src++;
	while(*src) dest[cnt++] = *(src++);
	cnt--;
	while(cnt > -1 && isspace(dest[cnt])) cnt--;
	dest[cnt+1] = 0;
	return dest;
}

char *util::trim(char *txt)
{
	char *s = strdup(txt);
	trim(txt, s);
	free(s);
	return txt;
}

///////////

std::string util::ltrim( const std::string &str, const std::string &whitespace)
{

   int idx = str.find_first_not_of(whitespace);
   if( idx != std::string::npos )
       return str.substr(idx);
   
   return "";
}

std::string util::rtrim( const std::string &str, const std::string &whitespace)
{

   int idx = str.find_last_not_of(whitespace);
   if( idx != std::string::npos )
       return str.substr(0,idx+1);

   return str;
}

std::string util::trim( const std::string &str, const std::string &whitespace)
{
    return rtrim(ltrim(str));
}

std::string util::unescape(const std::string &str)
{
	std::string tmp;
	tmp.reserve(str.size());

	FOR(0, str.size())
	{
		if(str[i] != '\\')
		{
			tmp += str[i];
		}
		else
		{
			// go to next character, unless we're at the end of the string
			if(++i == str.size()) break;
			tmp += str[i];
		}
	}
	
	return tmp;
}

///////////

/*
	Given a hashtable of key-value pairs, replace every occurence
	of $key in any value with hash[key] thus making for a simple 
	variable expansion engine
	
	TODO: error checking
*/
inline bool isvarchar(const char ch)
{
	if(isalnum(ch) || ch == '_' || ch == '.') return true;
	return false;
}

std::string expand_variable(std::string value, std::map<std::string, std::string> &h, const std::string &key = "", std::map<std::string, bool> *expanded = NULL)
{
	int at = 0, len;

	//std::cout << "Expanding " << key << "\n";

	std::string name;
	do {
		int idx = value.find('$', at);

		//std::cout << "\tidx=" << idx << "\n";

		if(idx == std::string::npos) { break; }
		if(idx+1 == value.size()) { break; }

		// extract variable name
		if(value[idx+1] == '{')
		{
			at = value.find('}', idx+1);
			if(at == std::string::npos) { break; }

			len = at - idx + 1;
			name = value.substr(idx + 2, len - 3);
		}
		else
		{
			at = idx + 1;
			while(at < value.size() && isvarchar(value[at])) { at++; }

			len = at - idx;
			name = value.substr(idx + 1, len - 1);
		}
		//std::cout << "\tname=" << name << "\n";

		if(h.count(name))
		{
			//std::cout << "\t" << name << " exists\n";
			if(key.size() && !expanded->count(name))
			{
				expand_variable(h[name], h, name, expanded);
			}
			value.replace(idx, len, h[name]);
		}
		else
		{
			value.erase(idx, len);
		}

		//std::cout << "\tvalue=" << value << "\n";
	} while(true);
	
	if(key.size())
	{
		(*expanded)[key] = true;
		h[key] = value;
	}

	//std::cout << "\t---\n";
	return value;
}

bool util::expand_dict(std::map<std::string, std::string> &h)
{
	//std::cout << "\n\nexpanding variables\n";

	std::map<std::string, bool> expanded;
	FOREACH(h)
	{
		if(!expanded.count((*i).first))
		{
			expand_variable((*i).second, h, (*i).first, &expanded);
		}
		//std::cout << (*i).first << " -> " << (*i).second << "\n";
	}
	return true;
}

std::string util::expand_text(std::string text, std::map<std::string, std::string> &dictionary)
{
	return expand_variable(text, dictionary);
}
