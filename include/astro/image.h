/***************************************************************************
                          image.h  -  description
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

#ifndef __astro_image_h
#define __astro_image_h

#include <astro/math/vector.h>
#include <astro/exceptions.h>

#include <algorithm>
#include <cmath>
#include <complex>

namespace peyton {
/// Manipulation of N-dimensional data
namespace image {

/**
	\brief A class to store 2-dimensional data
	
	\deprecated This class has been deprecated. It's bloated and it's design is questionable.
		Use indexers and valarrays as a replacement
	
	\sa astro/image/indexers.h
*/
template<typename T>
class ImageBase
{
protected:
	T *data;
	int h, w;
	peyton::math::V2 o; // physical origin
	double dx; // physical scale of each pixel
public:
	typedef T* iterator;
	typedef const T* const_iterator;
public:
	void setOrigin(const peyton::math::V2 orig) { o = orig; }
	void setScale(double d) { dx = d; }
	operator T*() { return data; }
	ImageBase() : o(0, 0), dx(0) { h = 0; w = 0; data = NULL; }
	ImageBase(int w_, int h_) : o(0, 0), dx(0) { h = h_; w = w_; data = new T[size()]; }
	ImageBase(const peyton::math::U2 dim) : o(0, 0), dx(0) { h = dim[1]; w = dim[0]; data = new T[size()]; }
	ImageBase(const ImageBase &i) { data = NULL; *this = i; }
	ImageBase& operator=(const ImageBase &i) {
		beLike(i);
		std::copy(i.begin(), i.end(), begin());
		return *this;
	}
	ImageBase &beLike(const ImageBase &i) {
		// physics
		o = i.o; dx = i.dx;
		// storage
		resize(i.dim());

		return *this;
	}
	ImageBase& operator=(T x) { FOREACHj(i, *this) { *i = x; }; return *this; }
	~ImageBase() { delete [] data; }

	T &at(int x, int y) { ASSERT(x >= 0 && y >= 0 && x < w && y < h); return data[y*w + x]; }
	T  at(int x, int y) const { ASSERT(x >= 0 && y >= 0 && x < w && y < h); return data[y*w + x]; }
	T &operator()(int x, int y) { return at(x, y); }
	T  operator()(int x, int y) const { return at(x, y); }
	T &operator()(peyton::math::V2 p) { return (*this)(int(p.x), int(p.y)); }
	T  operator()(peyton::math::V2 p) const { return (*this)(int(p.x), int(p.y)); }
	int height() const { return h; }
	int width() const { return w; }
	peyton::math::V2 origin() const { return o; }
	double scale() const { return dx; }
	int size() const  { return h*w; }
	peyton::math::U2 dim() const { return peyton::math::U2(w, h); }

	peyton::math::V2 pixelCoords(peyton::math::V2 p) const { return peyton::math::V2((p.x - o.x) / dx, (p.y - o.y) / dx); }
	peyton::math::V2 realCoords(peyton::math::V2 p)  const { return peyton::math::V2(p.x * dx + o.x, p.y * dx + o.y); }

	iterator begin() { return data; }
	iterator end() { return data + size(); }
	const_iterator begin() const { return data; }
	const_iterator end() const { return data + size(); }

	bool resize(const peyton::math::V2 s) { return resize(int(rint(s.x)), int(rint(s.y))); }
	bool resize(const peyton::math::U2 d) { return resize(d.x, d.y); }
	bool resize(int w_, int h_) {
		if(h == h_ && w == w_ && data != NULL) return true;
		delete data; data = NULL;
		h = h_; w = w_;
		if(!size()) return false;
		data = new T[size()];
		return true;
	}

	template<typename Scalar> ImageBase &operator*=(Scalar c) { FOREACHj(i, *this) { *i *= c; }; return *this; }

	ImageBase &operator*=(ImageBase &b) {
		if(b.w != w && b.h != h) THROW(peyton::exceptions::EAny, "Images are not of the same size!");

		iterator o = begin();
		FOREACHj(i, b) {
			*o *= *i;
			o++;
		}

		return *this;
	}

	ImageBase &operator/=(ImageBase &b) {
		if(b.w != w && b.h != h) THROW(peyton::exceptions::EAny, "Images are not of the same size!");

		iterator o = begin();
		FOREACHj(i, b) {
			if(*i) { *o /= *i; } else { *o = 0; }
			o++;
		}

		return *this;
	}

	ImageBase &operator*=(T f) { FOREACH(*this) { *i *= f; }; return *this; }
	ImageBase &operator/=(T f) { FOREACH(*this) { *i /= f; }; return *this; }
	ImageBase &operator+=(T f) { FOREACH(*this) { *i += f; }; return *this; }
	ImageBase &operator-=(T f) { FOREACH(*this) { *i -= f; }; return *this; }

};

typedef ImageBase<double> Image;
typedef ImageBase<std::complex<double> > ComplexImage;

inline ComplexImage &copy(ComplexImage &out, Image &in)
{
	out.resize(in.dim());
	ComplexImage::iterator o = out.begin();
	FOREACHj(i, in) { (*o) = *i; o++; }
	return out;
}

inline Image &copy(Image &out, ComplexImage &in)
{
	out.resize(in.dim());
	Image::iterator o = out.begin();
	FOREACHj(i, in) { *o = (*i).real(); o++; }
	return out;
}

inline Image &abs(Image &out, ComplexImage &in)
{
	out.resize(in.dim());
	Image::iterator o = out.begin();
	FOREACHj(i, in) { *o = abs(*i); o++; }
	return out;
}

template<typename V>
class CoordinateIterator : public V {
protected:
	V b, t, s;
public:
	CoordinateIterator(V min, V max, V step) : b(min), t(max), s(step), V(min) { }
	CoordinateIterator &operator++() {
		V last = *this;
		FOR(0, dim()) {
			if(val[i] += s[i] < t[i]) return *this;
			val[i] = b[i];
		}
		*this = last;
		return *this;
	}
	operator bool() { return val[dim()-1] < t[dim()-1]; }
};

#define FORIMAGEr(img, i, j, i0, i1, j0, j1) \
	FORj(i, i0, i1) \
		FORj(j, j0, j1)

#define FORIMAGEj(img, i, j) FORIMAGEr(img, i, j, 0, img.width(), 0, img.height())

#define FORIMAGE(img) FORIMAGEj(img, i, j)

}
}

#define __peyton_image peyton::image
	
#endif
