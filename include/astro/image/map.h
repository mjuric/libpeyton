/***************************************************************************
                          map.h  -  description
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

#ifndef __astro_image_map_h
#define __astro_image_map_h

#include <astro/math/vector.h>
#include <astro/image.h>
#include <astro/system/log.h>
#include <list>

namespace peyton {
namespace image {

//
//	Helper classes
//

typedef std::list<peyton::math::V2> Vertices;

class Poly {
public:
	Vertices vs;
	double xmin, xmax;
public:
	void push_back(peyton::math::V2 v);

	Poly cutflip(int x);
	double area() const;

	inline Poly cutMyself();
	void deintegerize();
	Poly unflip();
};

inline std::ostream &operator <<(std::ostream &o, const Poly &p) { FOREACHj(v, p.vs) o << *v << " "; o << '[' << p.area() << ']'; return o; }

template<typename T, typename MapSpecification>
T mapPixel(peyton::math::V2 pixel[4], MapSpecification &ms)
{
	Poly base;
	FOR(0,4) { base.push_back(pixel[i]); }

//	cout << "input base : " << base << "\n";
	ASSERT(base.area() >= 0);

	T mass(0.);
	base.deintegerize();
	for(int i = (int)base.xmin + 1; !base.vs.empty(); i++) {
		Poly left = base.cutflip(i);

//		cout << "left : " << left << "\n";
		ASSERT(left.area() >= 0);

		left.deintegerize();
		for(int j = (int)left.xmin + 1; !left.vs.empty(); j++) {
			Poly bottom = left.cutflip(j);

			ASSERT(bottom.area() >= 0);

//			cout << "   ---- j = " << j << "\n";
//			cout << "   bottom : " << bottom.unflip().unflip() << "\n";
//			cout << "   base   : " << left << "\n";
			mass = mass + ms(i, j) * bottom.area();
		}
	}
	return mass;
}

//
//	Public functions
//

template<typename T>
struct SafeMapSpecification {
protected:
	const ImageBase<T> &img;
	const T blank;
public:
	SafeMapSpecification(const ImageBase<T> &i, const T blank_ = T(0.)) : img(i), blank(blank_) {}

	// return the value of the requested pixel
	T operator()(int i, int j) const {
		if(i < 0 || j < 0 || i >= img.width() || j >= img.height()) { return blank; }
		return img(i, j);
	}

	// map pixel from destination image to original image
	// return value are pixel coordinates
	virtual peyton::math::V2 invMap(const peyton::math::V2 p) = 0;

	// volume transformation rho(new) = volumeFactor * rho(old)
	virtual double volumeFactor(const peyton::math::V2 p) = 0;
};


template<typename T, typename MapSpecification>
ImageBase<T> &mapTransform(
	ImageBase<T> &out,
	MapSpecification &ms,				// map specification
	peyton::math::V2 minn, peyton::math::V2 maxx,				// output rectangle
	double dx					// output scale factor
	)
{
	peyton::math::V2 size = round((maxx - minn) / dx);		// calculate image size
	maxx = minn + size*dx;				// adjust max values to rounded up versions

	out.resize(size);
	out.setOrigin(minn);
	out.setScale(dx);

	DEBUG(verbose, "map [ size = " << size << ", origin = " << minn << ", scale = " << dx << " ]");

	double offset[4][2] = { {0, 0}, {dx, 0}, {dx, dx}, {0, dx} };

	peyton::math::V2 r[4];
	for(int j = 0; j != int(size.y); j++) {
		const double y = minn.y + j * dx;

		for(int i = 0; i != int(size.x); i++) {
			const double x = minn.x + i * dx;

			FORj(k, 0, 4) { // map a destination pixel back to the originating image
				peyton::math::V2 p(x + offset[k][0], y + offset[k][1]);
				r[k] = ms.invMap(p);
			}

			out(i, j) = mapPixel<T, MapSpecification>(r, ms);			 // map the density into destination pixel
			out(i, j) = out(i, j) / ms.volumeFactor(peyton::math::V2(x + dx/2, y + dx/2)); // weighting of the space
		}
	}

	return out;
}

}
}

#define __peyton_image peyton::image

#endif
