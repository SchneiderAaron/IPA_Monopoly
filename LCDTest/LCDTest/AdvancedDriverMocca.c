/********************************************************************************\
* Dateiname: AdvancedDriverMocca.c
*
* Projekt  : uP-Basisausbildung AE2
* Hardware : Mocca-Board, ATmega2560v von Atmel
*
* Copyright: MSW, AE2
*
* Beschreibung:
* Port-Definitionen für uP-Ausbildung in der Werkstatt AE2:
* - PortA Bit 0..7 auf Ausgang (LED)
* - PortH Bit 3..5 auf Ausgang (RGB)
* - PortK Bit 0..7 auf Eingang (Schalter)
* - PortE Bit 4..7 auf Eingang (Taster)
* - PortG Bit 0..5 auf Ausgang (LCD)	
* - Timer 0 in Autoload-Modus setzen mit Grundfrequenz 15.625kHz
*
* Datum:      Autor:				Version, Grund der Änderung:
* 16.06.2015  Christian Marty		V1.0     Erstellung
* 16.06.2015  O.Schneider			V1.1     Bereitstellung für Unterricht
* 21.01.2020  Reto Messerli			V2.0     Erweiterungen, BugFixes und 
*											 allgemeines code-refactoring
* 02.07.2020  Dündar-Jenni, Bolli,  V2.1     Display-Reset-Fehler behoben:
*             Gysi, Roth                     Reset: 0V / !Reset: Tristate, weil
*                                            sonst 5V-Out auf 3.3V-Input
\********************************************************************************/
#ifndef F_CPU
#	define F_CPU 16000000
#endif


#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdarg.h>
#include "AdvancedDriverMocca.h"

void lcd_sid(uint8_t status);
void lcd_sclk(uint8_t status);
void write_lcd(bool rs, uint8_t value);

volatile uint16_t takt_5ms_zaehler=0;

//--------------------------------------------------------------------------------
// Advanced Driver Mocca initialisierung.
//
// was hier initialisiert wird:
// DDR-A,K,E,G,H 
// 5ms timer
// LCD 
// interrupts
//
// was hier nicht initialisiert wird:
// ADC
// RGB
// USB
// RS232
//--------------------------------------------------------------------------------
void adm_init()
{
	DDRA = 0xFF;	// LED Port als Ausgang
	DDRK = 0x00;	// Schalter Port als Eingang
	DDRE = 0x00;	// Taster als Eingang
	DDRG = 0x23;	// LCD port als Ausgang
	DDRH = 0x78;	// RGB, CTS als Ausgang	
	
	adm_wait_init_5ms_timer();
	
	sei();			// Global interrups aktivieren
	
	adm_LCD_init();
	adm_LCD_set_brightness(8);
}

//****************************** Warte-Funktionen ******************************//

//--------------------------------------------------------------------------------
// Timer 0 initialisirung
//--------------------------------------------------------------------------------
void adm_wait_init_5ms_timer(void)
{
	TCCR0A = 0b00000011; // Timer mode einstellungen -> Fast PWM mode
	TCCR0B = 0b00001101; // 16Mhz / 1024 = 15,635kHz = 64us
	TIMSK0 = 0b00000001; // Timer overflow Interrupt aktivieren
	OCR0A = 77;
}

//--------------------------------------------------------------------------------
// Timer 0 Overflow Interrupt ->  Wird alle 5ms aufgerufen
//--------------------------------------------------------------------------------
ISR(TIMER0_OVF_vect)
{
	if(takt_5ms_zaehler>0){
		takt_5ms_zaehler--;
	}
}

//--------------------------------------------------------------------------------
// 5ms Wartefunktion
//--------------------------------------------------------------------------------
void adm_wait_5ms(uint16_t faktor)
{
	while(faktor>0){
		_delay_ms(5);
		faktor--;
	}
}

//--------------------------------------------------------------------------------
// Wartet auf den nächsten (faktor * 5ms) intervall.
// Kann gebraucht werden um das Programm einfach 
// auf ein Mehrfaches von 5ms zu takten.
// Example: 
//	main(){
//		... programm initialisieren ...
//		while(1){
//			adm_wait_interval(100);  //taktet das programm auf 100*5ms=500ms
//			... weiterer programm code ...
//		}
//	} 
//--------------------------------------------------------------------------------
void adm_wait_interval(uint16_t faktor){
	while(takt_5ms_zaehler!=0);
	takt_5ms_zaehler=faktor;
}

