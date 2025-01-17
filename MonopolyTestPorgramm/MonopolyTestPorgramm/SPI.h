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

#ifndef SPI_H_
#define SPI_H_

#define USART0 0
#define USART1 1
#define USART2 2
#define USART3 3

void SPI_init_all(uint16_t baud);
void Send2SPI (uint8_t wert);
void USART_Transmit(uint8_t usart_wahl, uint8_t data);
void USART0_Transmit(uint8_t data);

#endif /* SPI_H_ */