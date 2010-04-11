#include <astro/util.h>
#include <astro/util/varexpand.h>
#include <astro/system/fs.h>

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
#include <fstream>

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

std::string util::pad(const std::string &s, size_t n, char c)
{
	std::string ret(s);
	ret.reserve(n+1);
	for(int i = ret.size(); i != n; i++)
	{
		ret += c;
	}
	return ret;
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

std::string expand_variable(std::string value, std::map<std::string, std::string> &h, bool allowEnvironmentVariables, const std::string &key = "", std::map<std::string, bool> *expanded = NULL)
{
	int at = 0, len;
	std::string value0(value);

	//std::cout << "Expanding " << key << "\n";

	std::string name;
	do {
		int idx = value.find('$', at);

		//std::cout << "\tidx=" << idx << "\n";

		if(idx == std::string::npos) { break; }
		if(idx+1 == value.size()) { break; }

		// skip parsing of artithmetic expressions (the next loop will do that)
		if(value[idx+1] == '(')
		{
			at++;
			continue;
		}

		int at0 = at;
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

		bool foundvar = false;
		if(h.count(name))
		{
			//std::cout << "\t" << name << " exists\n";
			if(key.size() && !expanded->count(name))
			{
				expand_variable(h[name], h, allowEnvironmentVariables, name, expanded);
			}
			value.replace(idx, len, h[name]);
			foundvar = true;
		}
		else if(allowEnvironmentVariables)
		{
			peyton::system::EnvVar env(name);
			if(env)
			{
				value.replace(idx, len, env);
				foundvar = true;
			}
		}

		at = at0;
		if(!foundvar)
		{
#if 0
			// If the variable has not been found, leave it as-is
			// NOTE: this is a change wrt. the old behavior, where we'd just erase it.
			at += len;
#else
			value.erase(idx, len);
#endif
			//MLOG(verb2) << "Variable '" << name << "' not found (while expanding '" << value0 << "')";
		}

		//std::cout << "\tvalue=" << value << "\n";
	} while(true);

	// expand arithmetic expressions
	if(allowEnvironmentVariables)
	{
		at = 0;
		while(true)
		{
			int idx = value.find("$(", at);

			if(idx == std::string::npos) { break; }
			if(idx+1 == value.size()) { break; }

			// find the closing parenthesis of the expression, noting that
			// the expression itself may contain parentheses
			at = idx+2;
			int pths = 1;
			while(at < value.size() && pths)
			{
				if(value[at] == '(') { pths++; }
				if(value[at] == ')') { pths--; }
				at++;
			}
			ASSERT(pths == 0);
			len = at - idx;
			std::string expr = value.substr(idx + 2, len - 3);
			expr += "\n";

			// evaluate arithmetic expressions by constructing a PERL statement
			// WARNING: HACK: NOTE: THIS IS TERRIBLY, HORRIBLY, UNSAFE FROM
			// A SECURITY STANDPOINT !!!.
			char *tmpfile = tempnam(NULL, NULL);
			std::string cmd = std::string("perl > '") + tmpfile + "'";
			FILE *pipe = popen(cmd.c_str(), "w");
			ASSERT(pipe != NULL);
			fprintf(pipe, "use Math::Trig; ");
			fprintf(pipe, "print (");
			fwrite(expr.c_str(), expr.size(), 1, pipe);
			fprintf(pipe, ");");
			pclose(pipe);

			// load the result
			std::ifstream res(tmpfile);
			std::string line;
			expr.clear();
			while(std::getline(res, line))
			{
				if(!expr.empty()) { expr += "\n"; }
				expr += line;
			}

			// clean up
			unlink(tmpfile);
			free(tmpfile);

			// replace with evaulation result
			value.replace(idx, len, expr);

			//std::cout << "\tvalue=" << value << "\n";
		};
	}
	//std::cout << "\tvalue=" << value << "\n";

	if(key.size())
	{
		(*expanded)[key] = true;
		h[key] = value;
	}

	//std::cout << "\t---\n";
	return value;
}

bool util::expand_dict(std::map<std::string, std::string> &h, bool allowEnvironmentVariables)
{
	//std::cout << "\n\nexpanding variables\n";

	std::map<std::string, bool> expanded;
	typedef std::map<std::string, std::string> m_t;
	FOREACH2(m_t::iterator, h)
	{
		if(!expanded.count((*i).first))
		{
			expand_variable((*i).second, h, allowEnvironmentVariables, (*i).first, &expanded);
		}
		//std::cout << (*i).first << " -> " << (*i).second << "\n";
	}
	return true;
}

std::string util::expand_text(std::string text, std::map<std::string, std::string> &dictionary, bool allowEnvironmentVariables)
{
	return expand_variable(text, dictionary, allowEnvironmentVariables);
}
