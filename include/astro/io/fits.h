/***************************************************************************
                          fits.h  -  description
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

#include <astro/image.h>
#include <valarray>
#include <map>

namespace peyton {
/// Classes and functions for I/O of data in various formats
namespace io {
/**
	\brief FITS input/output. Requires CCfits and cfitsio libraries.

	When using the functions from peyton::io::fits namespace make sure to link with
	CCfits and cfitsio libraries.

	CCfits can be found at http://heasarc.gsfc.nasa.gov/docs/software/fitsio/CCfits/ \n
	cfitsio can be found at http://heasarc.gsfc.nasa.gov/fitsio/
	
	Both can be found precompiled and ready to use in my LFS directory.
	
	\todo Explain what's my LFS directory.
*/
namespace fits {
	typedef std::pair<std::string, std::string> value_comment_pair;
	typedef std::map<std::string, value_comment_pair> keywords;

	/**
		\brief Write and image \a img to file \a file.
		\deprecated We don't like the peyton::image::Image class any more, so we don't like this function as well.
	*/
	void write(peyton::image::Image &img, std::string file);

	/**
		\brief Write a 2D FITS image.
		
		Image is stored in row-major order in \a img. Use indexers for easy access to image information.
		
		\sa astro/image/indexers.h
	*/
	void write(std::valarray<float> &img, int width, int height, std::string file, const keywords *header = NULL);

	/**
		\brief Write a 2D FITS image
		
		This template accepts anything that confirms to Image concept, unpacks it and forwards to one of
		the other write functions.
		
		Image concept requires:
			-# typename Image::value_type
				- scalar type which makes up the image (eg., \c float)
			-# typename Image::array_type
				- array type which is the actual data container (eg., \c std::valarray<float>)
			-# Image::x()
				- horizontal image dimension
			-# Image::y()
				- vertical image dimension
			-# operator Image::array_type &()
				- cast operator to reference of array_type

		\bug Image concept is in vewy vewy eawly alpha phase, hehehehe...
	*/
	template<typename Image>
	void write(Image &img, const std::string &file, const keywords *header = NULL)
	{
		write((typename Image::array_type &)img, img.x(), img.y(), file, header);
	}

	/**
		\brief Read a 2D FITS image.
		
		Image is stored in row-major order in \a img. Use indexers for easy access to image information.
		
		\sa astro/image/indexers.h
	*/
	void read(std::valarray<float> &img, int &width, int &height, std::string file, keywords *header = NULL);

	/**
		\brief Read a 2D FITS image into and Image concept conforming class.
	*/
	template<typename Image>
	void read(Image &img, const std::string &file)
	{
		read((typename Image::array_type &)img, img.x(), img.y(), file);
	}

}
}
}

#define __peyton_io_fits peyton::io::fits
