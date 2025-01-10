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

void writeHouse(uint8_t data[14][8]);
void setHouse(uint8_t FeldNr, uint8_t anzahlHaus);
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