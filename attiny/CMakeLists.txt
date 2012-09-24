CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(ShutterGlasses)

INCLUDE(avr_crosscompile.cmake)
IF(NOT CMAKE_CROSSCOMPILING)
	ERROR()
ENDIF()

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}
	/usr/share/arduino/hardware/arduino/cores/arduino
#	/usr/share/arduino/hardware/arduino/variants/standard
)

ADD_EXECUTABLE(ShutterGlassesTiny.elf 
	ShutterGlasses.cpp
	/usr/share/arduino/libraries/SoftwareSerial/SoftwareSerial.cpp
	/usr/share/arduino/hardware/arduino/cores/arduino/Print.cpp
	/usr/share/arduino/hardware/arduino/cores/arduino/WString.cpp
)

ADD_CUSTOM_COMMAND(TARGET ShutterGlassesTiny.elf POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom ShutterGlassesTiny.elf ShutterGlassesTiny.hex
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)


#IF(AVRDUDE)
#    ADD_CUSTOM_TARGET(flash)
#    ADD_DEPENDENCIES(flash ShutterGlassesTiny.hex)

#    ADD_CUSTOM_COMMAND(TARGET flash POST_BUILD
#        COMMAND ${AVRDUDE} -P ${PORT} -b ${AVR_UPLOAD_SPEED} -c ${AVR_PROTOCOL} -p ${AVR_MCU} -V -F -U flash:w:ShutterGlassesTiny.hex:i
#    )
#ENDIF()
