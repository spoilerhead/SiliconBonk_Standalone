# SiliconBonk_Standalone
Standalone versions of silicon bonk and fat toni

Dependencies:

* g++
* cmake
* boost program_options
* libmagick++
* lcms2
* dpkg (for package generation)


build steps:

mkdir build

cd build

cmake .. && make && make package
