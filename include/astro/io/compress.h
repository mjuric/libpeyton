#ifndef __astro_io_compress_h
#define __astro_io_compress_h

#include <astro/exceptions.h>

namespace peyton {
namespace io {
namespace compress {

	/// gzip a file given in variable \a path
	void gzip(const std::string &path) throw(peyton::exceptions::EIOException);

}
}
}

#define __peyton_io_compress peyton::io::compress

#endif
