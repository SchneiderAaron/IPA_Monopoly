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
* Dateiname: MonopolyTreiber.c
*
* Projekt  : IPA_Monopoly
* Hardware : Monopoly-Board, ATmega2560v von Atmel
*
*
* Copyright: MSW, AE3
*
* Beschreibung:
* =============
* Treiber zur ansteuerung der Monopoly Hardware
*
* Verlauf:
* ========
* Datum:      Autor:         Version   Grund der Änderung:
* 10.01.2025  A.Schneider    V1.0      Neuerstellung
*
\*********************************************************************************/

#include "MonopolyTreiber.h"


void writeHouse(uint8_t data[14][8])
{
    uint8_t transmitdata = 0;
    uint8_t bitcounter = 0;
    for(uint8_t i = 0; i < 15; i = i + 1)
    {
        transmitdata = 0;
        for (uint8_t j = 0; j < 8; j = j + 1)
        {
            transmitdata = transmitdata << 1;
            transmitdata = (transmitdata | data[14-i][7-j]);
        }
        USART_Transmit(0,transmitdata);
        _delay_us(500);
    }
    PORTE = PORTE | 0b00001000;
    PORTE = PORTE & ~0b00001000;
}

void setHouse(uint8_t FeldNr, uint8_t anzahlHaus)
{
    uint8_t anzahlLeds,hausRegister,startLed = 0;
    anzahlLeds = FeldNr * 5;    //Berechnet wie viele leds/Bit bis zum gewünschten feld kommen
    hausRegister = anzahlLeds / 8;  //Berechnet über welches Schieberegister die Leds angesteuert werden
    startLed = (anzahlLeds % 8);    //Berechnet welches bit im Schieberegister das erste Haus auf dem Feld ist
    
    //Wenn 5 Häuser auf dem Feld stehen, wird nur das Hotel(rote Led) leuchten
    if (anzahlHaus == 5)
    {
        //Setzt alle Häuser auf 0
        for (uint8_t i = 0; i < 4; i = i + 1)
        {
            houses[hausRegister][startLed + i] = 0;
        }
        //Setzt das Hotel auf 1
        houses[hausRegister][startLed + 4] = 1;
    }
    else
    {
        //schaltet die gewünschte Anzahl Häuser ein
        for (uint8_t i = 0; i < anzahlHaus; i = i + 1)
        {
            houses[hausRegister][startLed + i] = 1;
        }
        //schaltet die restlichen Häuser aus
        for(uint8_t i = anzahlHaus; i < 5; i = i + 1)
        {
            houses[hausRegister][startLed + i] = 0;
        }
    }
    
    
}