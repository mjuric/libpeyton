/***************************************************************************
                          kernels.h  -  description
                             -------------------
    begin                : Sat Nov 9 2002
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

#ifndef __astro_fourier_kernels_h
#define __astro_fourier_kernels_h

#include <astro/image.h>
#include <astro/constants.h>

namespace Kernels {
	void gaussian(Image &kernel, const double fwhm, const U2 dim, const double dx)
	{
 		const double sigma2 = sqr(fwhm / sqrt(8*log(2)));
		const double N = 1/(2*ctn::pi*sigma2);
		const double c = 1/(2*sigma2);

		kernel.resize(dim);

		Log::debug(Log::verbose, "Generating gaussian kernel [FWHM=%f, sigma=%f]", fwhm, sqrt(sigma2));

		double area = sqr(dx);
		V2 d(dim[0], dim[1]); d *= dx;
		V2 d2(d); d2 /= 2;		// center of the image, in physical units
		FORj(i, 0, dim[0]) {
			FORj(j, 0, dim[1]) {
				V2 r(i + .5, j + .5);
				r *= dx;			// center of the pixel, physical units
				if(r[0] > d2[0]) { r[0] -= d[0]; }
				if(r[1] > d2[1]) { r[1] -= d[1]; }
				double r2 = dot(r, r);
				double g = N*exp(-c*r2);	// probability density
				kernel(i, j) = g * area;	// gaussian value
			}
		}
	}
};

#endif
