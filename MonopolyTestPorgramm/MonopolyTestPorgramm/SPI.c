/*********************************************************************************\
*
* MMMMMMMMMMMMMMMMMM   SSSSSSSSSSSSSSSSSS   WWWW   WWWW   WWWW
* MMMMMMMMMMMMMMMMMM   SSSSSSSSSSSSSSSSSS   WWWW   WWWW   WWWW   MECHATRONIK
* MMMMMMMMMMMMMMMMMM   SSSSS                WWWW   WWWW   WWWW
* MMMM   MMMM   MMMM   SSSSSSSSSSSSSSSSSS   WWWW   WWWW   WWWW   SCHULE
* MMMM   MMMM   MMMM   SSSSSSSSSSSSSSSSSS   WWWW   WWWW   WWWW
* MMMM   MMMM   MMMM                SSSSS   WWWWWWWWWWWWWWWWWW   WINTERTHUR
* MMMM   MMMM   MMMM   SSSSSSSSSSSSSSSSSS   WWWWWWWWWWWWWWWWWW
* MMMM   MMMM   MMMM   SSSSSSSSSSSSSSSSSS   WWWWWWWWWWWWWWWWWW   www.msw.ch
*
*
* Dateiname: main.c
*
* Projekt  : Monopoly
* Hardware : Monopoly-Board, ATmega2560v von Atmel
*
*
* Copyright: MSW, AE2
*
* Beschreibung:
* =============
* Programm für das Monopoly Board zum Monopoly spielen
*
* Verlauf:
* ========
* Datum:      Autor:         Version   Grund der Änderung:
* 29.06.2021  A. Ulrich      V1.0      Neuerstellung
*
\*********************************************************************************/

//Standardisierte Datentypen
#include <stdint.h>
//ATmega2560v I/O-Definitionen
#include <avr/io.h>
#include "SPI.h"

void SPI_init(void)
{
	// SPI einschalten und konfigurieren
	SPCR = SPCR | (1 << SPE) | (1 << MSTR) | (1 << SPR0);
	// SPI clock setzen
	SPSR = SPSR | (1 << SPI2X);
}

void SPI_init_all_USART(uint8_t baud)
{
	UBRR0 = 0;
	/* Set MSPI mode of operation and SPI data mode 0. */
	UCSR0C = (1<<UMSEL01)|(1<<UMSEL00)|(0<<2)|(0<<UCPOL0);
	/* Enable receiver and transmitter. */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);	
	UBRR0 = baud;
	
	UBRR1 = 0;
	/* Set MSPI mode of operation and SPI data mode 0. */
	UCSR1C = (1<<UMSEL11)|(1<<UMSEL10)|(0<<2)|(0<<UCPOL1);
	/* Enable receiver and transmitter. */
	UCSR1B = (1<<RXEN1)|(1<<TXEN1);
	UBRR1 = baud;
	
	UBRR2 = 0;
	/* Set MSPI mode of operation and SPI data mode 0. */
	UCSR2C = (1<<UMSEL21)|(1<<UMSEL20)|(0<<2)|(0<<UCPOL2);
	/* Enable receiver and transmitter. */
	UCSR2B = (1<<RXEN2)|(1<<TXEN2);
	UBRR2 = baud;
		
	UBRR3 = 0;
	/* Set MSPI mode of operation and SPI data mode 0. */
	UCSR3C = (1<<UMSEL31)|(1<<UMSEL30)|(0<<2)|(0<<UCPOL3);
	/* Enable receiver and transmitter. */
	UCSR3B = (1<<RXEN3)|(1<<TXEN3);
	UBRR3 = baud;
}

void SPI_init_all(uint8_t baud)
{
	SPI_init();
	SPI_init_all_USART(baud);
}

/*********************************************************************************\
* Send2SPI
*
* Ein Byte über SPI Senden
*
* Parameter:
* wert = zu sendender Wert
*
*
\*********************************************************************************/
void Send2SPI (uint8_t wert)
{
	SPDR = wert;					// 8Bits senden
	while(!(SPSR & (1 << SPIF)));	// warten bis sendung erfolgte
}

void USART0_Transmit(uint8_t data)
{
	/* Wait for empty transmit buffer */
	while (!( UCSR0A & (1<<UDRE0)));
	/* Put data into buffer, sends the data */
	UDR0 = data;
	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)));
}

void USART1_Transmit(uint8_t data)
{
	/* Wait for empty transmit buffer */
	while (!( UCSR1A & (1<<UDRE1)));
	/* Put data into buffer, sends the data */
	UDR1 = data;
	/* Wait for data to be received */
	while (!(UCSR1A & (1<<RXC1)));
}

void USART2_Transmit(uint8_t data)
{
	/* Wait for empty transmit buffer */
	while (!( UCSR2A & (1<<UDRE2)));
	/* Put data into buffer, sends the data */
	UDR2 = data;
	/* Wait for data to be received */
	while (!(UCSR2A & (1<<RXC2)));
}

void USART3_Transmit(uint8_t data)
{
	/* Wait for empty transmit buffer */
	while (!( UCSR3A & (1<<UDRE3)));
	/* Put data into buffer, sends the data */
	UDR3 = data;
	/* Wait for data to be received */
	while (!(UCSR3A & (1<<RXC3)));
}

void USART_Transmit(uint8_t usart_wahl, uint8_t data)
{
	switch (usart_wahl)
	{
		case 0:		USART0_Transmit(data);
					break;
		case 1:		USART1_Transmit(data);
					break;
		case 2:		USART2_Transmit(data);
					break;
		case 3:		USART3_Transmit(data);
		default:	break;
	}
}