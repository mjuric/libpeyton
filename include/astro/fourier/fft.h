/***************************************************************************
                          fft.h  -  description
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

/*
	Using this header requires having fftw libraries and include files
	installed on your system
*/
 
#ifndef __astro_fourier_fft_h
#define __astro_fourier_fft_h

extern "C" {
	#include <rfftw.h>
	#include <fftw.h>
}

#include <astro/image.h>
#include <astro/system/log.h>

namespace Fourier {
	ComplexImage &transform(ComplexImage &img) {
		Log::debug(Log::verbose, "Forward FFT (%d, %d) [ real => complex ]", img.width(), img.height());
		fftwnd_plan plan = fftw2d_create_plan(img.height(), img.width(), FFTW_FORWARD, FFTW_IN_PLACE);
		fftwnd_one(plan, (fftw_complex *)(std::complex<double> *)img, (fftw_complex *)(std::complex<double> *)img);
		fftwnd_destroy_plan(plan);

		return img;
	}
	
	ComplexImage &transform(ComplexImage &img, Image &in) {
		copy(img, in);

		return transform(img);
	}

	Image &inverse(Image &out, ComplexImage &im) {
		ComplexImage img(im);

		Log::debug(Log::verbose, "Inverse FFT (%d, %d) [ complex => real ]", img.width(), img.height());
		fftwnd_plan plan = fftw2d_create_plan(img.height(), img.width(), FFTW_BACKWARD, FFTW_IN_PLACE);
		fftwnd_one(plan, (fftw_complex *)(std::complex<double> *)img, (fftw_complex *)(std::complex<double> *)img);
		fftwnd_destroy_plan(plan);

		copy(out, img);
		out *= 1./out.size();

		return out;
	}
}

#endif
