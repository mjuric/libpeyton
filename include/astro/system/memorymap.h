#ifndef __astro_system_memorymap_h
#define __astro_system_memorymap_h

#include <astro/system/log.h>
#include <astro/exceptions.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <iosfwd>

#include <map>
#include <deque>
#include <list>
#include <vector>
#include <iostream>

namespace peyton {

namespace exceptions {
	SIMPLE_EXCEPTION(EDMMException);
}

namespace system {

class MemoryMap
{
protected:
	std::string filename;
	int fd;

	int length;
	void *map;

	bool closefd;
public:
	enum {
		ro = PROT_READ,
		wo = PROT_WRITE,
		rw = PROT_READ | PROT_WRITE,
		none = PROT_NONE,
		exec = PROT_EXEC
	};
	
	enum {
		shared = MAP_SHARED,
		priv = MAP_PRIVATE
	};
	
	static const int pagesize;
	static void pagesizealign(std::ostream &out);
	static void pagesizealign(std::istream &in);
public:
	void open(int fd, int length_, int offset, int mode, int mapstyle, bool closefd = false);
public:
	MemoryMap();
	MemoryMap(const std::string &filename, int length, int offset = 0, int mode = ro, int map = shared);

	void open(const std::string &filename, int length, int offset = 0, int mode = ro, int map = shared);
	void sync();
	void close();

	~MemoryMap();

	operator void *() { return map; }
};

template<typename T>
class MemoryMapVector : public MemoryMap
{
public:
	typedef T* iterator;
	size_t siz;
public:
	MemoryMapVector() : MemoryMap(), siz(0) {}
	void open(const std::string &filename, int size = -1, int offset = 0, int mode = ro, int mapstyle = shared)
	{
		MemoryMap::open(filename, sizeof(T)*size, offset, mode, mapstyle);
		if(size < 0) { siz = length/sizeof(T); } else { siz = size; }
	}

	const T &operator[](int i) const { return ((const T *)map)[i]; }
	T &operator[](int i) { return ((T *)map)[i]; }

	iterator begin() { return (T *)map; }
	iterator end() { return ((T *)map) + siz; }

	T& front() { return *(T *)map; }
	T& back() { return ((T *)map) + (siz-1); }

	size_t size() const { return siz; }
	size_t allocated() { return length / sizeof(T); }
};

////////////////////////
#define PRIQUEUE 1

struct DMMBlock
{
	long long begin;	// long memory index to which 'offset' corresponds to (in bytes)
	std::string path;	// path to the file where this block resides
	int fileoffset;		// offset where DMM data begins (in bytes)
	int length;		// length of DMM data (in bytes)

	int fd;			// file descriptor linked to this DMMBlock - used my DMM* classes below
	
	DMMBlock() : begin(0), fileoffset(0), length(0), fd(0) {}
	DMMBlock(long long begin_, const std::string &path_, int offset_, int length_)
		: begin(begin_), path(path_), fileoffset(offset_), length(length_), fd(0)
	{}

	int openfile(int openmode);
	void closefile();

	int offset(long long o);
	int maxlen(long long o);

	std::string generatepathname(const std::string &prefix, int recordsize);
};

class DMMSet
{
protected:
	int maxfilelen;		/// maximum length of data in a file (in bytes)
	
	std::string prefix;	// prefix of storage files
	const int recordsize;		// size of each record

	long long arraysize;	// in records

	bool autoextend;	// if an out-of-range offset is requested, should we extend the map?
	int autoblocklen;	// blocklen for automatically added files (in records)
	int autooffset;		// offset of data start in automatically added files (bytes)

	bool dirty;
public:
	std::map<long long, DMMBlock> blocks;
protected:
	DMMBlock &autoaddblock(long long idx, DMMBlock *before, DMMBlock *after);
	void closefilehandles();
public:
	// creation/loading
	void create(const std::string &prefix, long long totallength = -1, int blocklen = 0, int offset = 0);
	bool load(const std::string &dmmfn);

	// saving
	void save(const std::string &dmmfn);
	
	// erasing the datafiles from disk
	void close();		// close all open filehandles, clear the blocks (i.e. - prepare this DMMSet for next load())
	void truncate();	// close then erase all the datafiles

	// low level construction
	DMMBlock &addblock(const DMMBlock &block);
	DMMBlock &addblock(int length = -1, int offset = 0, const std::string &file_ = std::string());
	void removeblock(int idx);

	// properties
	long long size() const;		// DMM set size in records
	void setsize(long long size);
	long long capacity() const;

	DMMSet(int recordsize);
	~DMMSet();

	friend class DiskMemoryModel;
	DMMBlock &findblock(long long idx, bool *wasnew = NULL);
};

class DiskMemoryModel
{
protected:
	DMMSet dmm;
	std::string dmmfn;

	struct Window
	{
		long long begin, end;

		MemoryMap *mm;
		int fd;

