/***************************************************************************
                          Options.h  -  description
                             -------------------
    begin                : Tue Nov 19 2002
    copyright            : (C) 2002 by Mario Juric
    email                : mjuric@astro.princeton.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __astro_system_Options_h
#define __astro_system_Options_h

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <getopt.h>

struct Opt : public option {
public:
	std::auto_ptr<Opt> next;
	std::string def; bool has_def;
	enum { noArgs = 0, mustArgs = 1, mayArgs = 2 };
public:
	Opt(const char *longName, const char shortName, int args, const char *dflt, Opt *next = NULL);
};

class Options : std::map<std::string, std::string> {
public:
	std::map<std::string, bool> options;
public:
	void get(int argc, char **argv, Opt *options);
	Options() {}
	
	bool found(std::string o) { return options.find(o) != options.end(); }

	double real(const std::string s) { return atof((*this)[s].c_str()); }
	int integer(const std::string s) { return atoi((*this)[s].c_str()); }
	bool boolean(const std::string s) { return integer(s) != 0; }
	std::string string(const std::string s) { return (*this)[s]; }
};

//////////////////  Implementation

Opt::Opt(const char *longName, const char shortName, int args, const char *dflt, Opt *nxt)
	: next(nxt)
{
	name = longName;
	has_arg = args;
	flag = NULL;
	val = shortName;

	if(dflt) { def = dflt; has_def = true;}
	else { has_def = false; }
}

void Options::get(int argc, char **argv, Opt *op)
{
	std::auto_ptr<Opt> autodelete(op);

	const Opt *o = op;
	std::vector<option> opts;

	std::map<char, int> short2long;

	std::string optstring;
	while(o != NULL) {
		if(o->val) {
			optstring += o->val;
			if(o->has_arg) optstring += ':';
			short2long[(char)o->val] = opts.size();
		}
		if(o->has_def) {
			(*this)[o->name] = o->def;
		}
		opts.push_back(*o);
		o = o->next.get();
	}
	opts.push_back(Opt(NULL, 0, false, 0));

//	FOREACH(short2long) { cout << (*i).first << " -> " << (*i).second << "\n"; }

	int index = -1;
	char c;
	while((c = getopt_long(argc, argv, optstring.c_str(), &(*opts.begin()), &index)) != -1) {
		if(c == '?') { DEBUG(verbose, "Option [" << optopt << "] unknown!"); continue; }

		if(c != 0) {
			if(short2long.find(c) == short2long.end()) { DEBUG(verbose, "Option without a long name ?!") };
			index = short2long[c];
		}

		options[opts[index].name] = true;
		if(optarg) { (*this)[opts[index].name] = optarg; }
	}
}

#define __peyton_system peyton::system

#endif
