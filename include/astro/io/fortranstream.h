/***************************************************************************
                          io.h  -  description
                             -------------------
    begin                : Sat Nov 16 2002
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

#ifndef __astro_fortran_io_h
#define __astro_fortran_io_h

#include <astro/system/exceptions.h>

#include <iostream>
#include <valarray>

namespace peyton {
namespace io {

//namespace __fio_impl {
//	// don't want to pollute the main namespace with aux. functions
//}

/**
	\brief Allows reading of files written using FORTRAN unformatted output
	
	\todo Add more detailed docs and an example.
*/
class fortranstream {
private:
	// this used to be in __fio_impl namespace
	template<int N> inline void swapBytes(char *c) { ASSERT(0); /* This swapfunction not implemented yet */ }
	template<>      inline void swapBytes<4>(char *c) { std::swap(c[0], c[3]); std::swap(c[1], c[2]); }

	template<typename T> inline void swap(T &v) { swapBytes<sizeof(T)>((char *)&v); }
protected:
	std::istream &f;
	int nread, nreadExpected;
protected:
	template<typename T>
	inline void directRead(T *data, const unsigned n) {
		const int size = n*sizeof(T);
		f.read((char *)data, size);
		FOR(0, n) { __fio_impl::swap(data[i]); }
		nread += size;
	}

	inline int recordSize() {
		int size;
		f.read((char *)&size, sizeof(int)); __fio_impl::swap(size);
		return size;
	}

	template<typename T>
	inline void read(T *data, const unsigned n)
	{
		if(!nread) { nreadExpected = recordSize(); }
		directRead(data, n);
	}

	/// Internal class.
	class transient {
	protected:
		fortranstream &parent;
		bool used;
	public:
		transient(fortranstream *p) : parent(*p), used(false) {}
		transient(const transient &t) : parent(t.parent), used(false) {}
	public:
		template<typename T> transient operator >>(T &i) {
			used = true;
			return parent >> i;
		}
		~transient() {
			if(!used) {
				size_t rs = parent.recordSize();
				if(rs == parent.nread && rs == parent.nreadExpected) {
					parent.nread = 0;
					return;
				}

				THROW(EAny, "Incomplete record [expected=" + str(parent.nreadExpected) +
					", nread=" + str(parent.nread) +
					", recordsize=" + str(rs) +
					"]");
			}
		}
	};

public:
	fortranstream(std::istream &i) : f(i), nread(0) {};

	template<typename T>
	transient operator >>(T &i) {
		read(&i, 1);
		return transient(this);
	}

	template<typename T>
	transient operator >>(std::valarray<T> &v) {
		T *data = NULL;
		try {
			data = new T[v.size()];
			read(data, v.size());
			FOR(0, v.size()) { v[i] = data[i]; }
			delete [] data;
		} catch(...) {
			delete [] data;
			throw;
		}
		return transient(this);
	}
};

} // namespace io
} // namespace peyton

#define __peyton_io peyton::io

#endif
