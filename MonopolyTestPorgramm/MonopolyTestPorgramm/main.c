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
#include "SPI.h"
#include "MonopolyTreiber.h"


void PortInitialisierung(void)
{
    DDRA = 0xFF;		// Port A auf Ausgang initialisieren (alle Pins)
    DDRB = 0xFF;		// Port B auf Ausgang initialisieren (alle Pins)
    DDRC = 0xFF;		// Port C auf Ausgang initialisieren (alle Pins)
    DDRD = 0xFF;		// Port D auf Ausgang initialisieren (alle Pins)
    DDRE = 0xFF;		// Port E auf Ausgang initialisieren (alle Pins)
    DDRF = 0xFF;		// Port F auf Ausgang initialisieren (alle Pins)
    DDRH = 0xFF;		// Port H auf Ausgang initialisieren (alle Pins)
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
int main(void)
{
    
    PortInitialisierung();
    SPI_init_all(9600);
    PORTE = 0b00010000;
    
    
    
    while (1)
    {
        for (uint8_t i = 0; i <= 5; i = i + 1)
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
        }
    }
}

