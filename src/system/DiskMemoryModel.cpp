#include <astro/system/memorymap.h>
#include <astro/system/fs.h>
#include <astro/exceptions.h>
#include <astro/util.h>
#include <astro/system/log.h>
#include <astro/system/config.h>
#include <astro/io/format.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream.h>

#include <deque>
#include <map>

#include <astro/useall.h>
using namespace std;

#if 1

template<typename K, typename V> inline V &last(map<K, V> &m)
{
	return (*m.rbegin()).second;
}

template<typename K, typename V> inline const V &last(const map<K, V> &m)
{
	return (*m.rbegin()).second;
}

/////////////////////////////////////////////////////////

int DMMBlock::openfile(int openmode)
{
	if(fd != 0) { return fd; }

	fd = ::open(path.c_str(), openmode, 0644);	// TODO: This with specifying the umask is a hack...
	if(fd == -1) { THROW(EIOException, string("Error opening file [") + path + "]"); }

	return fd;
}

void DMMBlock::closefile()
{
	if(!fd) return;
		
	if(::close(fd) == -1) { THROW(EIOException, string("Error closing file [") + path + "]"); }
	
	fd = 0;
}

int DMMBlock::offset(long long idx)
{
	return fileoffset + (idx - begin);
}

int DMMBlock::maxlen(long long idx)
{
	return length - (idx - begin);
}

std::string DMMBlock::generatepathname(const std::string &prefix, int recordsize)
{
	char buf[500];
	sprintf(buf, "%010lld-%010lld", begin / recordsize, (begin + length) / recordsize);
	return io::format("%s.%s.mem") << prefix << buf;
}

////////////////////


DMMSet::~DMMSet()
{
}

long long DMMSet::capacity() const
{
	if(!blocks.size()) return 0;
	const DMMBlock &b = last(blocks);
	return (b.begin + b.length) / recordsize;
}

long long DMMSet::size() const
{
	return arraysize;
}

void DMMSet::setsize(long long s)
{
	ASSERT(s <= capacity());
	arraysize = s;
}

DMMSet::DMMSet(int recordsize_)
	: dirty(false), maxfilelen((1 << 31) - 1), recordsize(recordsize_), arraysize(0),
	autoextend(false), autoblocklen(maxfilelen/recordsize), autooffset(0)
{
}

DMMBlock &DMMSet::addblock(const DMMBlock &block)
{
	blocks[block.begin] = block;
	return blocks[block.begin];
}

DMMBlock &DMMSet::addblock(int length, int offset, const std::string &file)
{
	if(length <= 0)
	{
		length = (maxfilelen - offset) / recordsize;
		if(length <= 0) { THROW(EDMMException, "Too high offset requested - not a single record can fit into a maximum sized file"); }
	}

	long long begin = blocks.size() ? last(blocks).begin + last(blocks).length : 0;
	DMMBlock block(begin, file, offset, length*recordsize);

	if(!file.size())
	{
		if(!prefix.size()) { THROW(EDMMException, "Before adding unnamed block to a DMM set, you have to name the set"); }
		block.path = block.generatepathname(prefix, recordsize);
	}

	return addblock(block);
}

void DMMSet::truncate()
{
	// erase any storage files we have
	closefilehandles();

	FOREACH2(blocks_t::iterator, blocks)
	{
		const char *fn = (*i).second.path.c_str();
		unlink(fn);
	}

	close();
}

void DMMSet::closefilehandles()
{
	FOREACH2(blocks_t::iterator, blocks)
	{
		(*i).second.closefile();
	}
}

void DMMSet::close()
{
	closefilehandles();

	blocks.clear();
	arraysize = 0;
}

