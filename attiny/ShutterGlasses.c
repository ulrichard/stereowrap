
#include "uart.h"
#include <avr/io.h>
#include <util/delay.h>

#undef UART_RX_PIN
#undef UART_RX_INTERRUPT
#undef UART_RX_INTERRUPT_PORT

// ATMEL ATTINY45 / ATTINY85
//                                    +-\/-+
// PCINT5/!RESET/ADC0/dW        PB5  1|    |8  Vcc
// PCINT3/XTAL1/CLKI/!OC1B/ADC3 PB3  2|    |7  PB2 SCK/USCK/ADC1/T0/INTO/PCINT2
// PCINT4/XTAL2/CLKO/OC1B/ADC2  PB4  3|    |6  PB1 MISO/D0/OC0B/OC1A/PCINT1            pwm1
//                              GND  4|    |5  PB0 MOSI/D1/SDA/AIN0/!OC0A/AREF/PCINT0  pwm0
//                                    +----+
//                  +-\/-+
//                 1|    |8  Vcc
//                 2|    |7
// output opamp <- 3|    |6
//            GND  4|    |5  <- uart rx
//                  +----+



int main(void)
{
    DDRB = (1 << PIN_OUT_OPAMP) | (1 << PB4); // declare output pins
    UART_INITIALIZE();

    // Set up a forever loop using your favorite C-style 'for' loop
    while(1)  // loop while 1 equals 1
    {
/*
        if(UART_RX_AVAILABLE())
        {
            // get incoming byte:
            uint8_t inByte = UART_RX_READ();

            if(inByte == 'l')
                PORTB |= (1 << PIN_OUT_OPAMP);
            else //if(inByte == 'r')
                PORTB &= ~(1 << PIN_OUT_OPAMP);
        }
*/
    }

    return 1;
}