//--------------------------------------------------------------------------------
// 1us Wartefunktion
//--------------------------------------------------------------------------------
void adm_wait_1us(uint16_t volatile faktor)
{
	while (faktor)
	{
		// 11 Taktzüklen keine operation, 
		// retliche 5 taktzyklen für while und faktor decrement
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		faktor --;
	}
}

//**************************** RS232 / USB -Treiber ****************************//

//--------------------------------------------------------------------------------
// RS232 Initialisiren
//--------------------------------------------------------------------------------
void adm_RS232_init (uint16_t baudratenregister)
{
	UCSR2B = 0b00011000; // RX und TX enable
	UCSR2C = 0b00000110; // Asynkrone USART, 8 Datenbits
	
	UBRR2 = baudratenregister;
}

//--------------------------------------------------------------------------------
// Ein einzelnes zeichen über RS232 Senden
//--------------------------------------------------------------------------------
void adm_RS232_send_byte(uint8_t zeichen)
{
	UDR2 = zeichen;
	
	while( (UCSR2A & 0b01000000) == 0); // Warten bis Byte gesendet
	UCSR2A |= 0b01000000;	//Byte gesendet Flagge löschen
}

//--------------------------------------------------------------------------------
// Einen Nullterminierten string über RS232 senden
//--------------------------------------------------------------------------------
void adm_RS232_send_string(const char*Text)
{
	uint8_t i = 0;
	
	while(Text[i])
	{
		adm_RS232_send_byte(Text[i]);
		i++;
	}
}

//--------------------------------------------------------------------------------
// USB Initialisiren
//--------------------------------------------------------------------------------
void adm_USB_init(void)//maybe rename to init uart 0
{
	UCSR0B = 0b00011000; // RX und TX enable
	UCSR0C = 0b00000110; // Asynkrone USART, 8 Datenbits

	// 9600 Baud
	UBRR0 = 103;
}

//--------------------------------------------------------------------------------
// Ein einzelnes zeichen über USB Senden
//--------------------------------------------------------------------------------
void adm_USB_send_byte(uint8_t zeichen)
{
	UDR0 = zeichen;
	
	while( (UCSR0A & 0b01000000) == 0); // Warten bis Byte gesendet
	UCSR0A |= 0b01000000;				//Byte gesendet Flagge löschen
}

//--------------------------------------------------------------------------------
// Einen Nullterminierten string über USB senden
//--------------------------------------------------------------------------------
void adm_USB_send_string(const char*Text)
{
	uint8_t i = 0;
	
	while(Text[i])
	{
		adm_USB_send_byte(Text[i]);
		i++;
	}
}

//**************************** A/D_Wandler-Treiber ****************************//

//--------------------------------------------------------------------------------
// Initialisirung des ADC
//--------------------------------------------------------------------------------
void adm_ADC_init(void)
{
	ADMUX  = 0x40;	//AVCC Als referenz
	DIDR0  = 0x0F;	// IO pins von Potentiometer deaktivieren
	// ADC einschalten, ADC clok = 16MHz / 128, Free runing mode
	ADCSRA = 0b10000111; 
}

//--------------------------------------------------------------------------------
// Eine ADC lesung tätigen am ausgewälten ADC-Pin.
//--------------------------------------------------------------------------------
uint16_t adm_ADC_read(uint8_t kanal)
{
	// Kanal definieren
	ADMUX&=0xf0;
	ADMUX|=kanal&0x07;		//write ls3b to ADMUX
	ADCSRB&=~0x08;
	ADCSRB|=kanal&0x08;		//write msb to ADCSRB
	
	ADCSRA |= _BV(ADSC);	 	// ADC Starten
	while(ADCSRA & _BV(ADSC));// Warten bis Messung abgeschllossen
	
	return ADC;
}


//****************************** RGB-LED-Treiber ******************************//

//--------------------------------------------------------------------------------
// Initialisirung des PWM der RGB-LED
//--------------------------------------------------------------------------------
void adm_RGB_init(void)
{
	adm_RGB_PWM(0,0,0);// RGB ausschalten bevor PWM aktiviert wird
	
	TCCR4A = 0b11111110; // Ausgänge als PWM Ausgang ,Fast PWM mode
	TCCR4B = 0b00011011; //  Clock / 64
	
	ICR4 = 1023;// 10bit pwm aufösung
}

