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



void writeText(uint8_t Zeile, uint8_t Spalte, const char *Text)
{
    CmdDisplay(CMD_SET_DDRAM_ADRESS + (0x10 * Zeile) + Spalte);
    uint8_t i = 0;
    while(Text[i])
    {
        DataDisplay(Text[i]);
        i++;
    }
}