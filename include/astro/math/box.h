/***************************************************************************
                          box.h  -  description
                             -------------------
    begin                : Fri Nov 8 2002
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

#ifndef __astro_math_box_h
#define __astro_math_box_h
 
#include <astro/math/vector.h>

namespace peyton {
namespace math {

/**
	\brief	A simple representation of D-dimensional box, specified by top and bottom corners
	
	\param T Underlying scalar datatype
	\param D Dimensionality of the vectors
*/
template<typename T, unsigned D>
class Box {
public:
#ifdef uBLAS
	typedef boost::numeric::ublas::vector<T, boost::numeric::ublas::bounded_array<T, D> > V;
#else
	typedef Vector<T, D> V;
#endif
public:
	V b, ///< bottom corner of the box
	  t; ///< top corner of the box
public:
	Box() {}
	Box(V bottom, V top) : b(bottom), t(top) {}
	Box(const Box &bx) : b(bx.b), t(bx.t) {}
	Box(const T x) : b(0), t(x) {}

	Box &operator=(const Box &bx) { b = bx.b; t = bx.t; return *this; }
	Box &operator=(const T x) { b = 0; t = x; return *this; }
	operator bool() { return b != t; }

	V size() const { return t - b; }
	static unsigned dim() { return D; }

	bool contains(const V &v) const {
		FOR(0, D) { if(b[i] > v[i] || v[i] > t[i]) return false; }
		return true;
	}

	// modifiers
	Box &move(const V &r) { b += r; t += r; return *this; }
	/// Resize the box (if needed) to contain the vector \p v
	void accomodate(const V &v) {
		FOR(0, D) {
			if(b[i] > v[i]) b[i] = v[i];
			if(v[i] > t[i]) t[i] = v[i];
		}
	}
};

#ifndef uBLAS
/**
	\brief Reduce the given vector to be in the box (assume the space is periodic)
	
	\todo Better explain what this function does :).
*/
template<typename T, unsigned D>
inline Vector<T, D> modulo(Vector<T, D> v, const Box<T, D> &b)
{
	const Vector<T, D> size = b.size();
	FOR(0, b.dim()) {
		T l = v[i] - b.b[i];
		if(v[i] < b.b[i]) {
			v[i] += size[i] * (int(l / size[i]) + 1);
		} else {
			v[i] -= size[i] * int(l / size[i]);
		}
	}
	return v;
}
#endif

// IO operators
template<typename T, unsigned D> OSTREAM(Box<T, D> b) { return out << '(' << b.b << ", " << b.t << ')'; }
template<typename T, unsigned D> ISTREAM(Box<T, D> &b) { char c; in >> c >> b.b >> c >> b.t >> c; return in; }

typedef Box<double, 3> V3Box;
typedef Box<double, 2> V2Box;

} // namespace math
} // namespace peyton

#define __peyton_math peyton::math

#endif