/**
	Create new DMM set, of totallength records, with blocklen records
	per file, and with each file starting at offset offset (given in bytes!)
*/
void DMMSet::create(const std::string &prefix_, long long totallength, int blocklen, int offset)
{
	ASSERT(prefix_.size());
	if(offset < 0 || offset >= maxfilelen) { THROW(EDMMException, "Invalid offset request (blocklen = " + str(offset) + ")"); }

	// check blocklen
	if(blocklen < 1)
	{
		blocklen = (maxfilelen - offset) / recordsize;
		if(blocklen <= 0) { THROW(EDMMException, "Too high offset requested - not a single record can fit into a maximum sized file"); }
	}

	truncate();
	prefix = prefix_;
	
	// if totallength <= 0, create an autoextendable DMM set
	if(totallength <= 0)
	{
		autoextend = true;
		autoblocklen = blocklen;
		autooffset = offset;
	}
	else
	{
		// create blocks
		while(totallength > 0)
		{
			if(totallength < blocklen) { blocklen = totallength; }
			addblock(blocklen, offset);
			totallength -= blocklen;
		}
	}
}

void DMMSet::save(const std::string &cfgfn)
{
	if(!cfgfn.size()) { THROW(EDMMException, "Invalid filename"); }

	ofstream out(cfgfn.c_str());
	if(!out.good()) { THROW(EDMMException, "Error opening [" + cfgfn + "] file for saving"); }
	
	out << "# Disk Memory Model (DMM) Set file\n#\n\n";

	out << "prefix = " << prefix << "\n";
	out << "recordsize = " << recordsize << "\n";
	out << "arraysize = " << arraysize << "\n";
	out << "autoextend = " << autoextend << "\n";
	
	if(autoextend)
	{
		out << "autoblocklen = " << autoblocklen << "\n";
		out << "autooffset = " << (int)autooffset << "\n";
	}
	out.flush();

	int i = 1;
	FOREACH2j(blocks_t::iterator, k, blocks)
	{
		DMMBlock &b = (*k).second;

		out << "\n";
		out << "block " << i << " path   = " << b.path << "\n";
		out << "block " << i << " offset = " << b.fileoffset << "\n";
		out << "block " << i << " begin = " << b.begin / recordsize << "\n";
		out << "block " << i << " length = " << b.length / recordsize << "\n";
		
		i++;
	}
}

bool DMMSet::load(const std::string &cfgfn)
{
	try
	{
		Config cfg(cfgfn);

		close();

		// global config
		if(!cfg.count("prefix")) { THROW(EDMMException, "No 'prefix' keyword found in DMM file"); }
		prefix = cfg["prefix"];
		
		if(!cfg.count("recordsize")) { THROW(EDMMException, "No 'recordsize' keyword found in DMM file"); }
		ASSERT(recordsize == (int)cfg["recordsize"]);
	
		if(!cfg.count("arraysize")) { THROW(EDMMException, "No 'arraysize' keyword found in DMM file"); }
		arraysize = cfg["arraysize"];

		if(!cfg.count("autoextend")) { THROW(EDMMException, "No 'autoextend' keyword found in DMM file"); }
		autoextend = (int)cfg["autoextend"] != 0;

		if(autoextend)
		{
			if(!cfg.count("autoblocklen")) { THROW(EDMMException, "No 'autoblocklen' keyword found in DMM file"); }
			autoblocklen = (int)cfg["autoblocklen"];

			if(!cfg.count("autooffset")) { THROW(EDMMException, "No 'autooffset' keyword found in DMM file"); }
			autooffset = (int)cfg["autooffset"];
		}

		// load blocks
		for(int k = 1; true; k++)
		{
			string prefix = "block " + str(k) + " ";
			
			if(!cfg.count(prefix + "path")) break;
			string path = cfg[prefix + "path"];
	
			if(!cfg.count(prefix + "offset")) { THROW(EDMMException, "No " + prefix + "offset" + " keyword found in DMM file"); }
			int offset = cfg[prefix + "offset"];

			if(!cfg.count(prefix + "begin")) { THROW(EDMMException, "No " + prefix + "begin" + " keyword found in DMM file"); }
			long long begin = cfg[prefix + "begin"];

			if(!cfg.count(prefix + "length")) { THROW(EDMMException, "No " + prefix + "length" + " keyword found in DMM file"); }
			long long length = cfg[prefix + "length"];
			
			addblock(DMMBlock(begin*recordsize, path, offset, length*recordsize));
		}
		
		return true;
	}
	catch(EFile &e)
	{
		return false;
	}
}
	
