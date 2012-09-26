set(CMAKE_TOOLCHAIN_FILE avr_crosscompile.cmake)

SET(CMCU "-mmcu=attiny45")
SET(CDEFS "-D F_CPU=8000000")

SET(AVRDUDE_DEVICE t45)
SET(AVRDUDE_PORT   /dev/ttyACM0)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(ShutterGlasses)

INCLUDE(avr_crosscompile.cmake)
IF(NOT CMAKE_CROSSCOMPILING)
	ERROR()
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(ShutterGlassesTiny.elf
	ShutterGlasses.c
	uart.c
)

ADD_CUSTOM_COMMAND(TARGET ShutterGlassesTiny.elf POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom ShutterGlassesTiny.elf ShutterGlassesTiny.hex
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# for fuse settings see http://www.engbedded.com/fusecalc/
ADD_CUSTOM_TARGET(flash
			avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -Uflash:w:ShutterGlassesTiny.hex
	COMMAND avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -Ulfuse:w:0xe2:m
	COMMAND avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -Uhfuse:w:0xdf:m
	COMMAND avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -Uefuse:w:0xff:m
	DEPENDS ShutterGlassesTiny.elf
)

