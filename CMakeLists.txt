cmake_minimum_required(VERSION 2.6)

project(SiliconBonk)
set (BONK_VERSION_MAJOR 0)
set (BONK_VERSION_MINOR 2)
set (BONK_VERSION_PATCH 0)

add_executable(bonk siliconbonk.cpp include/blend_modes.h include/colorspace.h include/fastmath.h include/siliconbonk.h)
add_executable(fattoni fattoni.cpp include/blend_modes.h include/colorspace.h include/fastmath.h include/siliconbonk.h)

# this is a C++ project
enable_language(CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/SiliconBonkConfig.h.in"
  "${PROJECT_BINARY_DIR}/SiliconBonkConfig.h"
  )
 
# add the binary tree to the search path for include files
# so that we will find SiliconBonkConfig.h
include_directories("${PROJECT_BINARY_DIR}")


include(FindOpenMP)
if(OPENMP_FOUND)
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
 set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()



#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

SET( CMAKE_CXX_FLAGS_DBG "-Wno-unused-value  -Wall  -g -O0 " CACHE STRING
    "Flags used by the C++ compiler during maintainer builds."
    FORCE )
MARK_AS_ADVANCED( CMAKE_CXX_FLAGS_DBG )

# Update the documentation string of CMAKE_BUILD_TYPE for GUIs
SET( CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Dbg."
    FORCE )


## Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-fPIC -Wall -Os  -mfpmath=sse -mtune=amdfam10 -msse2 -pipe -ffinite-math-only -fno-trapping-math -fno-signaling-nans -fno-rounding-math -funsafe-math-optimizations -funsafe-loop-optimizations -fsingle-precision-constant  ${CMAKE_CXX_FLAGS} ")
endif()


find_package(Boost 1.46.0 COMPONENTS program_options REQUIRED)
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)

##INCLUDE(FindLCMS2) 
find_package(LCMS2 REQUIRED)

include_directories(include ${PROJECT_SOURCE_DIR}/include ${ImageMagick_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${LCMS2_INCLUDE_DIRS})
target_link_libraries(bonk ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} ${LCMS2_LIBRARIES})
target_link_libraries(fattoni ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} ${LCMS2_LIBRARIES})

##
INSTALL(TARGETS bonk fattoni DESTINATION bin)

##------------------------------------------
SET(CPACK_PACKAGE_NAME "SiliconBonk")
SET(CPACK_PACKAGE_VENDOR "Spoilerhead")

SET(CPACK_PACKAGE_VERSION_MAJOR "${BONK_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${BONK_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${BONK_VERSION_PATCH}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "fast black and white for the masses. Command line version")
SET(CPACK_PACKAGE_EXECUTABLES "bonk fattoni")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Silicon Bonk Standalone")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Silicon Bonk Standalone")

SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
#SET(CPACK_GENERATOR TGZ;DEB;RPM)
SET(CPACK_GENERATOR DEB)

#SET(CPACK_SYSTEM_NAME "Linux-i686")
#SET(CPACK_TOPLEVEL_TAG "Linux-i686")

#Debian packaging stuff
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6), libgcc1 (>= 1:3.4.2-12),libstdc++6, libboost-program-options1.49.0|libboost-program-options1.46.1, libmagick++4 | libmagick++5")
SET(CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION}+sph1)
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dieter Steiner <spoilerhead@gmail.com>")
SET(CPACK_PACKAGE_CONTACT "Dieter Steiner <spoilerhead@gmail.com>")

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
  "/build/;/testdata/;/.git/;~$;.deb;.zip;${CPACK_SOURCE_IGNORE_FILES}")

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)


#create source packages:
set(CPACK_DEBIAN_BUILD_DEPENDS g++ lsb-release debhelper cmake libboost-dev libmagick++-dev libboost-program-options-dev)
set(CPACK_DEBIAN_PACKAGE_SOURCE_COPY git clone) #
set(CPACK_DEBIAN_DISTRIBUTION_NAME ubuntu)
#set(CPACK_DEBIAN_PACKAGE_REMOVE_SOURCE_FILES testdata)

execute_process(COMMAND lsb_release -is
    OUTPUT_VARIABLE _lsb_distribution OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _lsb_release_failed)
set(CPACK_DEBIAN_DISTRIBUTION_NAME ${_lsb_distribution} CACHE STRING "Name of the distribution")
string(TOLOWER ${CPACK_DEBIAN_DISTRIBUTION_NAME} CPACK_DEBIAN_DISTRIBUTION_NAME)
if( ${CPACK_DEBIAN_DISTRIBUTION_NAME} STREQUAL "ubuntu" )
    set(CPACK_DEBIAN_DISTRIBUTION_RELEASES oneiric precise quantal raring CACHE STRING "Release code-names of the distribution release")
endif()
set(DPUT_HOST "ppa:spoilerhead/ppa-bonk" CACHE STRING "PPA repository to upload the debian sources")
  
  
INCLUDE(CPack)
#needs https://openrave.svn.sourceforge.net/svnroot/openrave/trunk/modules-cmake/DebSourcePPA.cmake
INCLUDE(DebSourcePPA) 
