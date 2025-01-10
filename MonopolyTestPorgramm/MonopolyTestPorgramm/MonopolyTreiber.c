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
* Dateiname: MonopolyTreiber.c
*
* Projekt  : IPA_Monopoly
* Hardware : Monopoly-Board, ATmega2560v von Atmel
*
*
* Copyright: MSW, AE3
*
* Beschreibung:
* =============
* Treiber zur ansteuerung der Monopoly Hardware
*
* Verlauf:
* ========
* Datum:      Autor:         Version   Grund der Änderung:
* 10.01.2025  A.Schneider    V1.0      Neuerstellung
*
\*********************************************************************************/

#include "MonopolyTreiber.h"
#include "SPI.h"
#include "ws2812.h"
rgb_color leds[LED_COUNT];


void writeHouse(uint8_t data[14][8])
{
    //initialisierung der VAriablen
    uint8_t transmitdata = 0;
    //im for loop werden jeweils 8 Bit aus dem array gesendet
    for(uint8_t i = 0; i < 15; i = i + 1)
    {
        transmitdata = 0;
        for (uint8_t j = 0; j < 8; j = j + 1)
        {
            transmitdata = transmitdata << 1;
            transmitdata = (transmitdata | data[14-i][7-j]);
        }
        USART_Transmit(0,transmitdata);
        //delay damit SPI funktiuniert
        _delay_us(500);
    }
    //latch
    PORTE = PORTE | 0b00001000;
    PORTE = PORTE & ~0b00001000;
}

