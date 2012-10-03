
set(CMAKE_TOOLCHAIN_FILE avr_crosscompile.cmake)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(ShutterGlasses)

INCLUDE(avr_crosscompile.cmake)
IF(NOT CMAKE_CROSSCOMPILING)
	ERROR()
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(ShutterGlassesTiny.elf
	ShutterGlasses.c
)

ADD_CUSTOM_COMMAND(TARGET ShutterGlassesTiny.elf POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom ShutterGlassesTiny.elf ShutterGlassesTiny.hex
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

OPTION(USE_CRYSTAL_CLOCK on)

# for fuse settings see http://www.engbedded.com/fusecalc/
IF(USE_CRYSTAL_CLOCK)
        SET(LFUSE 0xce)
        SET(HFUSE 0xdf)
        SET(EFUSE 0xff)
ELSE()
        SET(LFUSE 0xe2)
        SET(HFUSE 0xdf)
        SET(EFUSE 0xff)
ENDIF()

ADD_CUSTOM_TARGET(flash
			avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -Uflash:w:ShutterGlassesTiny.hex
	COMMAND avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -Ulfuse:w:${LFUSE}:m
	COMMAND avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -Uhfuse:w:${HFUSE}:m
	COMMAND avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -Uefuse:w:${EFUSE}:m
	DEPENDS ShutterGlassesTiny.elf
)

ADD_CUSTOM_TARGET(simavr
			run_avr -m ${AVR_DEVICE} -f ${AVR_FREQUENCY} ShutterGlassesTiny.hex
	DEPENDS ShutterGlassesTiny.elf
)
