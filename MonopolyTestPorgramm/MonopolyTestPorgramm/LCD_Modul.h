/*
 * LCD_Modul.h
 *
 * Created: 27.04.2021 08:41:51
 *  Author: Administrator
 */ 


#ifndef LCD_MODUL_H_
#define LCD_MODUL_H_

void ST7036_write_byte( char data );
void ST7036_write_command_byte( char data );
void ST7036_write_data_byte( char data );
void ST7036_init(void);
void ST7036_puts( char * string );
void ST7036_putc( char zeichen );
void ST7036_goto_xy(unsigned char xwert, unsigned char ywert);
void ST7036_clearline( unsigned char zeile );
void ST7036_clear( void );


#endif /* LCD_MODUL_H_ */