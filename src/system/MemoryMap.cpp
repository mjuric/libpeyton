#include <astro/system/memorymap.h>
#include <astro/exceptions.h>
#include <astro/system/log.h>
#include <astro/util.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include <astro/useall.h>
using namespace std;

const int MemoryMap::pagesize = getpagesize();

void MemoryMap::pagesizealign(ostream &out)
{
	int at = out.tellp();
	int offs = at % pagesize;
	if(offs) { out.seekp(pagesize - offs, ios::cur); }
}

void MemoryMap::pagesizealign(istream &in)
{
	int at = in.tellg();
	int offs = at % pagesize;
	if(offs) { in.seekg(pagesize - offs, ios::cur); }
}

void MemoryMap::open(const std::string &fn, int length_, int offset, int mode, int mapstyle)
{
	if(offset > 0 && (offset % pagesize)) { THROW(EIOException, "Invalid offset requested for memory mapped area - not a multiple of pagesize (" + str(pagesize) + ")"); }

	int flags = 0;

	     if(mode & rw) { flags |= O_RDWR | O_CREAT; }
	else if(mode & ro) { flags |= O_RDONLY; }
	else if(mode & wo) { flags |= O_WRONLY | O_CREAT; }
	else THROW(EIOException, "Invalid mode parameter - mode needs to include ro, wo or rw");

	int fd = ::open(fn.c_str(), flags);
	if(fd == -1)
	{
		fd = 0;
		THROW(EIOException, string("Error opening file [") + fn + "]");
	}

	if(length_ < 0)
	{
		struct stat buf;
		fstat(fd, &buf);
		length_ = buf.st_size;
	}

	close();
	filename = fn;
	open(fd, length_, offset, mode, mapstyle, true);
}

void MemoryMap::open(int fd_, int length_, int offset, int prot, int mapstyle, bool closefd_)
{
	close();
	length = length_;
	closefd = closefd_;
	fd = fd_;

	// check if the length of the file is sufficient to hold the requested length
	// if not, enlarge if possible
	struct stat buf;
	fstat(fd, &buf);
	if(buf.st_size < offset + length)
	{
		if(!(prot & PROT_WRITE))
		{
			close();
			THROW(EIOException, string("A read-only file is smaller than the length requested to be memory mapped"));
		}

		// resize the file
		lseek(fd, offset + length - 1, SEEK_SET);
		char b = 1;
		write(fd, &b, 1);
	}

	map = mmap(0, length, prot, mapstyle, fd, offset);
	if(map == MAP_FAILED)
	{
		map = NULL;
		close();
		THROW(EIOException, string("Memory mapping of file [") + filename + "] falied. Parameters: length=" + str(length) + ", offset=" + str(offset));
	}
}

void MemoryMap::sync()
{
	ASSERT(fd != 0);
	ASSERT(map != NULL);
	msync(map, length, MS_SYNC);
}

MemoryMap::MemoryMap()
: filename(""), fd(0), map(NULL), length(0), closefd(true)
{
}

MemoryMap::MemoryMap(const std::string &fn, int length_, int offset, int mode, int mapstyle)
: filename(fn), fd(0), map(NULL), length(length_)
{
	open(fn, length, offset, mode, mapstyle);
}

void MemoryMap::close()
{
	if(map != NULL)
	{
		if(munmap(map, length) == -1) { THROW(EIOException, string("Error unmapping file [") + filename + "]"); }
		map = NULL;
	}

	if(closefd && fd != 0)
	{
		if(::close(fd) == -1) { THROW(EIOException, string("Error closing file [") + filename + "]"); }
		fd = 0;
	}
}

MemoryMap::~MemoryMap()
{
	close();
}
