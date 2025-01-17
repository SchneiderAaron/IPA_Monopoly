/*
 * display.cpp
 *
 * Created: 22.12.2022 10:02:28
 *  Author: e9Birch
 */ 
#include "display.h"

#ifndef OBJECTIVE

// ports definieren
#define PORT_DATA			PORTD
#define PORT_CONTROL		PORTC
#define INIT_PORT_DATA		(DDRD = 0xFF)
#define INIT_PORT_CONTROL	(DDRC = 0b11111111)

// paralell / seriell Ansteuerung
#define PSB_TRUE			(PORT_CONTROL |= 0b00001000)
#define PSB_FALSE			(PORT_CONTROL &= 0b11110111)

// Enable / disable
#define ENABLE				(PORT_CONTROL |= 0b00000010)
#define DISABLE				(PORT_CONTROL &= 0b11111101)

// READ / ~WRITE
#define RW_TRUE				(PORT_CONTROL |= 0b00000100)
#define RW_FALSE			(PORT_CONTROL &= 0b11111011)

// CSB-Signal
#define CHIP_DESELECT		(PORT_CONTROL |= 0b00000001)
#define CHIP_SELECT			(PORT_CONTROL &= 0b11111110)

// RS-Signal
#define RS_TRUE				(PORT_CONTROL |= 0b00010000)
#define RS_FALSE			(PORT_CONTROL &= 0b11101111)

// Reset ganzes LCD
#define	RESET_OFF			(PORT_CONTROL |= 0b00100000)
#define	RESET_ON			(PORT_CONTROL &= 0b11011111)

/*********************************************************************************\
* CmdDisplay
*
* Gibt Befehle an den Display
*
* Parameter:
* Cmd = Welcher Befehl soll geschickt werden
*
\*********************************************************************************/
void CmdDisplay(uint8_t Cmd)
{
    RS_FALSE;
    RW_FALSE;
    PORT_DATA = Cmd;
    ENABLE;
    if ((Cmd == 1) || (Cmd == 2) || (Cmd == 3))
    {
        _delay_us(1100);
    }
    else
    {
        _delay_us(200);
    }
    DISABLE;
}

/*********************************************************************************\
* DataDisplay
*
* Schickt Daten an den Display
*
* Parameter:
* Data = Welche Daten sollen übertragen werden
*
\*********************************************************************************/
void DataDisplay(uint8_t Data)
{
    RS_TRUE;
    _delay_us(100);
    RW_FALSE;
    _delay_us(100);
    PORT_DATA = Data;
    _delay_us(100);
    ENABLE;
    _delay_us(100);
    DISABLE;
}

/*********************************************************************************\
* initCustomChar
*
* Initialisiert individuelle Zeichen für den Display die nicht im ASCII satz sind
*
\*********************************************************************************/
void _initCustomChar(void)
{
    uint8_t customChar[8][8] = {{0b1, 0b11, 0b11, 0b111, 0b111, 0b1111, 0b11111, 0b11111},
    {31, 31, 31, 31, 31, 31, 31, 31},
    {0b1, 0b11, 0b11, 0b111, 0b111, 0b1111, 0b11111, 0b11111},
    {31, 31, 31, 31, 31, 31, 31, 31},
    {31, 31, 31, 31, 31, 31, 31, 31},
    {31, 31, 31, 31, 31, 31, 31, 31},
    {31, 31, 31, 31, 31, 31, 31, 31},
    {31, 31, 31, 31, 31, 31, 31, 31},};
    for (uint8_t Char = 0; Char < 8; Char++)
    {
        for (uint8_t row = 0; row < 8; row ++)
        {
            CmdDisplay(64 + (Char << 3) + row);
            DataDisplay(customChar[Char][row]);
            _delay_us(200);
        }
    }
}

