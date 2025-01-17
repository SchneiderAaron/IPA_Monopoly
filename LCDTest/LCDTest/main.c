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
* Dateiname: main.c
*
* Projekt  : Test Display
* Hardware : Mocca-Board, ATmega2560v von Atmel
*
* Copyright: MSW, AE2
*
* Beschreibung:
* Über den PORTJ und PORTC wird das Display EA DOGM162W-A angesteuert
*
* Portbelegung:
* PORTJ   -> D0...D7 Daten
* PORTC.0 -> E      Enable
* PORTC.1 -> RW     Read !Write
* PORTC.2 -> CSB    !Chip Select
* PORTC.3 -> RS     !Command / Data
*
* Datum:      Autor:         Version   Grund der Änderung:
* 31.08.2020  O.Schneider    V1.0      Neuerstellung (läuft nicht)
* 06.10.2020  O.Schneider    V1.1      Übernahme von Michels Code und Anpassungen (lauffähig)
*
\*********************************************************************************/

#include <avr/io.h>
# define F_CPU 16000000UL
#include <util/delay.h>
#include <stdint.h>
#include "AdvancedDriverMocca.h"

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
}

void home(void)
{
    CmdDisplay(CMD_RETURN_HOME);
}

void displayOnOff(uint8_t DisplayOn,uint8_t CursorOn, uint8_t BlinkOn)
{
    BlinkOn   = (BlinkOn   << 0) & 0x01;
    CursorOn  = (CursorOn  << 1) & 0x02;
    DisplayOn = (DisplayOn << 2) & 0x04;
    CmdDisplay(CMD_DISPLAY_ON_OFF | BlinkOn | CursorOn | DisplayOn);
}

void shift(void)
{
}



void writeText(uint8_t Zeile, uint8_t Spalte, uint8_t * Text)
{
    CmdDisplay(CMD_SET_DDRAM_ADRESS + (0x10 * Zeile) + Spalte);
    /*if (Zeile)
    {
        CmdDisplay(CMD_SET_DDRAM_ADRESS + 0x10 + Spalte);
    }
    else
    {
        CmdDisplay(CMD_SET_DDRAM_ADRESS + Spalte);
    }*/
    
    uint8_t i = 0;
    while(Text[i])
    {
        DataDisplay(Text[i]);
        i++;
    }
}


int main(void)
{
    PORTC = 0xFF;
    _delay_ms(1000);
    LCD_DDR_CONTROL = 0xFF;
    _delay_ms(100);
    initDisplay();
    clear();
    home();
    displayOnOff(1,1,1);
    //adm_init();
    //adm_LCD_init();
    
    //  Portbelegung am Display anzeigen
    /*adm_LCD_clear();
    adm_LCD_write_text(0, 0, "Display Testsoftware");
    adm_LCD_write_text(1, 0, "EN -> PC0  R/W-> PC1");
    adm_LCD_write_text(2, 0, "CSB-> PC2  RS -> PC3");
    adm_LCD_write_text(3, 0, "RST-> PC4  Dat-> PJ" );*/
    
    /*writeText(0,5,"Oliver");
    writeText(1,0,"1234567890123456789");
    _delay_ms(3000);
    clear();
    _delay_ms(1);
    writeText(0,3,"nach Clear");
    _delay_ms(2000);
    writeText(1,3,"HOME");
    home();
    _delay_ms(2000);
    writeText(1,3,"Blink off");
    displayOnOff(1,1,0); // Blink  off
    _delay_ms(2000);
    writeText(1,3,"Cursor off");
    displayOnOff(1,0,0); // Cursor off
    _delay_ms(2000);
    displayOnOff(0,1,1); // Display off
    _delay_ms(2000);
    displayOnOff(1,1,1); // Display on
    writeText(1,3,"Display on");*/
    writeText(1,0,"Hallo Welt");
    _delay_ms(1000);
    displayOnOff(1,1,0);
    writeText(0,0,"Hallo Welt 2");
    _delay_ms(1000);
    writeText(0,0,"1234567890");
    _delay_ms(1000);
    clear();
    home();
    _delay_ms(1000);
    writeText(1,0,"Aaron");
    while (1)
    {
        
    }
}
