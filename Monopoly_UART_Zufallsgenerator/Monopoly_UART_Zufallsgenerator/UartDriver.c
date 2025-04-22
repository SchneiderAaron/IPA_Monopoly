/*
 * UarttDriver.c
 *
 * Created: 25.03.2024 09:53:12
 *  Author: e1Schnei
 */ 
#include <avr/io.h>
#include "UartDriver.h"
#define MODE_SELECTOR_MIN 1 //Lowest number that can be selected in modeSelector
#define MODE_SELECTOR_MAX 2 //Highest number that can be selected in modeSelector

#define ASCII_Verschiebung 48

void init_Uart (uint8_t ubrr)
{
    //Set baud rate
    UBRR1H = (ubrr >>8);
    UBRR1L = ubrr;
    //Enable Receiver and Transmitter
    UCSR1B = (1<<RXEN1) | (1<<TXEN1);
    // 1StopBit / 8Bit data
    UCSR1C = (1<<USBS1) | (3<<UCSZ10);
}

void UART_Transmit (unsigned char data)
{
    //Wait for empty transmit buffer
    while (!(UCSR1A & (1<<UDRE1)));
    //Put the data into the buffer, sends the data
    UDR1 = data;
}


unsigned char UART_Receive (void)
{
    //Wait for data to be received
    while(!(UCSR1A & (1<<RXC1)));
    //Get and return received data from buffer
    return UDR1;
}


unsigned char UART_Receive_no_Delay (void)
{
    //Wait for data to be received
    if ((UCSR1A & (1<<RXC1)))
    {
        //Get and return received data from buffer
        return UDR1;
    }
    else
    {
        return 0;
    }
}


void UART_Text(char *text)
{
    for (uint8_t i = 0; text[i]; i = i + 1)
    {
        UART_Transmit(text[i]);
    }
}


uint16_t parserZahl (uint8_t *Array, uint8_t anzStellen)
{
    
    uint16_t parserOutput = 0;
    uint8_t multiplikator = 10;
    for (uint8_t i = 0; i < anzStellen; i = i + 1)
    {
        parserOutput = parserOutput * 10;
        parserOutput = parserOutput + (Array[i] - ASCII_Verschiebung);
        
    }
    return parserOutput;
}

