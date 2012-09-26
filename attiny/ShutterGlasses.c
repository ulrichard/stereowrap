#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"

// ATMEL ATTINY45 / ATTINY85
//
//                  +-\/-+
// Ain0 (D 5) PB5  1|    |8  Vcc
// Ain3 (D 3) PB3  2|    |7  PB2 (D 2)  Ain1
// Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
//            GND  4|    |5  PB0 (D 0) pwm0
//                  +----+

//                  +-\/-+
//                 1|    |8  Vcc
//                 2|    |7
// output opamp <- 3|    |6
//            GND  4|    |5  <- uart rx
//                  +----+


int main(void)
{
    const uint8_t outOpAmp = PB4;
    const uint8_t inUartRx = PB0;

    DDRB = (1 << outOpAmp);
    UART_INITIALIZE();

    // Set up a forever loop using your favorite C-style 'for' loop
    while(1)  // loop while 1 equals 1
    {

        if(UART_RX_AVAILABLE())
        {
            // get incoming byte:
            uint8_t inByte = UART_RX_READ();

            if(inByte == 'l')
                PORTB |= (1 << outOpAmp);
            if(inByte == 'r')
                PORTB ~= (1 << outOpAmp);
        }
    }

    return 1;
}



