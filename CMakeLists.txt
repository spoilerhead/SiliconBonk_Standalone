cmake_minimum_required(VERSION 2.6)
project(SiliconBonk
)
add_executable(bonk siliconbonk.cpp include/blend_modes.h include/colorspace.h include/fastmath.h)

# this is a C++ project
enable_language(CXX)


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
include_directories(nclude ${PROJECT_SOURCE_DIR}/include ${ImageMagick_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(bonk ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})

##
INSTALL(TARGETS bonk DESTINATION bin)
SET(CPACK_PACKAGE_NAME "SiliconBonk")
SET(CPACK_PACKAGE_VENDOR "Spoilerhead")
SET(CPACK_PACKAGE_VERSION "0.0.4")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "4")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "fast black and white for the masses. Command line version")
SET(CPACK_PACKAGE_EXECUTABLES "bonk")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Silicon Bonk Standalone")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Silicon Bonk Standalone")

SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
#SET(CPACK_GENERATOR TGZ;DEB;RPM)
SET(CPACK_GENERATOR DEB)

#SET(CPACK_SYSTEM_NAME "Linux-i686")
#SET(CPACK_TOPLEVEL_TAG "Linux-i686")

#Debian packaging stuff
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6), libgcc1 (>= 1:3.4.2-12), libboost-program-options1.46.1 | libboost-program-options1.42.0, libmagick++3 | libmagick++4")
SET(CPACK_DEBIAN_PACKAGE_VERSIONS "x64")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dieter Steiner <spoilerhead@gmail.com>")

INCLUDE(CPack)
