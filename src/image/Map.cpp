/***************************************************************************
                          Map.cpp  -  description
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

#include <astro/image/map.h>

#include <list>
#include <astro/util.h>
#include <astro/math/vector.h>
#include <astro/system/error.h>
#include <astro/system/log.h>
#include <string>
#include <algorithm>

using namespace peyton::math;
using namespace peyton::image;

const double epsilon = 1E-10;

inline V2 flip(const V2 v) { return V2(v[1], -v[0]); }

void Poly::push_back(V2 v)
{
	if(vs.empty())	{ xmin = xmax = v.x; }
	else 		{ xmin = std::min(xmin, v.x); xmax = std::max(xmax, v.x); }

	vs.push_back(v);
}

void Poly::deintegerize()
{
	FOREACHj(Vertices::iterator, v, vs) {
		if((*v).x == rint((*v).x)) {
			double w = xmax - xmin;
			(*v).x += epsilon * w;
//			Log::debug(Log::verbose, "Deintegerized (%f,%f)", (*v).x, (*v).y);
		}
	}
}

Poly Poly::unflip()
{
	Poly p;
	FOREACHj(Vertices::iterator, v, vs) { p.push_back(V2(-(*v).y, (*v).x)); }
	return p;
}

double Poly::area() const
{
	double sum = 0;
	Vertices::const_iterator j, i;
	for(j = vs.begin(), i = j++; j != vs.end(); j++, i++) {
		sum += (*i).x * (*j).y - (*j).x * (*i).y;
	}
	j = vs.begin();
	sum += (*i).x * (*j).y - (*j).x * (*i).y;

	return sum/2.;
}

#define v (*vit)
#define v0 (*vit0)

inline double calcY(
	const Vertices::iterator vit, const Vertices::iterator vit0,
	const double x,
	V2 &a, V2 &b
) {
	ASSERT(v.x != v0.x);
	return v0.y + (v.y - v0.y)/(v.x - v0.x)  *  (x - v0.x);
}

inline void insideInterval(const Vertices::iterator vit, const Vertices::iterator vit0, const double x,
	Vertices::iterator &ea, Vertices::iterator &eb, V2 &a, V2 &b)
{
	if (v0.x <= x && x < v.x) { eb = vit0; b.x = x; b.y = calcY(vit, vit0, x, a, b); }
	else if (v.x <= x && x < v0.x) { ea = vit0; a.x = x; a.y = calcY(vit, vit0, x, a, b); }
}

inline Poly Poly::cutMyself()
{
	// calculate y bounds
	Poly out;
	FOREACHj(Vertices::iterator, vit, vs) { out.push_back(flip(v)); }
	vs.clear();
	return out;
}

Poly Poly::cutflip(int x)
{
	if(x > (int)xmax) { return cutMyself(); }

	// find the edges we intersect
	V2 a, b;
	typeof(vs.begin()) vit = vs.begin(), vit0 = vit++, ea = NULL, eb = NULL;
	for(; vit != vs.end() && (ea == NULL || eb == NULL); vit++, vit0++) {
		insideInterval(vit, vit0, x, ea, eb, a, b);
	}
	vit = vs.begin();
	insideInterval(vit, vit0, x, ea, eb, a, b); // close the loop

	// check for degenerate single-V2 ending cases
	if(ea == NULL && eb == NULL) {
		char tmp[1000];
//		DEBUG(verbose) << "Single V2 ending in poly [" << *this << "]";
		return cutMyself();
	}

	// insert the V2s & cut the polygon
	Poly out;
	out.push_back(flip(b));
	out.push_back(flip(a));

	++ea; ++eb; // ea now V2s to the first V2 of left poly, eb to the first V2 of the right poly
	if(eb == vs.end()) eb = vs.begin();
	if(ea == vs.end()) ea = vs.begin();
	for(Vertices::iterator i = ea; i != eb;) {
		out.vs.push_back(flip(*i));

		vs.erase(i++);

		if(i == vs.end()) i = vs.begin();
	}
	vs.insert(eb, a);
	vs.insert(eb, b);

	// recalculate bounds
	xmin = x;

	return out;
}

void processPoly(Poly &base)
{

	DEBUG(verbose) << "input base : " << base << "\n";

	int i, j;
	double mass = 0;
	double s0 = base.area(), sx = 0;
	base.deintegerize();
	for(i = (int)base.xmin + 1; !base.vs.empty(); i++) {
		Poly left = base.cutflip(i);
		DEBUG(verbose) << "--- i = " << i;
		DEBUG(verbose) << "left  : " << left.unflip();
//		cout << "base  : " << base << "\n";
//		cout << "ranges : " << left.xmin << "," << left.xmax << "\n";
		sx += left.area();
		double sy = 0;
		left.deintegerize();
		for(j = (int)left.xmin + 1; !left.vs.empty(); j++) {
			Poly bottom = left.cutflip(j);
			DEBUG(verbose) << "   ---- j = " << j << "\n";
			DEBUG(verbose) << "   bottom : " << bottom.unflip().unflip() << "\n";
			DEBUG(verbose) << "   base   : " << left << "\n";
//			mass += rho(i, j) * bottom.area();
			sy += bottom.area();
		}
		DEBUG(verbose) << "   mass : " << sy << "\n";
	}
	DEBUG(verbose) << "   mass : " << sx;
}

#ifdef __TRANSFORM_TEST

int main()
{
for(int i = 0; i != 10000; i++) {
	Poly base;

/*	base.push_back(V2(0, 0));
	base.push_back(V2(1, 0));
	base.push_back(V2(1, 1));
	base.push_back(V2(0, 1));
*/

	base.push_back(V2(2.7, 2.5));
	base.push_back(V2(0.1, 2.5));
	base.push_back(V2(1.1, 1.2));
	base.push_back(V2(3.5, 1.2));


/*	base.push_back(V2(2.0125, -3));
	base.push_back(V2(1.2, -3));
	base.push_back(V2(1.2, -3.5));
*/

/*	base.push_back(V2(1, 1.1));
	base.push_back(V2(2, 1.1));
	base.push_back(V2(2, 1.2));
*/
	processPoly(base);
}
	return 0;
}

#endif
