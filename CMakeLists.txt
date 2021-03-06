cmake_minimum_required(VERSION 2.6)
project(peyton)

#
# git-based automatic versioning
#
add_custom_target(
  version-gen
  COMMAND "${PROJECT_SOURCE_DIR}/git-version-gen" "${PROJECT_BINARY_DIR}/include/astro/peyton_version.h" "${PROJECT_SOURCE_DIR}/.version"
  COMMENT "Generating version information"
)
execute_process(
  COMMAND "${PROJECT_SOURCE_DIR}/git-version-gen" "${PROJECT_BINARY_DIR}/include/astro/peyton_version.h" "${PROJECT_SOURCE_DIR}/.version"
)

install (FILES
  "${PROJECT_BINARY_DIR}/include/astro/peyton_version.h"
DESTINATION include/astro)

#
# Check requirements
#
include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
check_function_exists(fmemopen HAVE_FMEMOPEN)

find_package(Boost REQUIRED COMPONENTS regex program_options system )
include_directories(${Boost_INCLUDE_DIRS})

# configure a header file to pass some of the CMake settings to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/peyton_config.h.in"
  "${PROJECT_BINARY_DIR}/include/astro/peyton_config.h"
)
include_directories("${PROJECT_BINARY_DIR}/include")

#
# libpeyton library definitions
#
include_directories(include)
add_library(peyton SHARED

  src/Version.cpp
  src/Coordinates.cpp
  src/Util.cpp
  src/Time.cpp
  src/SkyPoint.cpp
  src/Observation.cpp
  src/Exceptions.cpp

  src/asteroids/catalogs/MemoryCatalog.cpp
  src/asteroids/catalogs/NativeCatalog.cpp
  src/asteroids/catalogs/BowellCatalog.cpp
  src/asteroids/catalogs/MPCCatalog.cpp
  src/asteroids/Asteroid.cpp
  src/asteroids/Catalog.cpp
  src/asteroids/Asteroids.cpp

  src/io/Compress.cpp
  src/io/fpnumber.cpp
  src/io/BinaryStream.cpp
  src/io/Format.cpp
  src/io/FITS.cpp

  src/image/Map.cpp

  src/net/RemoteInstance.cpp
  src/net/Socket.cpp

  src/sdss/Photometry.cpp
  src/sdss/RunGeometry.cpp

  src/system/Shell.cpp
  src/system/FS.cpp
  src/system/DiskMemoryModel.cpp
  src/system/Error.cpp
  src/system/System.cpp
  src/system/Config.cpp
  src/system/Dir.cpp
  src/system/Preferences.cpp
  src/system/Log.cpp
  src/system/Mixed.cpp
  src/system/MemoryMap.cpp
  src/system/Options.cpp

  src/ui/ticker.cpp

)
add_dependencies(peyton version-gen)
target_link_libraries(peyton ${Boost_LIBRARIES})

#
# demo executables
#
add_executable(libpeytondemo src/libpeytondemo.cpp src/demo_diskmemorymodel.cpp)

set(EXTRA_LIBS m dl peyton ${Boost_LIBRARIES})
target_link_libraries(libpeytondemo ${EXTRA_LIBS})

#
# install info
#
install (TARGETS peyton DESTINATION lib)
#install (TARGETS libpeytondemo DESTINATION bin)

install (FILES
  include/astro/version.h
  include/astro/assert.h
  include/astro/constants.h
  include/astro/coordinates.h
  include/astro/exceptions.h
  include/astro/image.h
  include/astro/math.h
  include/astro/macros.h
  include/astro/skypoint.h
  include/astro/time.h
  include/astro/types.h
  include/astro/useall.h
  include/astro/util.h
DESTINATION include/astro)

install (FILES
  include/astro/asteroids/asteroid.h
  include/astro/asteroids/catalog.h
  include/astro/asteroids.h
  include/astro/asteroids/observation.h
DESTINATION include/astro/asteroids)

install (FILES
  include/astro/compat/compat.h
DESTINATION include/astro/compat)

