#!/bin/sh

mkdir -p build
cd build
cmake -D CMAKE_TOOLCHAIN_FILE:PATH=avr_crosscompile.cmake ..
make
avrdude -P /dev/ttyACM0 -p t45 -c stk500v2 -Uflash:w:ShutterGlassesTiny.hex

