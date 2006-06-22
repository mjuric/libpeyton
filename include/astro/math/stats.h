/***************************************************************************
 *   Copyright (C) 2004 by Mario Juric                                     *
 *   mjuric@astro.Princeton.EDU                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef lp__astro_math_stats_h
#define lp__astro_math_stats_h

#include <iterator>

namespace peyton {
namespace math {
namespace stats {


	
	template<typename IT>
	typename std::iterator_traits<IT>::value_type median_of_sorted(const IT &begin, const IT &end)
	{
		typedef typename std::iterator_traits<IT>::difference_type D;
		D dist = end - begin;
		if(dist % 2 != 0)
		{
			return *(begin + dist / 2);
		}
		D h = dist / 2;
		return (*(begin + h - 1) + *(begin + h)) / 2.;
	}
	
	template<typename IT>
	typename std::iterator_traits<IT>::value_type mean(const IT &begin, const IT &end)
	{
		typename std::iterator_traits<IT>::value_type sum(0);
		typename std::iterator_traits<IT>::difference_type n(0);
	
		for(IT i = begin; i != end; ++i)
		{
			sum += *i;
			++n;
		}
	
		return sum / n;
	}

} // namespace stats
} // namespace math
} // namespace peyton

#define __peyton_math_stats peyton::math::stats

#endif
