#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


// ATMEL ATTINY45 / ATTINY85
//                                    +-\/-+
// PCINT5/!RESET/ADC0/dW        PB5  1|    |8  Vcc
// PCINT3/XTAL1/CLKI/!OC1B/ADC3 PB3  2|    |7  PB2 SCK/USCK/ADC1/T0/INTO/PCINT2
// PCINT4/XTAL2/CLKO/OC1B/ADC2  PB4  3|    |6  PB1 MISO/D0/OC0B/OC1A/PCINT1            pwm1
//                              GND  4|    |5  PB0 MOSI/D1/SDA/AIN0/!OC0A/AREF/PCINT0  pwm0
//                                    +----+
//                  +-\/-+
//                 1|    |8  Vcc
// crystal clock   2|    |7  -> debug LED
// crystal clock   3|    |6  -> op amp 
//            GND  4|    |5  <- uart rx
//                  +----+

//--------------------DEFINES---------------------
// pin definition
#define UART_RX_PIN            PB0
#define UART_RX_INTERRUPT      PCINT0
#define UART_RX_INTERRUPT_PORT PCIE
#define UART_RX_BUFFER_SIZE    32

#define PIN_OUT_OPAMP          PB1
#define PIN_OUT_DEBUG          PB2

// uart timing
#define UART_BAUD 38400
#define UART_HALF_BIT_WIDTH_CLOCKS (((F_CPU / 1) / UART_BAUD) / 2)
#define UART_HALF_BIT_DELAY_US (1000000UL * UART_HALF_BIT_WIDTH_CLOCKS / F_CPU)

// #define LED_DEBUGGING

int main(void)
{
    DDRB = (1 << PIN_OUT_OPAMP) | (1 << PIN_OUT_DEBUG); // declare output pins

    PCMSK |= (1 << UART_RX_INTERRUPT);       // Enable interrupts on RX PIN
    sei();                                   // Enable interrupts
    GIMSK |= (1 << UART_RX_INTERRUPT_PORT);  // Enable pin change interrupt for PORTB

    while(1)  // loop indefinitely doing nothing
        asm("NOP");

    return 1;
}

//-------------------------INTERRUPTS-----------------------
ISR(PCINT0_vect)
{
    if(!(PINB & (1<< UART_RX_PIN))) //if rx goes low
    {
        GIMSK &= ~(1 << UART_RX_INTERRUPT_PORT);
        _delay_us(UART_HALF_BIT_DELAY_US * 3);

        uint8_t inByte = 0;
        for(uint8_t i = 0; i < 8; ++i)
        {
            if(PINB & (1<< UART_RX_PIN)) //RX PIN IS HIGH
                inByte |= (1 << i);
            if(i != 7)
                _delay_us(UART_HALF_BIT_DELAY_US * 2);
        }

        // switch the shutter glasses
        if(inByte == 0xBC) // that's how the 'l' comes over
            PORTB |= (1 << PIN_OUT_OPAMP);
        else if(inByte == 0xB2) // that's how the 'r' comes over
            PORTB &= ~(1 << PIN_OUT_OPAMP);

#ifdef LED_DEBUGGING
        // show what we received in slow motion
        for(uint8_t i = 0; i < 8; ++i)
        {
            PORTB |= (1 << PIN_OUT_DEBUG); // enable debug LED
            _delay_us(200000);
            PORTB &= ~(1 << PIN_OUT_DEBUG); // disable debug LED
            _delay_us(200000);

            if(inByte & (1 << i))
                PORTB |= (1 << PIN_OUT_DEBUG); // enable debug LED
            else
                PORTB &= ~(1 << PIN_OUT_DEBUG); // disable debug LED

            _delay_us(1000000);
        }
#endif
        PORTB &= ~(1 << PIN_OUT_DEBUG); // disable debug LED

        GIMSK |= (1 << UART_RX_INTERRUPT_PORT);
    }
}
