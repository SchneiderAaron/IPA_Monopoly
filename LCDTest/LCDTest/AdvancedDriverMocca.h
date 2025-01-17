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
* 02.07.2020  Dündar-Jenni, Bolli,  V2.1     Desplay-Reset-Fehler behoben:
*             Gysi, Roth                     Reset: 0V / !Reset: Tristate, weil
*                                            sonst 5V-Out auf 3.3V-Input
\********************************************************************************/

#ifndef advanced_mocca_define
#define advanced_mocca_define

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

// RGB Ports
#define RGB_R_EIN PORTH = (PORTH |  (1<<PH3))	
#define RGB_G_EIN PORTH = (PORTH |  (1<<PH4))
#define RGB_B_EIN PORTH = (PORTH |  (1<<PH5))
						  
#define RGB_R_AUS PORTH = (PORTH & ~(1<<PH3))
#define RGB_G_AUS PORTH = (PORTH & ~(1<<PH4))
#define RGB_B_AUS PORTH = (PORTH & ~(1<<PH5))

// LED Ports
#define LED_0_EIN PORTA = (PORTA |  (1<<PA0))
#define LED_1_EIN PORTA = (PORTA |  (1<<PA1))
#define LED_2_EIN PORTA = (PORTA |  (1<<PA2))
#define LED_3_EIN PORTA = (PORTA |  (1<<PA3))
#define LED_4_EIN PORTA = (PORTA |  (1<<PA4))
#define LED_5_EIN PORTA = (PORTA |  (1<<PA5))
#define LED_6_EIN PORTA = (PORTA |  (1<<PA6))
#define LED_7_EIN PORTA = (PORTA |  (1<<PA7))

#define LED_0_AUS PORTA = (PORTA & ~(1<<PA0))
#define LED_1_AUS PORTA = (PORTA & ~(1<<PA1))
#define LED_2_AUS PORTA = (PORTA & ~(1<<PA2))
#define LED_3_AUS PORTA = (PORTA & ~(1<<PA3))
#define LED_4_AUS PORTA = (PORTA & ~(1<<PA4))
#define LED_5_AUS PORTA = (PORTA & ~(1<<PA5))
#define LED_6_AUS PORTA = (PORTA & ~(1<<PA6))
#define LED_7_AUS PORTA = (PORTA & ~(1<<PA7))

// Schalter S0..S7
#define LESE_S0 (PINK & (1<<PINK0))
#define LESE_S1 (PINK & (1<<PINK1))
#define LESE_S2 (PINK & (1<<PINK2))
#define LESE_S3 (PINK & (1<<PINK3))
#define LESE_S4 (PINK & (1<<PINK4))
#define LESE_S5 (PINK & (1<<PINK5))
#define LESE_S6 (PINK & (1<<PINK6))
#define LESE_S7 (PINK & (1<<PINK7))

// Taster T1..T4
#define LESE_T1 (PINE & (1<<PINE4))
#define LESE_T2 (PINE & (1<<PINE5))
#define LESE_T3 (PINE & (1<<PINE6))
#define LESE_T4 (PINE & (1<<PINE7))


void adm_init();

void adm_wait_init_5ms_timer(void);
void adm_wait_5ms(uint16_t faktor);
void adm_wait_interval(uint16_t faktor);
void adm_wait_1us(uint16_t volatile faktor);

void adm_RS232_init(uint16_t baudratenregister);
void adm_RS232_send_byte(uint8_t zeichen);
void adm_RS232_send_string(const char*Text);

void adm_USB_init(void);
void adm_USB_send_byte(uint8_t zeichen);
void adm_USB_send_string(const char*Text);

void     adm_ADC_init(void);
uint16_t adm_ADC_read(uint8_t kanal);

void adm_RGB_init(void);
void adm_RGB_PWM(uint16_t Rot, uint16_t Gruen, uint16_t Blau);
void adm_RGB_red(uint16_t Rot);
void adm_RGB_green(uint16_t Gruen);
void adm_RGB_blue(uint16_t Blau);

void adm_LCD_init(void);
void adm_LCD_set_brightness(uint8_t hellighkeit);
void adm_LCD_on_off(bool status);
void adm_LCD_clear(void);
void adm_LCD_write_character(uint8_t y, uint8_t x, char value);
void adm_LCD_write_text(uint8_t y, uint8_t x, const char *str_ptr);
void adm_LCD_write_decimal(uint8_t y, uint8_t x, int32_t value, uint8_t length, uint8_t nachkommastellen);//vieleicht umbenennen?
void adm_LCD_write_float(uint8_t y, uint8_t x, float value, uint8_t length, uint8_t nachkommastellen);
void adm_LCD_write_hex(uint8_t y, uint8_t x, uint32_t value, uint8_t length, bool writePrefix);
void adm_LCD_write_binary(uint8_t y, uint8_t x, uint16_t value, uint8_t length, bool writePrefix);

#endif


