#!/bin/sh

mkdir -p build
cd build
cmake -G "CodeBlocks - Unix Makefiles" -D  ..
make
make flash

