bin_PROGRAMS = libpeytondemo
libpeytondemo_SOURCES = libpeytondemo.cpp demo_diskmemorymodel.cpp

# set the include path found by configure
INCLUDES = -I$(top_srcdir)/include $(all_includes)
# the library search path.
libpeytondemo_LDFLAGS = $(all_libraries) 

lib_LIBRARIES =  libpeyton.a
libpeyton_a_SOURCES = Coordinates.cpp Exceptions.cpp Observation.cpp SkyPoint.cpp Time.cpp Util.cpp
SUBDIRS = asteroids image io net sdss system ui
libpeyton_a_LIBADD = $(top_builddir)/src/ui/libui.a $(top_builddir)/src/system/libsystem.a $(top_builddir)/src/asteroids/catalogs/libcatalogs.a $(top_builddir)/src/asteroids/libasteroids.a $(top_builddir)/src/sdss/libsdss.a $(top_builddir)/src/net/libnet.a $(top_builddir)/src/io/libio.a $(top_builddir)/src/image/libimage.a
libpeyton_a_AR = $(top_srcdir)/arlib cru

libpeytondemo_LDADD = libpeyton.a
