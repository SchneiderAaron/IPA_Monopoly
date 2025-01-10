/* ---------------------------------------------------------------------

   Sample code for driving ST7036 on ELECTRONIC ASSEMBLY's DOG-Series
   (tested on ATmega8, EA DOGM163, AVR-GCC)
   *** NO FREE SUPPORT ON THIS PIECE OF CODE ***
   if you need an offer: mailto: programmer@demmel-m.de
   
   --------------------------------------------------------------------- */

//Serieller Betrieb von EA DOGModulen mit dem ST7036
#define F_CPU 8000000UL

#include <avr/delay.h>
#include <avr/io.h>


#define SETBITSI	PORTC = PORTC | 0b000001
#define CLEARBITSI	PORTC = PORTC & 0b111110
#define SETBITCLK	PORTC = PORTC | 0b000010
#define CLEARBITCLK PORTC = PORTC & 0b111101
#define false 0
#define true 1

/* ein Byte in das ???-Register des ST7036 senden */
void ST7036_write_byte( char data )
{
signed char	u8_zahl = 8;
char c_data;
	c_data = data;
	CLEARBITCLK;
	do
	{
		if ( c_data & 0x80 )
		{		// oberstes Bit von c_data betrachten
			SETBITSI;		// und Datenleitung entsprechend setzen
		}
		else
		{
			CLEARBITSI;
		}
		SETBITCLK;
		_delay_us(1);
		CLEARBITCLK;

		c_data = c_data << 1;
		u8_zahl --;

	} while (u8_zahl > 0);
}

/* ein Byte in das Control-Register des KS0073 senden */
void ST7036_write_command_byte( char data )
{
	ST7036_write_byte( data );
}

/* ein Byte in das Daten-Register des KS0073 senden */
void ST7036_write_data_byte( char data )
{
	ST7036_write_byte( data );
}



/* ein Byte in das Daten-Register des KS0073 senden */
void ST7036_init(void)
{


	ST7036_write_command_byte( 0x38);	// Bias Set; BS 1/5; 3 zeiliges Display /1d


	ST7036_write_command_byte( 0x39);	// Kontrast C3, C2, C1 setzen /7c


	ST7036_write_command_byte( 0x15 );	// Booster aus; Kontrast C5, C4 setzen /50

	ST7036_write_command_byte( 0x78 );	// Spannungsfolger und Verstärkung setzen /6c


	ST7036_write_command_byte( 0x5F );	// Display EIN, Cursor EIN, Cursor BLINKEN /0f

	ST7036_write_command_byte( 0x69);	// Display löschen, Cursor Home
	_delay_ms(200);		//

	ST7036_write_command_byte( 0x0C );	// Cursor Auto-Increment

	
	ST7036_write_command_byte( 0x06 );	// Cursor Auto-Increment

	
	ST7036_write_command_byte( 0x0F );	// Cursor Auto-Increment


}


// void ST7036_putsf( PGM_P string )
// {
// unsigned char zahl;
// zahl = 0;
// while (string[zahl] != 0x00)
// 	{
// 	ST7036_write_data_byte( string[zahl]);
// 	zahl++;
// 	}
// }

void ST7036_puts( char * string )
{
unsigned char zahl;
zahl = 0;
while (string[zahl] != 0x00)
	{
	ST7036_write_data_byte( string[zahl] );
	string ++;
	}
}

void ST7036_putc( char zeichen )
{
	ST7036_write_data_byte( zeichen );
}

// positioniert den Cursor auf x/y.
// 0/0 ist links oben, 2/15 ist rechts unten
void ST7036_goto_xy(unsigned char xwert, unsigned char ywert)
{
	ST7036_write_command_byte(0x80 + ywert*0x20 + xwert);
}


void ST7036_clearline( unsigned char zeile )
{
unsigned char zahl;
ST7036_goto_xy( 0, zeile );
for (zahl=1; zahl<20; zahl++) ST7036_write_data_byte( ' ' );
}

void ST7036_clear( void )
{
	ST7036_clearline( 0 );
	ST7036_clearline( 1 );
	ST7036_clearline( 2 );
}