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

#include <astro/ui/ticker.h>
#include <iostream>

namespace peyton {
namespace ui {

ticker::ticker(int step_) : tickk(-1) { open("", step_); }
ticker::ticker(const std::string &title, int step_) : tickk(-1) { open(title, step_); }
ticker::~ticker() { close(); }

void ticker::open(const std::string &title, int step_)
{
	close();
	
	step = step_;
	tickk = 0;

	if(title.size())
	{
		std::cerr << title << "...\n";
	}
}

void ticker::close()
{
	if(tickk > 0)
	{
		std::cerr << " [" << tickk << "].\n";
	}
	tickk = 0;
}

int ticker::count() const { return tickk; }

void ticker::tick()
{
	tickk++;
	if(tickk % step == 0) { std::cerr << "#"; }
	if(tickk % (step*50) == 0) { std::cerr << " [" << tickk << "]\n"; }
}

void ticker::push_back(const int &s) { tick(); }

}
}
