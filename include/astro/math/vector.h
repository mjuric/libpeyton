/***************************************************************************
                          vector.h  -  description
                             -------------------
    begin                : Thu Nov 7 2002
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

#ifndef __astro_math_vector_h
#define __astro_math_vector_h

#include <astro/compat/compat.h>
#include <astro/system/error.h>
#include <astro/util.h>

#include <iosfwd>
#include <cmath>

//#define uBLAS

namespace peyton {
/// math related functionality (linear algebra, numerics, etc)
namespace math {

#ifdef uBLAS

	#include <boost/numeric/ublas/vector.hpp>

	#define nsBLAS boost::numeric::ublas
	#define VECTOR(T, D) nsBLAS::vector<T, nsBLAS::bounded_array<T, D> >

	typedef VECTOR(double, 3) V3;
	typedef VECTOR(double, 2) V2;
	typedef VECTOR(unsigned, 3) U3;
	typedef VECTOR(unsigned, 2) U2;

	template<class E>
	inline typename nsBLAS::vector_scalar_unary_traits<E, nsBLAS::vector_norm_1<typename E::value_type> >::result_type
	abs (const nsBLAS::vector_expression<E> &e) { return nsBLAS::norm_1(e); }

#else

//
//  General D-dim vector class
//

template<unsigned D> class Dtraits {};
template<> struct Dtraits<2> { typedef int dim_must_be_2; };
template<> struct Dtraits<3> { typedef int dim_must_be_3; };
template<> struct Dtraits<4> { typedef int dim_must_be_4; };

#define DIM(D) typename Dtraits<D>::dim_must_be_##D __dummy

/**
	\brief Define 'inherited' element-wise operator between a Vector<> and arbitrary scalar type T (helper macro)

	\param T Scalar operand type
	\param op Operator
*/
#define VECSCAT(T, op) \
	Vector &operator op##=(T f) { FOR(0, D) { this->val[i] op##= f; }; return *this; }
#define VECSCA(op) VECSCAT(T, op)

/**
	\brief Define 'inherited' element-wise operator between a Vector<> and arbitrary scalar type when used in binary form (helper macro)

	\param op Operator
*/
#define VECSCABIN(op) \
	template<typename T, unsigned D> \
	Vector<T, D> operator op(const Vector<T, D> &vv, const T &s) \
	{ Vector<T, D> v; FOR(0, D) { v[i] = vv[i] op s; }; return v; } \
	\
	template<typename T, unsigned D> \
	Vector<T, D> operator op(const T &s, const Vector<T, D> &vv) \
	{ Vector<T, D> v; FOR(0, D) { v[i] = s op vv[i]; }; return v; }
// #define VECSCABIN(op) \
// 	template<typename T, unsigned D, typename S> \
// 	Vector<T, D> operator op(const Vector<T, D> &vv, const S &s) \
// 	{ Vector<T, D> v; FOR(0, D) { v[i] = vv[i] op s; }; return v; } \
// 	\
// 	template<typename T, unsigned D, typename S> \
// 	Vector<T, D> operator op(const S &s, const Vector<T, D> &vv) \
// 	{ Vector<T, D> v; FOR(0, D) { v[i] = s op vv[i]; }; return v; }