void setHouse(uint8_t FeldNr, uint8_t anzahlHaus)
{
    uint8_t anzahlLeds,hausRegister,startLed = 0; // initialisierung der Variablen
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
//funktion um die RGB zu setzen um anzuzeigen wem die Immobilien gehören
void setPropertyRgb(uint8_t FeldNummer, uint8_t rot, uint8_t gruen, uint8_t blau)
{
    leds[FeldNummer] = (rgb_color){rot,gruen,blau}; //Setzt die RGB werte im leds Array
    for (uint8_t i = 0; i < LED_COUNT; i = i + 1)   //die for loop übermittelt die Daten an die WS2812 RGB Leds
    {
        led_strip_write(leds);
    }
    
}

void setPlayerPosition(uint8_t feld, uint8_t spielerNummer)
{
    uint8_t spielerRegister, startLed, spielerPositionAlt = 0;
    int8_t fehlerausgleich = 0;
    
    //schaltet die led der alten position aus
    spielerRegister = (spielerPos[spielerNummer - 1] * 4) / 8;
    startLed = (spielerPos[spielerNummer - 1] * 4) % 8;
    fehlerausgleich = spielerPosFehlerAusgleich(spielerNummer);
    spieler[spielerRegister][startLed + (spielerNummer - 1)+fehlerausgleich] = 0;
    
    //Speichert die neue Spielerposition
    spielerPos[spielerNummer - 1] = feld;
    //fehlerausgleich = spielerPosFehlerAusgleich(spielerNummer);
    //fehlerausgleich = spielerPosFehlerAusgleich(spielerNummer);
    
    //schaltet die LED der Neuen Position ein
    spielerRegister = (feld * 4) / 8;
    startLed = (feld * 4) % 8;
    fehlerausgleich = spielerPosFehlerAusgleich(spielerNummer);
    spieler[spielerRegister][startLed + (spielerNummer - 1)+fehlerausgleich] = 1;
    //initialisierung der VAriablen
    uint8_t transmitdata = 0;
    //im for loop werden jeweils 8 Bit aus dem array gesendet
    for(uint8_t i = 0; i < 21; i = i + 1)
    {
        transmitdata = 0;
        for (uint8_t j = 0; j < 8; j = j + 1)
        {
            transmitdata = transmitdata << 1;
            transmitdata = (transmitdata | spieler[20-i][7-j]);
        }
        Send2SPI(transmitdata);
        //delay damit SPI funktiuniert
        _delay_us(500);
    }
    //latch
    PORTB = PORTB | 0x10;
    PORTB = PORTB & ~0x10;
}

//funktion zur Korrektur von Hardwarefehler
int8_t spielerPosFehlerAusgleich(uint8_t spielerNummer)
{
    int8_t fehlerausgleich = 0;
    //in den Feldern 0 - 9 muss nichts korrigiert werden
    if(spielerPos[spielerNummer - 1] < 10)
    {
        fehlerausgleich = 0;
    }
    //Korrektur der felder 10-19
    else if((spielerPos[spielerNummer - 1] > 9) && (spielerPos[spielerNummer - 1] < 20))
    {
        switch (spielerNummer)
        {
            case 1:
            fehlerausgleich = 1;
            break;
            case 2:
            fehlerausgleich = 2;
            break;
            case 3:
            fehlerausgleich = -2;
            break;
            case 4:
            fehlerausgleich = -1;
            break;
        }
    }
    //Korrektur der felder 20-29
    else if((spielerPos[spielerNummer - 1] > 19) && (spielerPos[spielerNummer - 1] < 30))
    {
        
        switch (spielerNummer)
        {
            case 1:
            fehlerausgleich = 3;
            break;
            case 2:
            fehlerausgleich = 1;
            break;
            case 3:
            fehlerausgleich = -1;
            break;
            case 4:
            fehlerausgleich = -3;
            break;
        }
    }
    //Korrektur der felder 30-39
    else if((spielerPos[spielerNummer - 1] > 29) && (spielerPos[spielerNummer - 1] < 40))
    {
        
        switch (spielerNummer)
        {
            case 1:
            fehlerausgleich = 2;
            break;
            case 2:
            fehlerausgleich = -1;
            break;
            case 3:
            fehlerausgleich = 1;
            break;
            case 4:
            fehlerausgleich = -2;
            break;
        }
    }
    //gibt den korrektur Wert zurück
    return fehlerausgleich;
}


#define SEG_A       (1<<0)
#define SEG_B       (1<<1)
#define SEG_C       (1<<2)
#define SEG_D       (1<<3)
#define SEG_E       (1<<4)
#define SEG_F       (1<<5)
#define SEG_G       (1<<6)
#define DOPPELPUNKT (1<<7)
uint8_t ziffer[] =
{
    (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F),                            //0
    (SEG_B | SEG_C),                                                            //1
    (SEG_A | SEG_B | SEG_G | SEG_E | SEG_D),                                    //2
    (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G),                                    //3
    (SEG_F | SEG_G | SEG_B | SEG_C),                                            //4
    (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G),                                    //5
    (SEG_F | SEG_G | SEG_C | SEG_D | SEG_E | SEG_A),                            //6
    (SEG_A | SEG_B | SEG_C),                                                    //7
    (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G),                    //8
    (SEG_A | SEG_B | SEG_C | SEG_F | SEG_G | SEG_D)                             //9
};


void setGeld(uint16_t geld, uint8_t spieler)
{
    uint8_t tausender,hunderter,zehner,einer,transmitdata = 0;
    tausender   = (geld / 1000) % 10;
    hunderter   = (geld / 100)  % 10;
    zehner      = (geld / 10)   % 10;
    einer       =  geld         % 10;
    siebensegment[((spieler - 1) * 4)]      = ziffer[tausender];
    siebensegment[((spieler - 1) * 4) + 1]  = ziffer[hunderter];
    siebensegment[((spieler - 1) * 4) + 2]  = ziffer[zehner];
    siebensegment[((spieler - 1) * 4) + 3]  = ziffer[einer];
    //Ausgabe an siebensegment schieberegister
    for(uint8_t i = 0; i < 17; i = i + 1)
    {
        USART_Transmit(2,siebensegment[16-i]);
        _delay_us(500);
        //latch
        PORTH = PORTH | 0x08;
        PORTH = PORTH & ~0x08;
    }
    
    
}