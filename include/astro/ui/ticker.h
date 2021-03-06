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

#ifndef __astro_ui_h
#define __astro_ui_h

#include <iosfwd>
#include <string>

namespace peyton {
namespace ui {

class ticker {
public:
	int tickk;
	int step;

	typedef int value_type;
public:
	ticker(int step_);
	ticker(const std::string &title, int step_);
	~ticker();

	void open(const std::string &title, int step_);
	void close();
	
	
	int count() const;
	void tick();

	/// interface to make ticker compatible with bind() and load()	
	void push_back(const int &s);
};

} // namespace ui
} // namespace peyton

#define __peyton_ui peyton::ui

#endif
