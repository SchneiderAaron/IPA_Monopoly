/*
 * Monopoly_ZufallsgeneratorDriver.c
 *
 * Created: 10.04.2025 10:18:53
 *  Author: e1Schnei
 */ 

#include <avr/io.h>

#include "Monopoly_ZufallsgeneratorDriver.h"
#include "UartDriver.h"


uint16_t ausgabeStartSequenz(void)
{
    uint8_t anzahlEingaben = 0;
    uint8_t empfangsdaten = 0;
    uint16_t anzahlMessungen = 0;
    char startSequenzString1[20][130] = {
    {"\n\r"},
    {"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r"},
    {"\n\r"},
    {"MM    MM  OOOOO  NN   NN  OOOOO  PPPPPP   OOOOO  LL      YY   YY\n\r"},
    {"MMM  MMM OO   OO NNN  NN OO   OO PP   PP OO   OO LL      YY   YY\n\r"},
    {"MM MM MM OO   OO NN N NN OO   OO PPPPPP  OO   OO LL       YYYYY\n\r"},
    {"MM    MM OO   OO NN  NNN OO   OO PP      OO   OO LL        YYY\n\r"},
    {"MM    MM  OOOO0  NN   NN  OOOO0  PP       OOOO0  LLLLLLL   YYY\n\r"},
    {"                                                                 \n\r"},
    {"ZZZZZ UU   UU FFFFFFF   AAA   LL      LL       SSSSS    GGGG  EEEEEEE NN   NN EEEEEEE RRRRRR    AAA   TTTTTTT  OOOOO  RRRRRR\n\r"},
    {"   ZZ UU   UU FF       AAAAA  LL      LL      SS       GG  GG EE      NNN  NN EE      RR   RR  AAAAA    TTT   OO   OO RR   RR\n\r"},
    {"  ZZ  UU   UU FFFF    AA   AA LL      LL       SSSSS  GG      EEEEE   NN N NN EEEEE   RRRRRR  AA   AA   TTT   OO   OO RRRRRR\n\r"},
    {" ZZ   UU   UU FF      AAAAAAA LL      LL           SS GG   GG EE      NN  NNN EE      RR  RR  AAAAAAA   TTT   OO   OO RR  RR\n\r"},
    {"ZZZZZ  UUUUU  FF      AA   AA LLLLLLL LLLLLLL  SSSSS   GGGGGG EEEEEEE NN   NN EEEEEEE RR   RR AA   AA   TTT    OOOO0  RR   RR\n\r"},
    {"\n\r"},
    {"                                       08.05.2025    IPA-Monopoly    Aaron Schneider                                         \n\r"},
    {"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r"},
    {"\n\r"},
    {"Wie viele Messungen sollen durchgefuehrt werden?\n\r"},    
    {"Anzahl Messungen = "}
    };
    
    char startSequenzString2[3][130] = {
    {"\n\r"},
    {"\n\r"},
    {"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r"}
    };
    
    for (uint8_t i = 0; i < 20; i = i + 1)
    {
        for (uint8_t j = 0; startSequenzString1[i][j] > 0; j = j + 1)
        {
            UART_Transmit(startSequenzString1[i][j]);
        }
    }
    while (anzahlEingaben < 4)
    {
        empfangsdaten = UART_Receive();
        if (empfangsdaten)
        {
            anzahlEingaben += 1;
            PORTA = anzahlEingaben;
            UART_Transmit(empfangsdaten);
            switch (anzahlEingaben)
            {
                case 1:
                anzahlMessungen += (empfangsdaten - ASCII_Verschiebung) * 1000;
                break;
                case 2:
                anzahlMessungen += (empfangsdaten - ASCII_Verschiebung) * 100;
                break;
                case 3:
                anzahlMessungen += (empfangsdaten - ASCII_Verschiebung) * 10;
                break;
                case 4:
                anzahlMessungen += (empfangsdaten - ASCII_Verschiebung);
                break;
            } 
        }
        
    }
    
    for (uint8_t i = 0; i < 3; i = i + 1)
    {
        for (uint8_t j = 0; startSequenzString2[i][j] > 0; j = j + 1)
        {
            UART_Transmit(startSequenzString2[i][j]);
        }
    }
    return (anzahlMessungen);    
}


uint8_t modusAuswahl()
{
    uint8_t anzahlEingaben = 0;
    uint8_t empfangsdaten = 0;
    char schriftzug[7][50] = {
        {"MM    MM  OOOOO  DDDDD   UU   UU  SSSSS  \n\r"},
        {"MMM  MMM OO   OO DD  DD  UU   UU SS      \n\r"},
        {"MM MM MM OO   OO DD   DD UU   UU  SSSSS  \n\r"},
        {"MM    MM OO   OO DD   DD UU   UU      SS \n\r"},
        {"MM    MM  OOOOO  DDDDDD   UUUUU   SSSSS  \n\r"},
        {"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r"},
        {"1 = Nur Seed        2 = Seed & ADC-Werte \n\r"},
    };
    
    for (uint8_t i = 0; i < 7; i = i + 1)
    {
        for (uint8_t j = 0; schriftzug[i][j] > 0; j = j + 1)
        {
            UART_Transmit(schriftzug[i][j]);
        }
    }
    while (!anzahlEingaben)
    {
        empfangsdaten = (UART_Receive() - ASCII_Verschiebung);
        if ((empfangsdaten == 1) || (empfangsdaten == 2) || (empfangsdaten == 3))
        {
            anzahlEingaben = 1;
        }
    }
    return (empfangsdaten);
}




/******************************************************************************\
* adm_ADC_init
*
* Initialisiert den ADC (Analog-Digital-Wandler) für die Verwendung im System.
* - Setzt die Referenzspannung auf AVCC.
* - Deaktiviert die digitalen Eingänge für die Pins des Potentiometers.
* - Startet den ADC im Free Running Mode mit einer Taktfrequenz von 16 MHz / 128.
*
* Parameter: Keine
*
* Rückgabewert: Keine Rückgabe (void)
*
\******************************************************************************/
void adm_ADC_init(void)
{
    ADMUX  = 0x40;	//AVCC Als referenz
    DIDR0  = 0x0F;	// IO pins von Potentiometer deaktivieren
    // ADC einschalten, ADC clok = 16MHz / 128, Free runing mode
    ADCSRA = 0b10000111;
}


/******************************************************************************\
* adm_ADC_read
*
* Liest einen Wert von einem angegebenen ADC-Kanal.
* - Setzt den Kanal im ADMUX-Register.
* - Startet die ADC-Messung und wartet auf deren Abschluss.
* - Gibt den gemessenen Wert zurück.
*
* Parameter:
* kanal = Der ADC-Kanal, von dem der Wert gelesen werden soll (0-7 für niedrige Kanäle,
*         8-15 für hohe Kanäle).
*
* Rückgabewert: Der 10-Bit ADC-Wert, der vom Kanal gelesen wurde.
*
\******************************************************************************/
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