//--------------------------------------------------------------------------------
// Setzt die RGB LED auf die ausgewählte farbe. Rot, Gruen, Blau: 0 - 1023
//--------------------------------------------------------------------------------
void adm_RGB_PWM(uint16_t Rot, uint16_t Gruen, uint16_t Blau)
{
	adm_RGB_red(Rot);
	adm_RGB_green(Gruen);
	adm_RGB_blue(Blau);
}

//--------------------------------------------------------------------------------
// Setzt die helligkeit der Rot-Komponente der RGB LED. Rot: 0 - 1023
//--------------------------------------------------------------------------------
void adm_RGB_red(uint16_t Rot)
{
	// Wenn PWM nicht inizialisirt ist Pin mit LED High oder Low setztzen
	if(Rot)	PORTH |= 0x08;
	else PORTH &= 0xF7;
	
	// Wenn PWM initialisirt ist wert an PWM register übergeben
	OCR4A = 1023 - Rot;
}

//--------------------------------------------------------------------------------
// Setzt die helligkeit der Gruen-Komponente der RGB LED. Gruen: 0 - 1023
//--------------------------------------------------------------------------------
void adm_RGB_green(uint16_t Gruen)
{
	// Wenn PWM nicht inizialisirt ist Pin mit LED High oder Low setztzen
	if(Gruen)	PORTH |= 0x10;
	else PORTH &= 0xEF;
	
	// Wenn PWM initialisirt ist wert an PWM register übergeben
	OCR4B = 1023 - Gruen;
}

//--------------------------------------------------------------------------------
// Setzt die helligkeit der Blau-Komponente der RGB LED. Blau: 0 - 1023
//--------------------------------------------------------------------------------
void adm_RGB_blue(uint16_t Blau)
{
	// Wenn PWM nicht inizialisirt ist Pin mit LED High oder Low setztzen
	if(Blau)	PORTH |= 0x20;
	else PORTH &= 0xDF;
	
	// Wenn PWM initialisirt ist wert an PWM register übergeben
	OCR4C = 1023 - Blau;
}

//******************************** LCD-Treiber ********************************//

//--------------------------------------------------------------------------------
// Ansteuerung von einzelen Bits fur LCD
//--------------------------------------------------------------------------------
inline void lcd_sid(uint8_t status)		// LCD Datenleitung
{
	if(status)	PORTG |= 0x01;
	else PORTG &= 0xFE;
}

inline void lcd_sclk(uint8_t status)		// LCD Taktleitung
{
	if(status)	PORTG |= 0x02;
	else PORTG &= 0xFD;
}

//--------------------------------------------------------------------------------
// LCD ein oder aus schalten
//
// Reset: PG2 = 0V, !Reset: PG2 = Tristate, sonst 5V-Out auf 3.3V-In.
// Pullup ist im Display verbaut.
//--------------------------------------------------------------------------------
void adm_LCD_on_off(bool status)		//LCD ein/Aus
{
    PORTG &= 0xFB;
    
	if(status)	DDRG &= 0xFB;
	else DDRG |= 0x04;
}

//--------------------------------------------------------------------------------
// LCD Hintergundbeleuchtung mit PWM dimmen (0 bis 100%) 
// Timer 0 muss initialisiert sein!
// helligkeit: 0-10 
//--------------------------------------------------------------------------------
void adm_LCD_set_brightness(uint8_t hellighkeit) 
{
	if(hellighkeit == 0)
	{
		//Wenn heligkeit 0 -> Hintergrundbeleuchtung ausschalten
		TCCR0A &= 0b11011111; 
	}
	else
	{
		TCCR0A |= 0b00100000;
		if(hellighkeit >10)hellighkeit = 10;
		OCR0B = (hellighkeit*8);
	}
}

//--------------------------------------------------------------------------------
// Initialisierung des LCD's
//--------------------------------------------------------------------------------
void adm_LCD_init(void)
{
	lcd_sclk(1);
	lcd_sid(0);
	
	adm_LCD_on_off(0);
	adm_wait_5ms(2);
	
	adm_LCD_on_off(1);
	adm_wait_5ms(10);
	
	write_lcd(true,0x34);      // set 8-Bit-Interface RE = 1
	write_lcd(true,0x09);      // 4-Zeilen-Modus, 5-Dot Font-Breite
	
	write_lcd(true,0x30);      // set 8-Bit-Interface RE = 0
	write_lcd(true,0x0C);      // Display ON, Cursor OFF

	adm_LCD_clear();		   // Clear Display
	
	write_lcd(true,0x07);      // Entry Mode

	adm_LCD_write_text(0,0,
		"  Advanced  Driver  \n"
		"       Mocca        \n"
		"+------------------+\n"
		" CM & OS & RM  2020 \n"
	);
}