		Window(long long begin_ = 0, long long end_ = 0, int fd_ = 0) : begin(begin_), end(end_), fd(fd_), mm(NULL) {}
		char *memory() { return (char *)(void *)(*mm); }

		bool operator <(const Window &w) const { return begin < w.begin; }
	};

protected:

#if PRIQUEUE
	typedef std::list<Window> windowlist;
	typedef std::map<long long, windowlist::iterator> windowmap;
#else
	typedef std::map<long long, Window> windowmap;
	typedef std::deque<Window> windowlist;
#endif
	windowlist windowqueue;
	windowmap openwindows;
	
	int openmode;
	int prot;

	int maxwindows, windowsize;
	int maxfilelen;
public:
	int winopenstat;
protected:
	Window &findwindow(long long idx, long long end);
	windowmap::iterator openwindow(long long begin, long long end);

	MemoryMap *openmm(Window &w, int fileoffset);
	void closemm(Window &w);
	void closewindows();
	void setmode(const std::string &mode);
	void setmaxfilelen(const int filelen);
public:
	DiskMemoryModel(int recordsize = 1);
	~DiskMemoryModel();

	// setup and initialization
//	void addfile(const std::string &fn, int length = -1, int offset = 0);
//	int addfiles(const std::string &prefix, long long totallength = -1, int length = -1, int offset = 0);
	int  setmaxwindows(int mw);
	int  setwindowsize(int ws);

	void create(const std::string &dmmfn);
	void open(const std::string &dmmfn, const std::string &mode, bool create = true);
	void truncate();
	void sync();
	void close();

	char *get(long long at, int len);

	bool writable() { return (openmode & O_WRONLY) || (openmode & O_RDWR); } // O_* flags are defined in bits/fnctl.h
};

//#define DMMASSERT(x) x
#define DMMASSERT(x)

template <typename T>
class DMMArray : public DiskMemoryModel
{
protected:
	long long tooffset(int idx) { long long offset = idx; offset *= sizeof(T); return offset; }
public:
	struct iterator : public std::iterator<std::random_access_iterator_tag, T>
	{
		DMMArray *ref;
		int idx;

		T &operator*() { return (*ref)[idx]; }
		T  operator*() const { return (*ref)[idx]; }
		iterator &operator++() { ++idx; DMMASSERT(idx >= 0 && idx <= ref->size()); return *this; }
		iterator &operator--() { --idx; DMMASSERT(idx >= 0 && idx <= ref->size()); return *this; }
		iterator operator--(int) { iterator it(*this); --idx; DMMASSERT(idx >= 0 && idx <= ref->size()); return it; }
		bool operator==(const iterator &it) { return it.ref == ref && it.idx == idx; }
		bool operator!=(const iterator &it) { return !(*this == it); }

		iterator &operator+=(int k) { idx += k; DMMASSERT(idx >= 0 && idx <= ref->size()); return *this; }
		iterator &operator-=(int k) { idx -= k; DMMASSERT(idx >= 0 && idx <= ref->size()); return *this; }

		iterator operator+(const int n) const { return iterator(ref, idx + n); }
		iterator operator-(const int n) const { return iterator(ref, idx - n); }
		int operator-(const iterator &i) const { DMMASSERT(i.ref == ref); return idx - i.idx; }
		T &operator[](const int n) { DMMASSERT(idx +n >= 0 && idx +n <= ref->size()); return (*ref)[idx + n]; }
		T  operator[](const int n) const { DMMASSERT(idx +n >= 0 && idx +n <= ref->size()); return (*ref)[idx + n]; }

		bool operator<(const iterator &i) const { return idx < i.idx; }

		iterator(DMMArray *ref_ = NULL, int idx_ = 0) : ref(ref_), idx(idx_) 
		{
			if(ref)
			{
				DMMASSERT(idx >= 0 && idx <= ref->size());
			}
		}
	};
protected:
	iterator beg;
public:
	DMMArray(const std::string &dmmfn = std::string(), const std::string &mode = "r", bool create = true)
	: DiskMemoryModel(sizeof(T)), beg(this)
	{
		if(dmmfn.size())
		{
			open(dmmfn, mode, create);
		}
	}

	T &operator[](int idx)
	{
		DMMASSERT(idx >= 0 && idx < capacity());
		//ASSERT(writable() || idx < size());
		if(!(writable() || idx < size())) { std::cerr << "ASSF: " << idx << " " << size() << "\n";}
		T &tmp = *(T *)get(tooffset(idx), sizeof(T));
		if(idx >= dmm.size()) { dmm.setsize(idx+1); }
		return tmp;
	}
	int size() const { return dmm.size(); }
	int capacity() const { return dmm.capacity(); }

	void push_back(const T &v) { (*this)[size()] = v; }

	iterator begin() const { return beg; }
	iterator end() { return iterator(this, size()); }
};

} // namespace system
} // namespace peyton

#define __peyton_system peyton::system
#endif