install (FILES
  include/astro/fourier/fft.h
  include/astro/fourier/kernels.h
DESTINATION include/astro/fourier)

install (FILES
  include/astro/image/indexers.h
  include/astro/image/map.h
DESTINATION include/astro/image)

install (FILES
  include/astro/io/binarystream.h
  include/astro/io/compress.h
  include/astro/io/fits.h
  include/astro/io/format.h
  include/astro/io/fortranstream.h
  include/astro/io/fpnumber.h
  include/astro/io/iostate_base.h
  include/astro/io/magick.h
DESTINATION include/astro/io)

install (FILES
  include/astro/io/bits/auto_ptr.h
DESTINATION include/astro/io/bits)

install (FILES
  include/astro/io/gzstream/config/config.gcc2.h
  include/astro/io/gzstream/config/config.gcc3.h
DESTINATION include/astro/io/gzstream/config)

install (FILES
  include/astro/io/gzstream/compat.h
  include/astro/io/gzstream/config.h
  include/astro/io/gzstream/file.h
  include/astro/io/gzstream/fstream.h
  include/astro/io/gzstream/streambuf.h
DESTINATION include/astro/io/gzstream)

install (FILES
  include/astro/math/numeric/integrators.h
DESTINATION include/astro/math/numeric)

install (FILES
  include/astro/math/box.h
  include/astro/math/stats.h
  include/astro/math/vector.h
DESTINATION include/astro/math)

install (FILES
  include/astro/net/remoteinstance.h
  include/astro/net/socket.h
DESTINATION include/astro/net)

install (FILES
  include/astro/sdss/photometry.h
  include/astro/sdss/rungeometry.h
DESTINATION include/astro/sdss)

install (FILES
  include/astro/system/config.h
  include/astro/system/error.h
  include/astro/system/fs.h
  include/astro/system/getopt.h
  include/astro/system.h
  include/astro/system/log.h
  include/astro/system/memorymap.h
  include/astro/system/mixed.h
  include/astro/system/options.h
  include/astro/system/preferences.h
  include/astro/system/shell.h
DESTINATION include/astro/system)

install (FILES
  include/astro/ui/term.h
  include/astro/ui/ticker.h
DESTINATION include/astro/ui)

install (FILES
  include/astro/util/varexpand.h
DESTINATION include/astro/util)

#
# package creation info
#   - this generates a "dist" target to create binary and source packages
#

execute_process(
  COMMAND "${PROJECT_SOURCE_DIR}/git-untracked-files" "${PROJECT_SOURCE_DIR}"
  OUTPUT_VARIABLE CPACK_IGNORE_FILES
)

add_custom_target(dist
	rm -f CPackConfig.make CPackSourceConfig.make &&
	cmake "${CMAKE_CURRENT_SOURCE_DIR}" &&
	make package_source &&
	make package
)

file(READ "${PROJECT_SOURCE_DIR}/.version" VLIST)
STRING(REGEX MATCHALL "[^ ]+" VLIST ${VLIST})
list(GET VLIST 0 CPACK_PACKAGE_FILE_NAME)
list(GET VLIST 1 CPACK_PACKAGE_VERSION_MAJOR)
list(GET VLIST 2 CPACK_PACKAGE_VERSION_MINOR)
list(GET VLIST 3 CPACK_PACKAGE_VERSION_PATCH)
list(GET VLIST 4 CPACK_RPM_PACKAGE_RELEASE)
file(READ "${PROJECT_SOURCE_DIR}/.version.fn" FULL_VERSION)

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set (CPACK_GENERATOR "TBZ2")
set (CPACK_SOURCE_GENERATOR "TBZ2")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "libpeyton - a library of routines for astronomy and astrophysics")
set (CPACK_PACKAGE_DEFAULT_LOCATION "/usr")
set (CPACK_PACKAGE_VENDOR "Mario Juric")
set (CPACK_PACKAGE_CONTACT "Mario Juric <mjuric@ias.edu>")
set (CPACK_PACKAGE_FILE_NAME "libpeyton-${FULL_VERSION}.bin")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "libpeyton-${FULL_VERSION}")
set (CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
include (CPack)
