#ifndef _UART_H_
#define _UART_H_

// this code is copied from http://blog.ethanfrei.com/2011/08/attiny84-uart-receive-naive.html
// Copyright 2011  Ethan Frei

#include <avr/io.h>

#include <avr/interrupt.h>

#include <util/delay.h>

//--------------------DEFINES---------------------

//UART GENERAL INFORMATION

#define UART_BAUD 57600

#define UART_HALF_BIT_WIDTH_CLOCKS (((F_CPU / 1) / UART_BAUD) / 2)

#define UART_HALF_BIT_DELAY_US (1000000UL * UART_HALF_BIT_WIDTH_CLOCKS / F_CPU)

#define UART_STATUS_BUSY 0

#define UART_STATUS_RECEIVING 1

//UART RX INFORMATION

#define UART_RX_PIN PB0

#define UART_RX_INTERRUPT PCINT1

#define UART_RX_INTERRUPT_PORT PCIE

#define UART_RX_BUFFER_SIZE 32

//-------------------VARIABLES--------------------

//UART GENERAL VARIABLES

uint8_t uart_status_bitfield;

//UART RX VARIABLES

uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];

uint8_t uart_rx_buff_position;

uint8_t uart_rx_buff_end;

//--------------------METHODS--------------------

void UART_INITIALIZE();

void UART_INITIALIZE_RX();                 //ENABLES INTERRUPTS

uint8_t UART_RX_AVAILABLE();

uint8_t UART_RX_READ();

#endif
