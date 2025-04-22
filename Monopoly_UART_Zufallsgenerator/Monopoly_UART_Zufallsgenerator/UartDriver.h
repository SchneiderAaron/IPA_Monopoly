/*
 * UartDriver.h
 *
 * Created: 25.03.2024 09:53:31
 *  Author: e1Schnei
 */ 


#ifndef UARTDRIVER_H_
#define UARTDRIVER_H_


#include <util/delay.h>

void init_Uart (uint8_t ubrr);

void UART_Transmit (uint8_t data);

void parserGUI (uint8_t phraserStage);

void parserLogo (void);

void UART_Text(char *text);

uint16_t parserZahl (uint8_t *Array, uint8_t anzStellen);

unsigned char UART_Receive_no_Delay (void);

unsigned char UART_Receive (void);
#endif /* UARTDRIVER_H_ */