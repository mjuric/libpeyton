#ifndef _astro_mixed_h
#define _astro_mixed_h

#include <string>

/**
	\brief Simple implementation of a class with mixed datatype storage.
	
	This is a class only used internaly by peyton::system::Preferences and is deprecated.
	
	\deprecated This is an internal (and deprecated) class and should not be used.
*/
class Mixed {
public:
	int type;
	enum { tInt, tDouble, tString, tBool };
protected:
	union {
		int i;
		double d;
		bool b;
	} scalars;
	std::string str;

public:
	Mixed() { type = tInt; scalars.i = 0; }
	Mixed & operator=(const Mixed &m) {
		str = m.str;
		scalars = m.scalars;
	}
	operator const char*() { return str.c_str(); }
	operator int() { return scalars.i; }
	operator double() { return scalars.d; }

	int operator =(int iX) { scalars.i = iX; type = tInt; return scalars.i; }
	double operator =(double dX) { scalars.d = dX; type = tDouble; return scalars.d; }
	bool operator =(bool bX) { scalars.b = bX; type = tBool; return scalars.b; }
	const char * operator=(const char * cX) { str = cX; type = tString; return str.c_str(); }
};

#define __peyton_system peyton::system

#endif
