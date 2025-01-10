/*
 * MonopolyTestPorgramm.c
 *
 * Created: 09.01.2025 13:30:12
 * Author : e1Schnei
 */ 
#pragma GCC optimize 0
//Standardisierte Datentypen
#include <stdint.h>
//ATmega2560v I/O-Definitionen
#include <avr/io.h>
//CPU Clock Definition
#define F_CPU 16000000UL
//Wartefunktion von Atmel
#include <util/delay.h>

#include "MonopolyTreiber.h"




void PortInitialisierung(void)
{
    DDRA = 0xFF;		// Port A auf Ausgang initialisieren (alle Pins)
    DDRB = 0xFF;		// Port B auf Ausgang initialisieren (alle Pins)
    PORTB = 0b00100000; //Setzt Clear der Spieler Schieberegister auf 1
    DDRC = 0xFF;		// Port C auf Ausgang initialisieren (alle Pins)
    DDRD = 0xFF;		// Port D auf Ausgang initialisieren (alle Pins)
    DDRE = 0xFF;		// Port E auf Ausgang initialisieren (alle Pins)
    PORTE = 0b00010000; //Setzt Clear der Häuser schieberegister auf 1
    DDRF = 0xFF;		// Port F auf Ausgang initialisieren (alle Pins)
    DDRH = 0xFF;		// Port H auf Ausgang initialisieren (alle Pins)
    PORTH = 0x10;       //Setzt Clear der Siebensegmente schieberegister auf 1
    DDRJ = 0xFF;		// Port J auf Ausgang initialisieren (alle Pins)
    DDRK = 0x00;		// Port K auf Eingang initialisieren (alle Pins)
    DDRL = 0x00;		// Port L auf Eingang initialisieren (alle Pins)
}
uint8_t houses[14][8] =
{
    {1,1,1,1,1,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,1,0,0,0,0,0,0}
};
uint8_t spieler[20][8] = {0};
uint8_t spielerPos[4] = {0};

uint8_t siebensegment[16] = {0};
struct spielerStruct 
{
    uint16_t geld;
    uint8_t position;
};
int main(void)
{
    struct spielerStruct spieler1 = {1234,9};
    struct spielerStruct spieler2 = {5678,10};
    struct spielerStruct spieler3 = {9012,11};
    struct spielerStruct spieler4 = {3456,12};
    PortInitialisierung();
    SPI_init_all(9600);

    setPropertyRgb(2,0,0,0);
    setPropertyRgb(10,0,0,0);
    setPropertyRgb(20,0,0,0);
    setPlayerPosition(spieler1.position,1);
    setPlayerPosition(spieler2.position,2);
    setPlayerPosition(spieler3.position,3);
    setPlayerPosition(spieler4.position,4);
    
    setGeld(spieler1.geld,1);
    setGeld(spieler2.geld,2);
    setGeld(spieler3.geld,3);
    setGeld(spieler4.geld,4);
    while (1)
    {
        //setPlayerPosition(39,4);
        /*for (uint8_t i = 0; i <= 5; i = i + 1)
        {
            for (uint8_t j = 0; j < 22; j = j + 1)
            {
                setHouse(j,i);
                _delay_ms(10);
                writeHouse(houses);
            }
        }
        for (uint8_t i = 6; i > 0; i = i - 1)
        {
            for (uint8_t j = 0; j < 22; j = j + 1)
            {
                setHouse(j,i-1);
                _delay_ms(10);
                writeHouse(houses);
            }
        }*/
        /*for (uint8_t j = 1; j < 5; j = j + 1)
        {
            for (uint8_t i = 0; i < 40; i = i + 1)
            {
                setPlayerPosition(i,j);
                _delay_ms(10);
            }
        }*/
    }
}

