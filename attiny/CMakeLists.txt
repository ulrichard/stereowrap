CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(ShutterGlasses)

INCLUDE(avr_crosscompile.cmake)
IF(NOT CMAKE_CROSSCOMPILING)
	ERROR()
ENDIF()

INCLUDE_DIRECTORIES(
	/usr/share/arduino/hardware/arduino/cores/arduino
	/usr/share/arduino/hardware/arduino/variants/standard
)

ADD_AVR_EXECUTABLE(ShutterGlassesTiny 
	ShutterGlasses.cpp
	/usr/share/arduino/libraries/SoftwareSerial/SoftwareSerial.cpp
)

