#! /bin/sh

mkdir build
cd build
cmake -G "CodeBlocks - Unix Makefiles" -D CMAKE_BUILD_TYPE:STRING=Release ..
make

export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
../stereowrap -m sequential glxgears -stereo


