#include "uart.h"

// this code is copied from http://blog.ethanfrei.com/2011/08/attiny84-uart-receive-naive.html
// Copyright 2011  Ethan Frei

void UART_INITIALIZE(){

        uart_rx_buff_end = 0;

        uart_rx_buff_position = 0;

        uart_status_bitfield = 0;

        for(uint8_t i = 0; i < UART_RX_BUFFER_SIZE; i++){

                uart_rx_buffer[i] = 0;

        }

        UART_INITIALIZE_RX();

}

void UART_INITIALIZE_RX(){

        PCMSK0 |= (1 << UART_RX_INTERRUPT);                        //Enable interrupts on RX PIN

        sei();

        GIMSK |= (1 << UART_RX_INTERRUPT_PORT);                //Enable interrupts period for PCI0 (PCINT7:0)

}

uint8_t UART_RX_AVAILABLE(){

        return uart_rx_buff_end != uart_rx_buff_position;

}

uint8_t UART_RX_READ(){

        uint8_t result = 0;

        if(UART_RX_AVAILABLE()){

                result = uart_rx_buffer[uart_rx_buff_position];

                uart_rx_buff_position++;

                uart_rx_buff_position = uart_rx_buff_position % UART_RX_BUFFER_SIZE;

        }

        return result;

}

//-------------------------INTERRUPTS-----------------------

ISR(PCINT0_vect){

        if(!(PINA & (1<<UART_RX_INTERRUPT))){                //if rx goes low

                GIMSK &= ~(1 << UART_RX_INTERRUPT_PORT);

                _delay_us(UART_HALF_BIT_DELAY_US * 3);

                for(uint8_t i = 0; i < 8; i += 1){

                        if(PINA & (1<<UART_RX_INTERRUPT)){        //RX PIN IS HIGH

                                uart_rx_buffer[uart_rx_buff_end] |= (1 << i);

                        }else{                                                                //RX PIN IS LOW

                                uart_rx_buffer[uart_rx_buff_end] &= ~(1 << i);

                        }

                        if(i != 7) _delay_us(UART_HALF_BIT_DELAY_US * 2);

                }

                //_delay_us(UART_HALF_BIT_DELAY_US * 2);                                //Get past stop bit

                uart_rx_buff_end++;

                uart_rx_buff_end= uart_rx_buff_end % UART_RX_BUFFER_SIZE;

                GIMSK |= (1 << UART_RX_INTERRUPT_PORT);

        }

}