//--------------------------------------------------------------------------------
// Schreibt ein Kommando oder ein Datenbyte (Zeichen) 
// zum LCD SPI (Serial Protokoll Interface)
//--------------------------------------------------------------------------------
void write_lcd(bool rs, uint8_t value)
{
	uint8_t i;
	// Manche Befehle müssen doppelt ausgeführt werden, damit die
	// minimale Pulslänge von 400ns eingehalten wird. (bei 20MHz)

	// Synchronisierung: 5x "1" senden
	lcd_sid(1);				// Daten-Bit = 1
	for(i=0;i<5; i++)
	{
		lcd_sclk(0);			// Synch-Bits senden
		lcd_sclk(1);
	}
	// R/W: 1=Read, 0=Write
	lcd_sid(0);				    // R/W = 0
	lcd_sclk(0);				// R/W-Bit senden
	lcd_sclk(1);

	// RS Register Selection: 0=Command, 1=Data
	lcd_sid(!rs);

	lcd_sclk(0);				// RS-Bit senden
	lcd_sclk(1);

	// End-Marke 0
	lcd_sid(0);
	
	lcd_sclk(0);				// END-Bit senden
	lcd_sclk(1);
	
	// Daten-Bit 0-3
	for(i=0;i<4; i++)
	{
		lcd_sclk(0);
		lcd_sid(value & 0x01);
		value >>= 1;
		lcd_sclk(0);
		lcd_sclk(1);
	}

	lcd_sid(0);		// 4x "0" senden
	for(i=0;i<4; i++)
	{
		lcd_sclk(0);
		lcd_sclk(1);
	}
	
	// Daten-Bit 4-7
	for(i=0;i<4; i++)
	{
		lcd_sclk(0);
		lcd_sid(value & 0x01);
		value >>= 1;
		lcd_sclk(0);
		lcd_sclk(1);
	}
	
	lcd_sid(0);			// 4x "0" senden
	for(i=0;i<4; i++)
	{
		lcd_sclk(0);
		lcd_sclk(1);
	}
	
	lcd_sid(1);
	
	// Write-Befehl auf 43us verlängern, 
	// damit minimale Execution-Time von 43us eingehalten ist.
	adm_wait_1us(43);
}

//--------------------------------------------------------------------------------
// Setzt den cursor auf die angegebene position
//--------------------------------------------------------------------------------
void set_cursor_position(uint8_t y_pos, uint8_t x_pos){
	write_lcd(true, (x_pos + (y_pos * 0x20)) | 0x80);
}

//--------------------------------------------------------------------------------
// Text an xy-Position ausgeben
// Falls im String ein '\n' gefunden wird, wird der rest des Strings auf 
// der nächsten Zeile Fortgeschrieben. 
//--------------------------------------------------------------------------------
void adm_LCD_write_text(uint8_t y_pos, uint8_t x_pos, const char *str)
{
	if(y_pos>=4) return;
	set_cursor_position(y_pos,x_pos);
	
	while (*str) {
		if (*str == '\n') {//go down 1 line and reset to starting position
			y_pos++;
			set_cursor_position(y_pos, x_pos);
		}
		else {//draw the charachter to the screen
			write_lcd(false, *str);
		}
		str++;//get the next charachter in the string
	}
}

//--------------------------------------------------------------------------------
// Wandelt eine zahl von 0-15 zu einem Hex ASCII zeichen um. 
//--------------------------------------------------------------------------------
char num_to_hex(uint8_t number){
	if(number>9){
		return 'A'-10+number;
	}
	return '0'+number;
}