DMMBlock &DMMSet::autoaddblock(long long byteidx, DMMBlock *before, DMMBlock *after)
{
	DMMBlock b;
	b.fileoffset = autooffset;
	const long long byteblocklen = autoblocklen * recordsize;

	if(before != NULL && after != NULL && 
		(after->begin - (before->begin + before->length) <= byteblocklen))
	{
		// fill the gap, if it's small enough
		b.begin = before->begin + before->length;
		b.length = after->begin - b.begin;
	} else {
		// method 2 - partition the space into byteblocklen sized
		// blocks, from 0 to infinity, and map thisone wherever
		// it falls
		b.begin = (byteidx / byteblocklen) * byteblocklen;
		b.length = byteblocklen;
	}
	ASSERT(b.begin <= byteidx && byteidx < b.begin + b.length);
	
	// generate a filename
	b.path = b.generatepathname(prefix, recordsize);

	return addblock(b);
}

DMMBlock &DMMSet::findblock(long long byteidx, bool *wasnew)
{
	ASSERT(byteidx >= 0);
	bool dummy;
	if(!wasnew) { wasnew = &dummy; }
	
	// find the file which hosts this offset
	map<long long, DMMBlock>::iterator i;

	i = blocks.upper_bound(byteidx);		// first window greater than required

	*wasnew = true;
	if(i == blocks.begin())		// all covered indices are higher than this one
	{
		DMMBlock *last = i == blocks.end() ? NULL : &(*i).second;
		if(autoextend) { return autoaddblock(byteidx, NULL, last); }
		else { THROW(EDMMException, "Requested index not in range, and autoextend is off\n"); }
	}
	else
	{
		--i;
		DMMBlock &b = (*i).second;
		if(b.begin + b.length <= byteidx)	// if this block precedes the requested index...
		{
			++i;
			DMMBlock *last = i == blocks.end() ? NULL : &(*i).second;
			if(autoextend) { return autoaddblock(byteidx, &b, last); }
			else { THROW(EDMMException, "Requested index not in range, and autoextend is off\n"); }
		}
		*wasnew = false;
		return b;
	}
	ASSERT(0);
}

/////////////////////////////////////////////////////////

DiskMemoryModel::DiskMemoryModel(int recordsize_)
	: maxfilelen((1 << 31) - 1), winopenstat(0), dmm(recordsize_)
{
	setmode("r");

	setmaxwindows(50);
	setwindowsize(5*1024*1024);
}

#if 0
DiskMemoryModel::DiskMemoryModel(const DMMSet &dmm, const std::string &mode, int recordsize_)
	: maxfilelen((1 << 31) - 1), recordsize(recordsize_), winopenstat(0), dmm(0, "")
{
	setmaxwindows(50);
	setwindowsize(5*1024*1024);

	open(dmm, mode);
}
#endif

void DiskMemoryModel::setmaxfilelen(const int filelen)
{
	maxfilelen = filelen;
}

void DiskMemoryModel::setmode(const std::string &mode)
{
	     if(mode == "rw") { openmode = O_RDWR | O_CREAT; prot = PROT_READ | PROT_WRITE; }
	else if(mode == "r")  { openmode = O_RDONLY; prot = PROT_READ; }
	else if(mode == "w")  { openmode = O_WRONLY | O_CREAT; prot = PROT_WRITE; }
	else THROW(EIOException, "Invalid mode parameter - mode needs to include ro, wo or rw");
}

#if 0
void DiskMemoryModel::addfile(const std::string &fn, int length, int offset)
{
	FileWindow fv;
	fv.path = fn;

	// if length == -1, figure out the maximum length from the file size
	if(length == -1)
	{
		fv.length = Filename(fv.path).size() - offset;
	}
	else
	{
		fv.length = length;
	}

	// adjust begin offset
	if(files.size())
	{
		const FileWindow &prev = last(files);
		fv.begin = prev.begin + prev.length;
	}

	// store file offset
	fv.offset = offset;

	// increase our current maximum size
	storagesize += fv.length;

	files[fv.begin] = fv;
}
#endif

