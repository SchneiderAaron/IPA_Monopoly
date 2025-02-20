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
* Dateiname: LCD.c
*
* Projekt  : IPA_Monopoly
* Hardware : Monopoly-Board, ATmega2560v von Atmel
*
*
* Copyright: MSW, AE3
*
* Beschreibung:
* =============
* Treiber zur ansteuerung des LCDs auf der Monopoly Hardware
* um den buchstben ü zu schreiben muss das zeichen š verwendet werden
* Verlauf:
* ========
* Datum:      Autor:         Version   Grund der Änderung:
* 10.01.2025  A.Schneider    V1.0      Neuerstellung
*
\*********************************************************************************/
/*
„ = ä
Ž = Ä
” = ö
™ = Ö
š = ü
*/

#include <avr/io.h>
# define F_CPU 16000000UL
#include <util/delay.h>
#include <stdint.h>
//#include "AdvancedDriverMocca.h"

// Ports definieren


#define LCD_PORT_DATA	      PORTD
#define LCD_PORT_CONTROL	  PORTC
#define LCD_DDR_DATA	      DDRD
#define LCD_DDR_CONTROL       DDRC

// Enable / !Disable
#define ENABLE  (LCD_PORT_CONTROL |= 2)
#define DISABLE (LCD_PORT_CONTROL &= ~2)

// Read / !Write
#define READ  (LCD_PORT_CONTROL |= 4)
#define WRITE (LCD_PORT_CONTROL &= ~4)

// CSB-Signal
#define CHIP_DESELECT  (LCD_PORT_CONTROL |= 1)
#define CHIP_SELECT    (LCD_PORT_CONTROL &= ~1)

// RS-Signal
#define DATA    (LCD_PORT_CONTROL |= 16)
#define COMMAND (LCD_PORT_CONTROL &= ~16)

// !Reset LCD
#define	RESET_OFF	(LCD_PORT_CONTROL |= 32)
#define	RESET_ON	(LCD_PORT_CONTROL &= ~32)

// Display Commandos
#define CMD_CLEAR_DISPLAY    0x01
#define CMD_RETURN_HOME      0x02
#define CMD_ENTRY_MODE_SET   0x04
#define CMD_DISPLAY_ON_OFF   0x08
#define CMD_FUNCTION_SET     0x20
#define CMD_SET_DDRAM_ADRESS 0x80

void initDisplay(void)
{
    LCD_DDR_DATA    = 0xFF;
    LCD_DDR_CONTROL = 0xFF;
    RESET_OFF;
    _delay_ms(50);
    CHIP_SELECT;
    uint8_t initCMD[9] = {0x39, 0x1C, 0x52, 0x69, 0x74, 0x38, 0x0F, 0x01, 0x06};
    for (uint8_t Command = 0; Command < 9; Command++)
    {
        CmdDisplay(initCMD[Command]);
    }
    _delay_ms(1);
}

void CmdDisplay(uint8_t Cmd)
{
    COMMAND;
    WRITE;
    LCD_PORT_DATA = Cmd;
    ENABLE;
    if ((Cmd == 1) || (Cmd == 2) || (Cmd == 3))
    _delay_us(1100); //1100
    else
    _delay_us(30); //30
    DISABLE;
}

void DataDisplay(uint8_t Data)
{
    DATA;
    WRITE;
    LCD_PORT_DATA = Data;
    ENABLE;
    _delay_us(30);
    DISABLE;
}

void clear(void)
{
    CmdDisplay(CMD_CLEAR_DISPLAY);
    _delay_ms(1);
}

void home(void)
{
    CmdDisplay(CMD_RETURN_HOME);
    _delay_ms(2);
}

void displayOnOff(uint8_t DisplayOn,uint8_t CursorOn, uint8_t BlinkOn)
{
    BlinkOn   = (BlinkOn   << 0) & 0x01;
    CursorOn  = (CursorOn  << 1) & 0x02;
    DisplayOn = (DisplayOn << 2) & 0x04;
    CmdDisplay(CMD_DISPLAY_ON_OFF | BlinkOn | CursorOn | DisplayOn);
    _delay_ms(1);
}

void shift(void)
{
}



void writeText(uint8_t Zeile, uint8_t Spalte, const char *Text)
{
    CmdDisplay(CMD_SET_DDRAM_ADRESS + (0x10 * Zeile) + Spalte);
    uint8_t i = 0;
    while(Text[i])
    {
        DataDisplay(Text[i]);
        i++;
    }
    _delay_ms(2);
}
void displayCharacterAt(uint8_t zeile, uint8_t spalte, uint8_t charAddress)
{
    // Setze die DDRAM-Adresse basierend auf Zeile und Spalte
    CmdDisplay(CMD_SET_DDRAM_ADRESS + (0x10 * zeile) + spalte);

    // Sende die Character-Adresse an das Display
    DataDisplay(charAddress);
}
void defineCustomCharacters(void) 
{
    // Up Arrow
    CmdDisplay(0x48); // Set CGRAM address to 16
    DataDisplay(0b00100);
    DataDisplay(0b01110);
    DataDisplay(0b10101);
    DataDisplay(0b00100);
    DataDisplay(0b00100);
    DataDisplay(0b00100);
    DataDisplay(0b00000);
    DataDisplay(0b00000);
    // Down Arrow
    CmdDisplay(0x50); // Set CGRAM address to 24
    DataDisplay(0b00000);
    DataDisplay(0b00100);
    DataDisplay(0b00100);
    DataDisplay(0b00100);
    DataDisplay(0b10101);
    DataDisplay(0b01110);
    DataDisplay(0b00100);
    DataDisplay(0b00000);
}

void lcdInitAll(void)
{
    initDisplay();
    clear();
    home();
    defineCustomCharacters();
    displayOnOff(1,0,0);
}
void lcdReInit(void)
{
    RESET_ON;  // Aktiver Reset (Low)
    _delay_ms(10);
    RESET_OFF; // Reset deaktivieren (High)
    _delay_ms(10);
    initDisplay(); // Danach normal initialisieren
    clear();
    home();
    displayOnOff(1,0,0);
}

#define LCD_WIDTH 16 
#define MAX_TEXT_LEN 32
uint8_t lcdLauftext(const char *text, uint8_t schritt)
{
    char zeile1[17] = {0};
    char zeile2[17] = {0};
    int textLen = strlen(text);
    strncpy(zeile1,text + (schritt * 16),16);
    zeile1[16] =  '\0';
    if (textLen > 16 + (schritt * 16))
    {
        strncpy(zeile2,text + 16 + (schritt * 16),16);
        zeile2[16] = '\0';
    }
    clear();
    writeText(1,0,zeile1);
    writeText(2,0,zeile2);
    if ((schritt * 16) > textLen-32)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}