/**
	\brief Define 'inherited' element-wise operator between two Vector<> classes (helper macro)

	\param op Operator
*/
#define VECVEC(op) \
	Vector operator op(const Vector &a) const { Vector v(*this); FOR(0, D) { v.val[i] op##= a.val[i]; }; return v; } \
	Vector &operator op##=(const Vector &a) { FOR(0, D) { this->val[i] op##= a.val[i]; }; return *this; }

/**
	\brief Define 'inherited' elementwise unary function (helper macro)
	
	Example might be \c round() which we want to generalize to work on vectors, elementwise.
*/
#define VECFUN(fun) \
	template<typename T, unsigned D> \
	Vector<T,D> fun(Vector<T, D> v) { typedef Vector<T, D> V; FOREACH(V, v) { *i = fun(*i); }; return v; }

/**
	\brief Define 'inherited' elementwise unary function which is in a namespace (helper macro)

	Example might be \c std::abs() which we want to generalize to work on vectors, elementwise.
	
	\param fun	Function name
	\param ns	Namespace
*/
#define VECFUNNS(fun, ns) \
	template<typename T, unsigned D> \
	Vector<T,D> fun(Vector<T, D> v) { typename Vector<T, D>::iterator i; for(i=v.begin(); i != v.end(); i++) { *i = ns::fun(*i); }; return v; }

/**
	\brief Define 'inherited' elementwise binary function which is in a namespace (helper macro)

	Example might be \c std::min(a, b) which we want to generalize to work on vectors, elementwise.
	
	\param fun	Function name
	\param ns	Namespace
	\param T2	Type of the second parameter
	\param i2	Indexer of the second parameter (eg, [i] if it's a vector, or nothing if it's a scalar)
*/
#define VECFUN2NS(fun, ns, T2, i2) \
	template<typename T, unsigned D> inline Vector<T, D> fun(const Vector<T, D> &a, T2 b) \
	{ Vector<T, D> r; FOR(0, D) { r[i] = ns::fun(a[i], b i2); }; return r; }

template<typename T, unsigned D> class Box;

/**
	\brief General storage template class for vector template.

	This template is specialized for 2D, 3D and 4D vectors to enable intuitive accessing
	using named components (eg. instead of v[0], you can write v.x)
	
	\param T Datatype of vector components
	\param D Vector dimension
*/
template<typename T, unsigned D> struct VectorStorage { T val[D]; };
template<typename T> struct VectorStorage<T, 2> { union { T val[2]; struct { T x, y;}; }; };
template<typename T> struct VectorStorage<T, 3> { union { T val[3]; struct { T x, y, z;}; }; };
template<typename T> struct VectorStorage<T, 4> { union { T val[4]; struct { T x, y, z, w;}; }; };

/**
	\brief Constant length vector template, suitable for linear algebra applications
	
	Usually you do not want to use Vector<> template directly, but use the typedefs for
	the most common datatypes.

	\param T Datatype of vector components
	\param D Vector dimension
	
	\sa V3
*/
template<typename T, unsigned D>
class Vector : public VectorStorage<T, D> {
public:
	typedef Vector<T, D> type;
	typedef T content_type;
	typedef T* iterator;
public:
	Vector() {}
	explicit Vector(const T a) { *this = a; }
	template<typename TT, unsigned DD> Vector<T, D>(const Vector<TT, DD> &v) { set(v); }

	// standard operators
	VECSCA(+); VECSCA(-); VECSCA(*); VECSCA(/); VECSCAT(unsigned, >>);
	VECVEC(+); VECVEC(-); VECVEC(*); VECVEC(/);

	Vector &operator=(const Vector &a)  { FOR(0, D) { this->val[i] = a.val[i]; }; return *this; }
	Vector &operator=(const T &a)       { FOR(0, D) { this->val[i] = a; };        return *this; }

	Vector operator-() const { Vector v(*this); FOR(0, D) { v[i] = -v[i]; }; return v; }

	bool operator==(const Vector &v) const { FOR(0, D) { if(v[i] != this->val[i]) return false; }; return true; }
	bool operator!=(const Vector &v) const { return !((*this) == v); }

	// member access
	T& operator[](int i) { return this->val[i]; }
	const T operator[](int i) const { return this->val[i]; }

	// geometry
	/// project vector \c *this along vector \c a
	T projectAlong(const Vector &a) { return dot(*this, a) / abs(a); }

	// vector -> vector asignment
	template<typename TT, unsigned DD>
	void set(const Vector<TT, DD> &v) {
		FOR(0, std::min(D, DD)) { this->val[i] = (T) v[i]; }
		FOR(std::min(D, DD), D) { this->val[i] = (T) 0; }
	}

	// STL-style iterators and functions
	static inline unsigned size() { return D; }
	T* begin() { return this->val; }
	T* end() { return this->val+D; }
	const T* begin() const { return this->val; }
	const T* end() const { return this->val+D; }

	// specializations for frequently used dimensions
	T phi() const { return atan2(this->y, this->x); }

	// 2D
	Vector(const T a, const T b) { DIM(2); set(a, b); }

	void set(T a, T b) { DIM(2); this->val[0] = a; this->val[1] = b; }
	void polar(double r, double phi) { DIM(2); set(r * cos(phi), r * sin(phi)); }

	// 3D
	Vector(const T a, const T b, const T c) { DIM(3); set(a, b, c); }

	void set(T a, T b, T c) { DIM(3); this->val[0] = a; this->val[1] = b; this->val[2] = c; }
	void spherical(T r, T theta, T phi) { DIM(3);
		const double rxy = sin(theta) * r;
		set(rxy * cos(phi), rxy * sin(phi), r * cos(theta));
	}
	void cylindrical(const T r, const T phi, const T z) { DIM(3);
		set(r * cos(phi), r * sin(phi), z);
	}
	void celestial(T r, T lon, T lat)
	{
		spherical(r, ctn::pi/2. - lat, lon);
	}

	// 3D cylindrical radius
	T rho() const { DIM(3); return sqrt(sqr(this->x)+sqr(this->y)); }

	T theta() const { DIM(3); return acos(this->z / abs(*this)); }
	T lat() const { DIM(3); return peyton::ctn::pi/2. - theta(); }

	// 4D
	Vector(const T a, const T b, const T c, const T d) { DIM(4); set(a, b, c, d); }
	void set(T a, T b, T c, T d) { DIM(4); this->val[0] = a; this->val[1] = b; this->val[2] = c; this->val[3] = d; }
};

// mathematical vector operations
template<typename T, unsigned D> inline double abs(const Vector<T, D> &a) { return sqrt(double(dot(a, a))); }
template<typename T, unsigned D> inline double sqr(const Vector<T, D> &a) { return double(dot(a, a)); }
template<typename T, unsigned D> inline T dot(const Vector<T, D> &a, const Vector<T, D> &b) { T sum = 0; FOR(0, D) { sum += a[i]*b[i]; }; return sum; }
template<typename T, unsigned D> inline T sum(const Vector<T, D> &a) { T sum = 0; FOR(0, D) { sum += a[i]; }; return sum; }

// cross product
template<typename T> inline Vector<T, 3> cross(const Vector<T, 3> &a, const Vector<T, 3> &b)
{
	return Vector<T, 3>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// standard binary memberwise operations
VECSCABIN(+); VECSCABIN(-); VECSCABIN(*); VECSCABIN(/); VECSCABIN(>>);

// memberwise function application
VECFUNNS(round, );	// global namespace (TODO: check if round is in std namespace)
VECFUNNS(ceil, );		// global namespace
VECFUNNS(floor, );	// global namespace
VECFUN2NS(pow, std, T, );
#define VTD	const Vector<T, D> &
VECFUN2NS(min, std, VTD, [i]);
VECFUN2NS(max, std, VTD, [i]);
#undef VTD

// statistical template functions
template<typename T>
void rand(T &v, double a, double b)	///< generate a vector of random numbers ranging from a to b
{
	typedef typename T::iterator it;
	for(it i = v.begin(); i != v.end(); i++) { *i = a + (b - a) * (float(std::rand())/RAND_MAX); }
}

#endif

// output operators
template<typename T, unsigned D>
std::ostream &operator <<(std::ostream &out, const Vector<T, D> &v)
{
	out << '[';
	FOR(0, D-1) { out << v[i] << ","; }
	return out << v[D-1] << ']';
}

template<typename T, unsigned D>
std::istream &operator >>(std::istream &in, Vector<T, D> &v)
{
	char c;
	in >> c;
	FOR(0, D-1) { in >> v[i] >> c; }
	return in >> v[D-1] >> c;
}

#ifndef uBLAS

//
// Helper typedefs and functions
//

typedef Vector<double, 2> V2;
typedef Vector<double, 3> V3;
typedef Vector<double, 4> V4;
typedef Vector<int, 2> I2;
typedef Vector<int, 3> I3;
typedef Vector<short, 3> S3;
typedef Vector<short, 2> S2;
typedef Vector<unsigned, 2> U2;
typedef Vector<unsigned, 3> U3;

#endif

} // namespace math
} // namespace peyton

#define __peyton_math peyton::math

#endif
