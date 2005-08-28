/***************************************************************************
                        fpnumber.cpp  -  description
                             -------------------
    copyright            : (C) 2005 by Mario Juric
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

#include <astro/io/fpnumber.h>
#include <astro/io/iostate_base.h>

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////////////////////
// Low level bit pack/unpack functions
//////////////////////////////////////////////////////////////////////////////////////////////

namespace peyton {
namespace io {

void bitprint(unsigned x)
{
	for(int i = 32; i > 0; i--)
	{
		if(i == 14) { std::cerr << "|"; }
		std::cerr << bool(x & (1 << (i-1)));
	}
	std::cerr << " ";
}

double bitunpack(int m, int k, unsigned v)
{
	if(k != 0)
	{
		unsigned mint = v << (32-m) >> (32-m);
		
		int exp;
		double mantissa = frexp(mint, &exp);
		exp = -(v >> m);
	
		return ldexp(mantissa, exp);
	} else {
		double x = 0;
		for(int i = 0; i != m; i++)
		{
			x += (v & 1);
			x = ldexp(x, -1);
			v >>= 1;
		}
		return x;
	}
}

unsigned bitpack(int m, int k, double x)
{
	unsigned data;
	if(k != 0)
	{
		int exp;

		double mantissa = frexp(x, &exp);
		int mint = (int)rint(ldexp(mantissa, m));

		// error checking
		exp *= -1;
		if(exp < 0)  { throw 0; } // only non-positive exponents (|x| <= 1)
		if(mint < 0) { throw 0; } // only non-negative numbers (x >= 0)

		// range checking
		if(exp > k) { throw 0; } // smaller than our precision, set to 0

		// storage
		data = (exp << m) | (mint);
	} else {
		if(x < 0) { throw 0; } // only non-negative numbers.
		if(x >= 1.) { throw 0; } // denormalized numbers have to be < 1
		data = 0;
		for(int i = 0; i != m; i++)
		{
			data <<= 1;
			x = ldexp(x, 1);
			if((int)x)
			{
				data |= 1;
				x -= 1.;
			}
		}
		// rounding
		if(x >= 0.5)
		{
			data += 1; // remarkably, but in this representation, this is all it takes!
			if(data & (1 << m)) { throw 0; } // overflow
		}
	}

	return data;
}

//////////////////////////////////////////////////////////////////////////////////////////////

class fpnumber::iostate : public iostate_base<fpnumber::iostate>
{
public:
	const unsigned unit;		// size, in bits, of bitbuffer type (for unsigned, 32)
	typedef unsigned block;
	block bitbuffer[2];
	int ptr;

	std::ostream *out;

	int fmtflags;
	fpnumber::format f;		// format specification of the floating point number
public:
	// low-level read/write
	void write(unsigned len);
	block read(std::istream &in);

	// sanity checking
	std::streampos putptr;
	void check_putptr();
public:
	iostate(fpnumber::format f_ = format(12, 2), int fmt_ = 0)
		: out(NULL), f(f_), fmtflags(fmt_), ptr(0), putptr(-1), unit(8*sizeof(unsigned))
	{
		for(int i = 0; i != sizeof(bitbuffer); i++) { bitbuffer[i] = 0; }
	}
	iostate(const iostate &cpy)
		: out(NULL), f(cpy.f), fmtflags(cpy.fmtflags), ptr(0), putptr(-1), unit(8*sizeof(unsigned))
	{
		for(int i = 0; i != sizeof(bitbuffer); i++) { bitbuffer[i] = 0; }
	}
	void flush();
public:
	// format flags and interface
	static const int binary;
	
	int setf(int flag) { return fmtflags |= flag; }
	int unsetf(int flag) { return fmtflags &= ~flag; }

	// public interface
	void bitwrite(std::ostream &o, const double x);
	double bitread(std::istream &i);
	void mark();

	~iostate();
};

// flags
const int fpnumber::iostate::binary = 0x01;

//////////////////////////////////////////////////////////////////////////////////////////////
// fpnumber::iostate implementation
//////////////////////////////////////////////////////////////////////////////////////////////

unsigned lshift(unsigned x, int sh)
{
	if(abs(sh) >= 32) return 0;

	return sh >= 0 ? x << sh : x >> -sh;
}

void fpnumber::iostate::write(unsigned len) // len is the number of units to write
{
	check_putptr();

	if(fmtflags & iostate::binary)
	{
		out->write((char *)bitbuffer, len*unit/8);
	}
	else
	{
		for(unsigned i = 0; i != len*unit; i++) // iterate over bits
		{
			unsigned v = bitbuffer[i/unit];
			*out << bool(v & (1 << (unit - 1 - i % unit)));
		}
	}

	putptr = out->tellp();
}

fpnumber::iostate::block fpnumber::iostate::read(std::istream &in)
{
	if(fmtflags & iostate::binary)
	{
		block v;
		in.read((char *)&v, sizeof(block));
		return v;
	}
	else
	{
		// read in 1s and 0s, ignoring everything else
		char c;
		block v = 0;
		int i;
		for(i = 0; !in.eof() && i != unit; )
		{
			in >> c;
			if(c == '0')
			{
				i++;
			}
			else if (c == '1')
			{
				v |= 1 << ((unit-1) - i);
				i++;
			}
		}
		if(i != unit) { abort(); }
		return v;
	}
}

void fpnumber::iostate::flush()
{
	const int n = ptr / unit;
	if(n == 0) return;
	write(n);

	memset(bitbuffer, 0, n * (unit/8));
	bitbuffer[0] = bitbuffer[n];

	ptr = ptr % unit;
}

void fpnumber::iostate::check_putptr()
{
	std::streampos at = out->tellp();
	if(putptr != at)
	{
		std::cerr << "You wrote something to the stream, without calling fpnumber::flush() first!\n";
		abort();
	}
}

/// interface functions

void fpnumber::iostate::bitwrite(std::ostream &o, const double x)
{
	out = &o;
	if(putptr == std::streampos(-1)) { putptr = out->tellp(); }

	unsigned data = bitpack(f.m, f.k, x);
	unsigned n = (ptr % unit);
	int at = ptr / unit;
	bitbuffer[at]   |= lshift(data, unit - (n+f.l));
	bitbuffer[at+1]  = lshift(data, 2*unit - (n+f.l));
	ptr += f.l;

	if((sizeof(bitbuffer)/4-1)*unit <= ptr)
	{
		flush();
	}
}

double fpnumber::iostate::bitread(std::istream &i)
{
	// ptr here is in [0, unit) range (that is the way we read data)
	// ptr == 0 means there's no data at all in the buffer
	int e;

	unsigned v = 0;
	if(ptr != 0)
	{
		e = ptr + f.l - unit; // how much bits we extend into the next unit

		v = bitbuffer[0] << ptr >> ptr; // clean up higher significance bits
		v = lshift(v, e);
	} else {
		e = f.l;
	}

	// see if we have enough data in the buffer to decode
	if(e > 0)
	{
		bitbuffer[0] = read(i);
		v |= bitbuffer[0] >> (unit-e);
	}

	ptr = e > 0 ? e : unit + e;
	return bitunpack(f.m, f.k, v);
}

void fpnumber::iostate::mark()
{
	flush();

	if(ptr != 0)
	{
		write(1); // write the last, incomplete, unit
		bitbuffer[0] = 0;
		ptr = 0;
	}
}

inline fpnumber::iostate::~iostate()
{
	// check that we've flushed all of our stuff to the stream
	// the user must send a fpnumber::mark() when he's finished
	// writing the numbers
	if(out && ptr != 0)
	{
		std::cerr << "You are closing the stream, without calling fpnumber::mark() first (sorry that this cannot be made automatic :( )!\n";
		abort();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// fpnumber operators and manipulators (the public interface)
//////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator <<(std::ostream &out, const fpnumber &x)
{
	fpnumber::iostate &ios = fpnumber::iostate::get_iostate(out);
	ios.bitwrite(out, x);
	return out;
}

std::ostream &operator <<(std::ostream &out, const fpnumber::format &f)
{
	fpnumber::iostate &ios = fpnumber::iostate::get_iostate(out);
	ios.f = f;
	return out;
}

std::istream &operator >>(std::istream &in, fpnumber &x)
{
	fpnumber::iostate &ios = fpnumber::iostate::get_iostate(in);
	x = ios.bitread(in);
	return in;
}

std::istream &operator >>(std::istream &in, const fpnumber::format &f)
{
	fpnumber::iostate &ios = fpnumber::iostate::get_iostate(in);
	ios.f = f;
	return in;
}

std::ostream &fpnumber::mark(std::ostream &out)
{
	fpnumber::iostate &ios = fpnumber::iostate::get_iostate(out);
	ios.mark();
	return out;
}

std::ostream &fpnumber::binary(std::ostream &out)
{
	fpnumber::iostate &ios = fpnumber::iostate::get_iostate(out);
	ios.setf(iostate::binary);
	return out;
}

std::ostream &fpnumber::text(std::ostream &out)
{
	fpnumber::iostate &ios = fpnumber::iostate::get_iostate(out);
	ios.unsetf(iostate::binary);
	return out;
}

std::istream &fpnumber::mark(std::istream &in)
{
	// do nothing, mark is unneeded when reading - it's here just for consistency with ostream version
	return in;
}

std::istream &fpnumber::binary(std::istream &in)
{
	fpnumber::iostate &ios = fpnumber::iostate::get_iostate(in);
	ios.setf(iostate::binary);
	return in;
}

std::istream &fpnumber::text(std::istream &in)
{
	fpnumber::iostate &ios = fpnumber::iostate::get_iostate(in);
	ios.unsetf(iostate::binary);
	return in;
}

}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Test main() function
//
// To test, compile using something like:  icc -Dmain_fpnumber=main fpnumber.cpp
//////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace peyton::io;

int main_fpnumber(int argc, char**argv)
{
//	ofstream fs("output.dat", ofstream::binary);
//#define fs cout
	ostringstream fs;
//	fs << fpnumber::binary << fpnumber::format(14, 0);

	double var[6] = { 0.2, 0.5, 0.25, 0.125, 0.75, 0.2};
	for(int i = 0; i != 5; i++) { fs << fpnumber(var[i]); }
	fs << fpnumber::mark;

	std::string bits = fs.str();
	cerr << "bits: ["  << bits << "] [len = " << bits.size() << "]\n";
	istringstream is(bits);
//	is >> fpnumber::binary >> fpnumber::format(14, 0);
	fpnumber x;
	for(int i = 0; i != 5; i++) { is >> x; cerr << (double)x << " "; }
	is >> fpnumber::mark;
	cerr << "\n";

	return 0;
}