//--------------------------------------------------------------------------------
// Dezimal Zahl an xy-Position ausgeben mit einer bestimmten Länge und einer 
// bestimmten Anzahl von Nachkomma-Stellen. Negative Zahlen funktionieren auch.
//--------------------------------------------------------------------------------
void adm_LCD_write_decimal(uint8_t y, uint8_t x, int32_t value,
					   uint8_t length, uint8_t nachkommastellen){
	if(y>=4) return;
	
	char buffer[13];//a max int 32 value can be 10 digits long 
	bool isNegative=value<0;
	
	if(isNegative){
		value=-value;//only deal with positive numbers
	}
	//falls ein punkt gezeichnet werden muss, ziehe das von der länge ab
	length-=nachkommastellen!=0;
	
	for(uint8_t i=0;i<length;i++){
		//müssen keine zahlen mehr in den string gespeichert werden?
		if(value==0&&i>nachkommastellen){ 
			if(isNegative){
				//gib dem string ein minus zeichen falls die zahl negativ ist
				buffer[length-i-1]='-';
				i++;
			}
			while(i<length){
				//überschreibe den restlichen string mit leer-zeichen
				buffer[length-i-1]=' ';
				i++;
			}
			break;
		}
		buffer[length-i-1]='0'+(value%10); //speichere das LSD in den string
		value/=10;
	}	
	
	set_cursor_position(y,x);
	for(int8_t i=0;i<length;i++){
		if(length-i==nachkommastellen){
			//zeichne den decimal-punkt an der richtigen stelle
			write_lcd(false,'.');
		}
		//zeichne den string auf das display
		write_lcd(false,buffer[i]);
	}
}

//--------------------------------------------------------------------------------
// Float Zahl an xy-Position ausgeben mit einer bestimmten Länge und einer
// bestimmten Anzahl von Nachkomma-Stellen. Negative Zahlen funktionieren auch.
//--------------------------------------------------------------------------------
void adm_LCD_write_float(uint8_t y, uint8_t x, float value, 
						 uint8_t length, uint8_t nachkommastellen){
	for(uint8_t i=0;i<nachkommastellen;i++){
		//multipliziere die zahl * 10 pro nachkommastelle 
		//und benutze dann die write decimal funktion
		value*=10;
	}
	adm_LCD_write_decimal(y,x,(int32_t)value,length,nachkommastellen);
}

//--------------------------------------------------------------------------------
// Hexadecimal Zahl an xy-Position ausgeben mit einer bestimmten Länge. 
// Mit writePrefix kann man bestimmen, ob vor der Hex-Zahl noch 0x steht.
//--------------------------------------------------------------------------------
void adm_LCD_write_hex(uint8_t y, uint8_t x, uint32_t value,
				   uint8_t length, bool writePrefix){
	if(length>8){//eine 32 bit zahl kann nicht länger als 8 hex zeichen sein
		length=8;
	}
	
	char buffer[8+1];
	
	for (uint8_t i=0;i<length;i++)//wandle die zahl zum ASCII string um
	{
		buffer[i]=num_to_hex((value>>((length-i-1)*4))&0x0f);
	}
	buffer[length]=0;//insert string end zeichen
	
	//zeichne den string aufs display
	if(writePrefix){
		adm_LCD_write_text(y,x,"0x");
		adm_LCD_write_text(y,x+2,buffer);
	}
	else{
		adm_LCD_write_text(y,x,buffer);
	}
}

//--------------------------------------------------------------------------------
// Binär Zahl an xy-Position ausgeben mit einer bestimmten Länge.
// Mit writePrefix kann man bestimmen, ob vor der Binär-Zahl noch 0b steht.
//--------------------------------------------------------------------------------
void adm_LCD_write_binary(uint8_t y, uint8_t x, uint16_t value,
					  uint8_t length, bool writePrefix){
	if(length>16){//eine 16 bit zahl kann nicht länger als 16 binär zeichen sein
		length=16;
	}
	
	char buffer[16+1];
	uint8_t i;
	
	for (i=0;i<length;i++)//wandle die zahl zum ASCII string um
	{
		buffer[i]='0'+((value>>(length-i-1))&1);
	}
	buffer[length]=0;//insert string end zeichen
	
	//zeichne den string aufs display
	if(writePrefix){
		adm_LCD_write_text(y,x,"0b");
		adm_LCD_write_text(y,x+2,buffer);
	}
	else{
		adm_LCD_write_text(y,x,buffer);
	}
}

//--------------------------------------------------------------------------------
// ASCII Charakter an xy-Position ausgeben
//--------------------------------------------------------------------------------
void adm_LCD_write_character(uint8_t y, uint8_t x, char value){
	set_cursor_position(y,x);
	write_lcd(false,value);
}

//--------------------------------------------------------------------------------
// Alle Zeichen auf dem LCD Löschen
//--------------------------------------------------------------------------------
void adm_LCD_clear(void)
{
	write_lcd(true,0x01);  // Clear Display
	_delay_ms(2);		   // 2ms warten, bis LCD gelöscht ist
	
	adm_LCD_write_text(0,0," ");	// Blödes Zeichen auf Display löschen
}

