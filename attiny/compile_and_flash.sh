#!/bin/sh

mkdir -p build
cd build
cmake -G "CodeBlocks - Unix Makefiles"  ..
make
make flash