#if 0
int DiskMemoryModel::addfiles(const std::string &prefix, long long totallength, int length, int offset)
{
	ASSERT(!(offset % MemoryMap::pagesize));

	// if totallength < 0, scan the directory for existing memory files matching the pattern
	// and include all of those
	if(totallength < 0)
	{
		int k = 0;
		totallength = 0;
		do {
			Filename f(prefix + str(k) + ".mem");
			if(!f.exists()) break;
			int size = f.size();
			if(size < offset)
			{
				THROW(EIOException, "The file [" + (std::string)f + "] is too small to be included in this DMM set.");
			}
			totallength += size - offset;
		} while(true);
		if(totallength = 0)
		{
			THROW(EIOException, "To open a DMM without specifying the size, you need to have existing files. No files found");
		}
	}

	int k = 0;
	while(totallength > 0)
	{
		std::string fn(prefix + str(k) + ".mem");

		int blocklen = 0;
		if(length > 0)
		{
			blocklen = length;
		}
		else
		{
			// either read in the size from the files, or make it the maximum size
			Filename f(fn);
			if(f.exists())
			{
				blocklen = f.size() - offset;
			}
			
			if(blocklen <= 0)
			{
				// conjure up maximum reasonable length
				blocklen = std::min(totallength, (long long)(maxfilelen - offset));
				// adjust the length to be a multiple of recordsize
				blocklen -= blocklen % recordsize;

				// check if this is the last file to be added, and if so, if there's a little
				// bit dangling at the end (shouldn't be for any realistic application,
				// but check anyway)
				if(blocklen < totallength && blocklen < maxfilelen - recordsize)
				{
					blocklen += recordsize;
				}
			}
		}

		addfile(fn, blocklen, offset);
		totallength -= blocklen;
		k++;
	}
	return k;
}
#endif

int DiskMemoryModel::setmaxwindows(int mw)
{
	int cur = maxwindows;
	maxwindows = mw;
	return cur;
}

int DiskMemoryModel::setwindowsize(int ws)
{
	int cur = windowsize;
	windowsize = ws;
	return cur;
}

#if 0
void DiskMemoryModel::resize(long long newsize)
{
	// add files, open them
	dmm.resize(newsize / dmm.recordsize);
}
#endif

void DiskMemoryModel::truncate()
{
	ASSERT(dmmfn.size());
	closewindows();
	dmm.truncate();
	sync();
}

void DiskMemoryModel::create(const std::string &dmmfn)
{
	open(dmmfn, "rw", true);
	truncate();
}

void DiskMemoryModel::open(const std::string &dmmfn_, const std::string &mode, bool create)
{
	close();

	dmmfn = dmmfn_;
	setmode(mode);

	bool succ = dmm.load(dmmfn);
	if(succ) return;

	if(create && (mode == "rw" || mode == "w"))
	{
		dmm.create(dmmfn);
		sync();
	}
	else
	{
		THROW(EDMMException, "Could not open [" + dmmfn + "] DMM set");
	}
}

#if 0
void DiskMemoryModel::open(const DMMSet &dmm_, const std::string &mode)
{
	close();

	dmm = dmm_;

	open(mode);
}
#endif

#if PRIQUEUE
	#define LIST(i) (*(*i).second)
#else
	#define LIST(i) ((*i).second)
#endif

void DiskMemoryModel::sync()
{
	FOREACH2(windowmap::iterator, openwindows) { LIST(i).mm->sync(); }
	if(openmode & O_RDWR || openmode & O_WRONLY) { dmm.save(dmmfn); }
}

void DiskMemoryModel::closewindows()
{
	FOREACH2(windowmap::iterator, openwindows) { closemm(LIST(i)); }
	openwindows.clear();
	windowqueue.clear();
}

void DiskMemoryModel::close()
{
	if(!dmmfn.size()) return;

	closewindows();

	if(openmode & O_RDWR || openmode & O_WRONLY) { dmm.save(dmmfn); }

	dmm.close();
	dmmfn.clear();
}