/*********************************************************************************\
* _initDisplay
*
* Initialisiert das Display damit Anzeige funktioniert und der Kontrast stimmt
*
\*********************************************************************************/
void _initDisplay(void)
{
    INIT_PORT_DATA;
    INIT_PORT_CONTROL;
    PSB_TRUE;
    RESET_OFF;
    CHIP_SELECT;
    _delay_ms(50);
    uint8_t initCMD[9] = {0x39, 0x1D, 0x50, 0x6C, 0x75, 0x38, 0x0C, 0x01,0x06};
    for (uint8_t Command = 0; Command < 9; Command++)
    {
        CmdDisplay(initCMD[Command]);
        _delay_us(100);
    }
    _initCustomChar();
}

/*********************************************************************************\
* _writeText
*
* Zeigt den entsprechenden Text auf dem Display an. Buchstaben müssen im ASCII
* satz vorhanden sein
*
* Parameter:
* Zeile = Auf Welcher Zeile soll angefangen werden zu schreiben
* Spalte = Auf Welcher Spalte soll angefangen werden zu schreiben
* Text = Welcher Text soll übertragen werden
*
\*********************************************************************************/
void _writeText(uint8_t Zeile, uint8_t Spalte, char* Text)
{
    if (Zeile == 2)
    {
        CmdDisplay(128 + 0x20 + Spalte);
    }
    else if (Zeile == 1)
    {
        CmdDisplay(128 + 0x10 + Spalte);
    }
    else
    {
        CmdDisplay(128 + Spalte);
    }
    
    uint16_t i = 0;
    while(Text[i])
    {
        DataDisplay(Text[i]);
        i++;
        _delay_us(500);
    }
}

/*********************************************************************************\
* _writeZeichen
*
* Schreibt ein spezielles Zeichen auserhalb des ASCII Satzes
*
* Parameter:
* Zeile = Auf Welcher Zeile soll angefangen werden zu schreiben
* Spalte = Auf Welcher Spalte soll angefangen werden zu schreiben
* Zeichen = Welches Zeichen soll geschrieben werden
*
\*********************************************************************************/
void _writeZeichen(uint8_t Zeile, uint8_t Spalte, uint8_t Zeichen)
{
    if (Zeile)
    {
        CmdDisplay(128 + 0x40 + Spalte);
    }
    else
    {
        CmdDisplay(128 + Spalte);
    }
    DataDisplay(Zeichen);
}

/*********************************************************************************\
* WriteNumber
*
* Schreibt ein Zahl
*
* Parameter:
* Zeile = Auf Welcher Zeile soll angefangen werden zu schreiben
* Spalte = Auf Welcher Spalte soll angefangen werden zu schreiben
* Zahl = Welche Zahl soll geschrieben werden
*
\*********************************************************************************/
void _writeZahl(uint8_t Zeile, uint8_t Spalte, uint16_t zahl)
{
    char send_buffer[4];


    sprintf(send_buffer,"%u",zahl);


    _writeText(Zeile, Spalte, send_buffer);
}

/*********************************************************************************\
* _clearDisplay
*
* Löscht das ganze Display
*
\*********************************************************************************/
void _clearDisplay(void)
{
    _writeText(0,0,(char*)"                ");
    _delay_us(500);
    _writeText(1,0,(char*)"                ");
    _delay_us(500);
    _writeText(2,0,(char*)"                ");
}


Display::Display(){
    _initDisplay();
}


void Display::writeText(uint8_t Row, uint8_t Column, char* Text){
    _writeText(Row,Column,Text);
}

void Display::writeText(uint8_t Row, uint8_t Column, const char* Text){
    _writeText(Row,Column,(char*)Text);
}

void Display::WriteChar(uint8_t Row, uint8_t Column, uint8_t Character){
    _writeZeichen(Row,Column,Character);
}
void Display::clearDisplay(){
    _clearDisplay();
}
void Display::WriteNumber(uint8_t Row, uint8_t Column, uint16_t Number){
    _writeZahl(Row,Column,Number);
}

#endif