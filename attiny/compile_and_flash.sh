#!/bin/sh

mkdir -p build
cd build
cmake -G "CodeBlocks - Unix Makefiles" -D CMAKE_TOOLCHAIN_FILE:PATH=avr_crosscompile.cmake ..
make
avrdude -P /dev/ttyACM0 -p t45 -c stk500v2 -Uflash:w:ShutterGlassesTiny.hex
avrdude -P /dev/ttyACM0 -p t45 -c stk500v2 -Ulfuse:w:0xe2:m
avrdude -P /dev/ttyACM0 -p t45 -c stk500v2 -Uhfuse:w:0xdf:m
avrdude -P /dev/ttyACM0 -p t45 -c stk500v2 -Uefuse:w:0xff:m
