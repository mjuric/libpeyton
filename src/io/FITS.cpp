/***************************************************************************
                          FITS.cpp  -  description
                             -------------------
    begin                : Sun Nov 10 2002
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_PKG_CCfits

#include <astro/io/fits.h>
#include <astro/util.h>
#include <astro/system/log.h>
#include <astro/io/gzstream/fstream.h>

#include <string>
#include <CCfits/CCfits>
#include <memory>
#include <unistd.h>

using namespace peyton::exceptions;
using namespace peyton::io;
using namespace CCfits;

void fits::write(peyton::image::Image &img, std::string file)
{
	DEBUG(verbose) << "io::fits: Writing [" << file << "] ...";

	long naxis = 2;
	long naxes[2] = { img.width(), img.height() };

	std::auto_ptr<CCfits::FITS> pFits(0);

	try {
		unlink(file.c_str());
		pFits.reset(new FITS(file, FLOAT_IMG, naxis, naxes));
	} catch(FITS::CantCreate) {
		THROW(EAny, "Cannot create output FITS file");
	}

	std::valarray<double> data((double *)img, img.size());
	pFits->pHDU().write(1, img.size(), data);

	DEBUG(verbose) << "io::fits: ... done";
	
//	std::cout << pFits->pHDU() << std::endl;
}

void fits::write(std::valarray<float> &img, int width, int height, std::string file,
	const keywords *kwords)
{
	DEBUG(verbose) << "io::fits: Writing [" << file << "] ...";

	long naxis = 2;
	long naxes[2] = { width, height };

	std::auto_ptr<CCfits::FITS> pFits(0);

	try {
		unlink(file.c_str());
		pFits.reset(new FITS(file, FLOAT_IMG, naxis, naxes));
	} catch(FITS::CantCreate) {
		THROW(EAny, "Cannot create output FITS file");
	}

	pFits->pHDU().write(1, img.size(), img);

	if(kwords)
	{
		FOREACH(keywords::const_iterator, *kwords) { pFits->pHDU().addKey((*i).first, (*i).second.first, (*i).second.second); }
	}

	DEBUG(verbose) << "io::fits: ... done";

//	std::cout << pFits->pHDU() << std::endl;
}

void fits::read(std::valarray<float> &img, int &width, int &height, std::string file,
	keywords *kwords)
{
	std::auto_ptr<FITS> pInfile(new FITS(file,Read,true));

	PHDU& image = pInfile->pHDU(); 

	// read all user-specifed, coordinate, and checksum keys in the image
	image.readAllKeys();

	image.read(img);

//	std::cout << image << std::endl;

	width = image.axis(0);
	height = image.axis(1);
	
	if(kwords)
	{
		std::string v;
		typedef std::map< string, Keyword *> sK_t;
		FOREACH(sK_t::iterator, image.keyWord())
		{
			Keyword &kw = *(*i).second;
			(*kwords)[kw.name()] = make_pair(kw.value(v), kw.comment());
		}
	}
}

#endif // HAVE_PKG_CCfits
