SET(CMAKE_SYSTEM_NAME Generic)

SET(CMAKE_C_COMPILER /usr/bin/avr-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/avr-g++)
SET(CMAKE_AR /usr/bin/avr-ar)
SET(CMAKE_LINKER /usr/bin/avr-ld)
SET(CMAKE_NM /usr/bin/avr-nm)
SET(CMAKE_OBJCOPY /usr/bin/avr-objcopy)
SET(CMAKE_OBJDUMP /usr/bin/avr-objdump)
SET(CMAKE_RANLIB /usr/bin/avr-ranlib)
SET(CMAKE_STRIP /usr/bin/avr-strip)

set(CMAKE_EXE_LINKER_FLAGS "-static")

SET(CSTANDARD "-std=gnu99")
SET(CDEBUG "-gstabs")
SET(CWARN "-Wall -Wstrict-prototypes")
SET(CTUNING "-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums")
SET(COPT "-Os")
SET(CINCS "")
SET(CMCU "-mmcu=attiny45")
SET(CDEFS "-D F_CPU=16000000")

SET(CFLAGS "${CMCU} ${CDEBUG} ${CDEFS} ${CINCS} ${COPT} ${CWARN} ${CSTANDARD} ${CEXTRA}")
SET(CXXFLAGS "${CMCU} ${CDEFS} ${CINCS} ${COPT}")

SET(CMAKE_C_FLAGS  ${CFLAGS})
SET(CMAKE_CXX_FLAGS ${CXXFLAGS})

IF(NOT CMAKE_OBJCOPY)
	ERROR()
ENDIF()

# if you don't trust the static lib, maybe because you have another microprocessor, you can instead compile the lib 
# sources directly in your prroject. Another reason to do so is if you mix c and c++ (don't know why that is a problem yet)
OPTION(USE_ARM_LIB_SRC "Compile the sources of the robot arm lib for every target instead of using the static lib" ON)

MACRO(ADD_AVR_EXECUTABLE AVR_EXE_NAME)

	SET(SourceFiles 
		${ARGV1}
		${ARGV2}
		${ARGV3}
		${ARGV4}
		${ARGV5}
		${ARGV6}
		${ARGV7}
		${ARGV8}
		${ARGV9}
	)
	
	ADD_EXECUTABLE(${AVR_EXE_NAME}.elf ${SourceFiles})

	ADD_CUSTOM_COMMAND(TARGET ${AVR_EXE_NAME}.elf POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom ${AVR_EXE_NAME}.elf ${AVR_EXE_NAME}.hex
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
ENDMACRO(ADD_AVR_EXECUTABLE)

#IF(AVRDUDE)
#    ADD_CUSTOM_TARGET(flash)
#    ADD_DEPENDENCIES(flash RobotArm_RACSQT.hex)

#    ADD_CUSTOM_COMMAND(TARGET flash POST_BUILD
#        COMMAND ${AVRDUDE} -P ${PORT} -b ${ARDUINO_UPLOAD_SPEED} -c ${ARDUINO_PROTOCOL} -p ${ARDUINO_MCU} -V -F -U flash:w:RobotArm_RACSQT.hex:i
#    )
#ENDIF()

