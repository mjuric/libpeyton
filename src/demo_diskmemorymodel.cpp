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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>

#include <stdio.h>

#include <astro/system/memorymap.h>
#include <astro/exceptions.h>
#include <astro/util.h>
#include <astro/types.h>
#include <astro/system/log.h>
#include <astro/useall.h>

using namespace std;


struct myobject
{
	int idx;
	Radians ra, dec;
	double x, y, z;
};

void createhugefile(const std::string &s, int offset, int n, int &idx0)
{
	FILE *f;
	f = fopen(s.c_str(), "wb");

	int pagesize = getpagesize();
	if(offset % pagesize) { offset = pagesize * (1 + offset / pagesize); }

	char buf[offset];
	fwrite(buf, 1, offset, f);

	while(n)
	{
		myobject o = { idx0, 5, 5, 2, 3, 4 };
		fwrite(&o, sizeof(o), 1, f);
		idx0++;
		n--;
	}
	fclose(f);
}

void createfiles()
{
	int idx = 0;
	createhugefile("memory0.mem", 773, 47000000, idx);
	cout << "idx = " << idx << "\n";
	createhugefile("memory1.mem", 5573, 47000000, idx);
	cout << "idx = " << idx << "\n";
}

struct ordering
{
	bool operator()(const myobject a, const myobject b) const;
};

bool ordering::operator()(myobject a, myobject b) const { return b.idx < a.idx; }

int main_diskmemorymodel(int argc, char *argv[])
{
	try
	{
		cout << "Hello, world!" << endl;

//		createfiles();
//		return 0;

#if 0
		DiskMemoryModel smm;
		smm.setmode("r");
		smm.addfile("memory0.mem", sizeof(myobject)*47000000, 4096);
		smm.addfile("memory1.mem", sizeof(myobject)*47000000, 8192);

		smm.open();

#if 0
		// basic get tests
		long long offset = 55000000;
		offset *= sizeof(myobject);
		int len = sizeof(myobject);
		myobject *o = (myobject *)smm.get(offset, len);
		cout << o->idx << "\n";

		offset += sizeof(myobject)*2;
		o = (myobject *)smm.get(offset, len);
		cout << o->idx << "\n";
#endif

#if 0
		// sequential access test
		for(int i = 0; i != 55000000; i++)
		{
			offset = i; offset *= sizeof(myobject);
			o = (myobject *)smm.get(offset, len);
//			cout << o->idx << "\n";
			ASSERT(i == o->idx);
		}
#endif

#if 0
		// multi-sequential access test
		smm.setwindowsize(100*1024*1024);
		int offsets[] = {0, 1000000, 10000000, 60000000, 80000000};
		for(int i = 0; i != 10000000; i++)
		{
			for(int j = 0; j != 5; j++)
			{
				long long offset = offsets[j] + i; offset *= sizeof(myobject);
				myobject *o = (myobject *)smm.get(offset, sizeof(myobject));
//				cout << o->idx << "\n";
				ASSERT(offsets[j] + i == o->idx);
			}
		}
#endif

#if 0
		// random access test
		smm.setmaxwindows(50000);
		smm.setwindowsize(4096);
		for(int i = 0; i != 500000; i++)
		{
			int idx = (int)(double(rand()) / double(RAND_MAX) * double(94000000.));
			offset = idx;
			offset *= sizeof(myobject);
			o = (myobject *)smm.get(offset, len);
			ASSERT(idx == o->idx);
		}
#endif
#endif

#if 0
		// writable file test
		DiskMemoryModel smm;
		smm.setmode("rw");
		smm.addfile("memory0.mem", sizeof(myobject)*47000000, 4096);
		smm.addfile("memory1.mem", sizeof(myobject)*47000000, 8192);

		smm.open();

		// sequential access test
		for(int i = 0; i != 500; i++)
		{
			long long offset = i; offset *= sizeof(myobject);
			myobject *o = (myobject *)smm.get(offset, sizeof(myobject));
			o->ra = i;
//			cout << o->idx << "\n";
			ASSERT(i == o->idx);
		}
#endif

#if 0
		// new writable file test
		DiskMemoryModel smm;
		smm.addfile("memoryx0.mem", sizeof(myobject)*47000000, 4096);
		smm.addfile("memoryx1.mem", sizeof(myobject)*47000000, 8192);

		smm.open("rw");

#if 0
		// sequential write access test
		for(int i = 10000000; i != 11000000; i++)
		{
			long long offset = i; offset *= sizeof(myobject);
			myobject *o = (myobject *)smm.get(offset, sizeof(myobject));
			o->ra = i;
//			cout << o->idx << "\n";
//			ASSERT(i == o->idx);
		}
#endif

		// fill full files
		for(int i = 0; i != 94000000; i++)
		{
			long long offset = i; offset *= sizeof(myobject);
			myobject *o = (myobject *)smm.get(offset, sizeof(myobject));
			myobject mo = { i, 5, 5, 2, 3, 4 };
			*o = mo;
		}
#endif

#if 0
		// test DMMArray
		DMMArray<myobject> a;
		a.addfile("memory0.mem", 47000000, 4096);
		a.addfile("memory1.mem", 47000000, 8192);

#if 0
		a.open("r");
		cout << "Array size: " << a.size() << "\n";
		for(int i = 0; i != a.size(); i++)
		{
			myobject &o = a[i];
			ASSERT(o.idx == i);
		}
#endif

#if 1
		// test writability & iterators
		int n = 0;
		a.open("rw");
		FOREACH(a)
		{
			myobject &o = *i;
			myobject mo = { n, 5, 5, 2, 3, 4 };
			o = mo;
			n++;
		}
#endif

#endif

#if 0
	// file creation
	DMMArray<myobject> a;
	a.create("coolstuff.dmm", offset, length);

	// file opening for reading
	a.open("fark.dmm", "r");
	a.open("fark.dmm", "rw", true); // call create if it does not exist
	
	// truncating a file - this deletes all
	// physical files from the DMM set
	a.truncate();

	// capacity related actions on file
	a.capacity();
#endif

#if 1
		// test automatic generation of memory files
		DMMArray<myobject> a;
//		a.open("memory.dmm", "rw");
		a.create("memory.dmm");
		cout << "Size: " << a.size() << "\n";

//		a[55354].idx = 10;
//		a.sync();
//		cout << "Size: " << a.size() << "\n";

		vector<int> offsets;
		srand(time(NULL));
		const int l = 1000;
		FORj(k, 0, 20)
		{
			int i0 = 1+(int) (1000000000.0*rand()/(RAND_MAX+1.0));
			offsets.push_back(i0);
			for(int i = i0; i != i0 + l; i++)
			{
				myobject mo = { i, 5, 5, 2, 3, 4 };
				a[i] = mo;
			}
		}
		cout << "W = " << a.winopenstat << "\n";
		cout << "Capacity = " << a.capacity() << "\n";
		cout << "Size: " << a.size() << "\n";

		// verify
		cout << "Verifying\n";
		FOREACH(offsets)
		{
			int i0 = *i;
			for(int i = i0; i != i0 + l; i++)
			{
				ASSERT(a[i].idx == i);
			}
		}
#if 0
		cout << "Sorting...\n"; cout.flush();

		sort(a.begin(), a.end(), ordering());
		for(int i = 0; i != 50; i++)
		{
			myobject mo = a[i];
//			cout << mo.idx;
		}
		cout << "W = " << a.winopenstat << "\n";
#endif

#endif

		return EXIT_SUCCESS;
	} catch(EAny &e) {
		e.print();
	}
}
