#ifndef _astro_catalog_h
#define _astro_catalog_h

#include <vector>
#include <astro/asteroids/asteroid.h>
#include <astro/exceptions.h>

namespace peyton {

namespace exceptions {
	SIMPLE_EXCEPTION(ECatalog);
	DERIVE_EXCEPTION(ECatalogTypeUnknown, ECatalog);
}

namespace asteroids {

	/**
		\brief Abstract class for accessing asteroid catalogs.
		
		You can not instantiate this class directly. Instead, use the static member function
		#openCatalog().
		
		\todo Add example code
	*/
	class Catalog {
	protected:
		virtual bool openCatalog(const char *filename, const char *mode) = 0;
	public:
		static Catalog *open(const char *filename, const char *type = NULL, const char *mode = "r");

		virtual int read(Asteroid &obj, const char *name) = 0;
		virtual int read(std::vector<Asteroid> &obj, const int from, const int to) = 0;
		virtual int read(Asteroid &obj, const int id) = 0;
		virtual int recordCount() = 0;

		virtual int read(std::vector<Asteroid> &obj, int *ids, int num);

		virtual int write(Asteroid &obj, int at) = 0;

		virtual char *identify(char *) = 0;

		virtual ~Catalog() {};
	};

} // namespace asteroids
} // namespace peyton

#endif
