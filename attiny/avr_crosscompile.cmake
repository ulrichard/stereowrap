SET(AVR_DEVICE     attiny45)
SET(AVR_FREQUENCY  8000000)
SET(AVRDUDE_DEVICE t45)
SET(AVRDUDE_PORT   /dev/ttyACM0)

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
SET(CMCU "-mmcu=${AVR_DEVICE}")
SET(CDEFS "-D F_CPU=${AVR_FREQUENCY}")

SET(CFLAGS "${CMCU} ${CDEBUG} ${CDEFS} ${CINCS} ${COPT} ${CWARN} ${CSTANDARD} ${CEXTRA}")
SET(CXXFLAGS "${CMCU} ${CDEFS} ${CINCS} ${COPT}")

SET(CMAKE_C_FLAGS  ${CFLAGS})
SET(CMAKE_CXX_FLAGS ${CXXFLAGS})

IF(NOT CMAKE_OBJCOPY)
	ERROR()
ENDIF()