char *DiskMemoryModel::get(long long at, int len)
{
#if 0
	if(at + len >= storagesize && resizable)
	{
		resize(at + len);
	} else {
		ASSERT(at >= 0 && at + len <= storagesize);
	}
#endif
	Window &w = findwindow(at, at + len);
	char *mem = w.memory() + (at - w.begin);
	return mem;
}

DiskMemoryModel::Window &DiskMemoryModel::findwindow(long long idx, long long end)
{
	windowmap::iterator i;
	bool newwin = false;

	i = openwindows.upper_bound(idx);	// first window with 'begin' greater than 'idx'

	if(i == openwindows.begin())		// if this is the first window in the map, we need to create our window
	{
		i = openwindow(idx, end);
		newwin = true;
	}
	else
	{
		--i;
	}

	Window &w = LIST(i);
	if( idx < w.begin || end > w.end )
	{
		i = openwindow(idx, end);	// open a new window for this location
		newwin = true;
	}

#if PRIQUEUE
	// move the window up the priority queue if it's not a new window
	if(!newwin)
	{
		windowlist::iterator j = (*i).second;
		windowqueue.splice(windowqueue.end(), windowqueue, (*i).second, ++j);
	}
#endif
		
	return LIST(i);
}

DiskMemoryModel::windowmap::iterator DiskMemoryModel::openwindow(long long begin, long long end)
{
	// check if the queue is full, erase the oldest window if so
	if(windowqueue.size() >= maxwindows)
	{
		closemm(windowqueue.front());
		openwindows.erase(windowqueue.front().begin);
		windowqueue.pop_front();
	}
//	std::cout << "N(win) = " << windowqueue.size() << " | begin = " << begin/44 << "\n";


	// find the file descriptor of the file that covers this window
	bool wasnew;
	DMMBlock &block = dmm.findblock(begin, &wasnew);
	int fd = block.openfile(openmode);
	int fileoffset = block.offset(begin);
	int len = std::min(block.maxlen(begin), windowsize);

	// quick test: center the window
#if 1
	if(fileoffset >= windowsize)
	{
		fileoffset -= windowsize;
		begin -= windowsize;
		len += windowsize;
	} else {
		begin -= fileoffset;
		len += fileoffset;
		fileoffset = 0;
	}
#endif

	//
	// adjust fileoffset and begin location to pagesize multiple
	//
	int psoffs = fileoffset % MemoryMap::pagesize;
	begin -= psoffs;
	fileoffset -= psoffs;
	len += psoffs;

	// if this is a read only DMM, then its end can at most equat the
	// end of the file, no matter what the DMMBlock says
	if(openmode == O_RDONLY)
	{
		struct stat buf;
		fstat(fd, &buf);
		if(buf.st_size < fileoffset + len)
		{
			len = buf.st_size - fileoffset;
		}
	}

	// it's an error if the range requested spans physical files
	if(!(len >= end - begin))
	{
		psoffs = 543;
		ASSERT(0);
	}
	ASSERT(len >= end - begin);

	// if the DMM set was autoextended, sync it to disk to record
	// this major change
	if(wasnew) { sync(); }
	
	// open a new memory mapping
	Window w(begin, begin + len, fd);
	w.mm = openmm(w, fileoffset);

	// add this window to queue and map
	windowqueue.push_back(w);
#if PRIQUEUE
	openwindows[w.begin] = --windowqueue.end();
#else
	openwindows[w.begin] = w;
#endif
	winopenstat++;
	return openwindows.find(w.begin);
}

void DiskMemoryModel::closemm(Window &w)
{
	if(w.mm) { delete w.mm; w.mm = NULL; }
}

MemoryMap *DiskMemoryModel::openmm(Window &w, int fileoffset)
{
	closemm(w);

	auto_ptr<MemoryMap> mm(new MemoryMap);

	mm->open(w.fd, w.end - w.begin, fileoffset, prot, MAP_SHARED);

	return mm.release();
}

DiskMemoryModel::~DiskMemoryModel()
{
	close();
}

#endif
