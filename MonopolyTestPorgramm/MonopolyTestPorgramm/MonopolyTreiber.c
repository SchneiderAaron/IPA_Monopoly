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
#include "LCD.h"

#include <string.h>

#include <stdlib.h>

#include <stdio.h>
rgb_color leds[LED_COUNT];

#pragma GCC optimize 0

#define USART_HAUS_SCHIEBEREGISTER 0
#define USART_GELD_SCHIEBEREGISTER 2
#define HAUS_SCHIEBEREGISTER_LATCH_PIN 0b00001000
#define GELD_SCHIEBEREGISTER_LATCH_PIN 0x08
#define MAX_ANZAHL_HAEUSER 5
#define ANZAHL_HAUS_SCHIEBEREGISTER 14
#define SPIELER_POSITION_LATCH_PIN 0x10
/******************************************************************************\
* resetMonopoly
*
* Setzt das Monopoly-Spiel zurück, indem alle Spielerpositionen auf das Startfeld
* gesetzt werden und alle Immobilien (Häuser) zurückgesetzt werden.
*
* Rückgabewert: Keine Rückgabe
*
\******************************************************************************/
void resetMonopoly(void)
{
    //Setze das Haus-Register zurück
    writeHaus(hausRegister);

    //Setze die Position der Spieler
    setPlayerPosition(0, 1);
    setPlayerPosition(0, 2);
    setPlayerPosition(0, 3);
    setPlayerPosition(0, 4);

    //Setze alle Grundstücke auf RGB-Wert 0 (zurücksetzen)
    for (uint8_t i = 0; i < 28; i = i + 1)
    {
        setPropertyRgb(i, 0);
    }
}
/******************************************************************************\
* setHaus
*
* Setzt die Anzahl Häuser auf einem Feld
* 
*
* Parameter:
* FeldNr = Nummer des Spielfelds, auf dem die Häuser gesetzt werden
* anzahlHaus = Anzahl der Häuser, die auf dem Feld gebaut sind
*
* Rückgabewert: Keine Rückgabe
*
\******************************************************************************/
void setHaus(uint8_t FeldNr, uint8_t anzahlHaus)
{
    uint8_t anzahlLeds, startRegister, startLed, hausWert = 0;
    //Berechnet die Anzahl der LEDs für das angegebene Feld
    anzahlLeds = FeldNr * 5;
    //Berechnet das Schieberegister, über das die LEDs angesteuert werden
    startRegister = anzahlLeds / 8;
    //Berechnet die Position des ersten Haus-Bits im Schieberegister
    startLed = (anzahlLeds % 8);

    //Wenn 5 Häuser gesetzt sind, wird nur das Hotel aktiviert
    if (anzahlHaus == 5)
    {
        hausWert = 0x10;
    }
    else
    {
        //Berechnet den Hauswert für weniger als 5 Häuser
        hausWert = ~(0xFFE0 >> (MAX_ANZAHL_HAEUSER - anzahlHaus));
    }

    //Überprüft, ob der Hauswert in das nächste Schieberegister überlappt
    if (startLed > 3)
    {
        //Setzt die 5 Bits im aktuellen Register auf 0
        hausRegister[startRegister] &= (~(0x1F << startLed));
        //Setzt die 5 Bits im nächsten Register auf 0
        hausRegister[startRegister + 1] &= ~0x1F >> (8 - startLed);
        
        //Füllt das erste Register mit den Hausbits
        hausRegister[startRegister] |= (uint8_t)(hausWert << startLed);
        //Füllt das nächste Register mit den verbleibenden Hausbits
        hausRegister[startRegister + 1] |= (uint8_t)(hausWert >> (8 - startLed));
    }
    else
    {
        //Setzt die 5 Bits im aktuellen Register auf 0
        hausRegister[startRegister] &= (~(0x1F << startLed));
        //Setzt die gewünschten Hausbits im aktuellen Register
        hausRegister[startRegister] |= (uint8_t)(hausWert << startLed);
    }

    //Gibt das aktualisierte Haus-Register aus
    writeHaus(hausRegister);
}
/******************************************************************************\
* writeHaus
*
* Überträgt die angegebenen 14 Byte an Daten an das Haus-Register zur Anzeige
* der anzahl Häuser auf dem Spielfeld.
*
* Parameter:
* data = Array von 14 Byte, das die Hausdaten enthält, die übertragen werden sollen
*
* Rückgabewert: Keine Rückgabe
*
\******************************************************************************/
void writeHaus(uint8_t data[14])
{
    //initialisierung der Variablen
    uint8_t transmitdata = 0;
    //im for loop werden jeweils 8 Bit aus dem array gesendet
    for(uint8_t i = 0; i < ANZAHL_HAUS_SCHIEBEREGISTER + 1; i = i + 1)
    {
        //Jeweils 8 Bit werden an transmitdata übergeben
        transmitdata = data[ANZAHL_HAUS_SCHIEBEREGISTER-i];
        //Ausgabe über USART 0
        USART_Transmit(USART_HAUS_SCHIEBEREGISTER,transmitdata); 
        //delay damit SPI funktiuniert
        _delay_us(500);
    }
    //latch
    PORTE = PORTE |  HAUS_SCHIEBEREGISTER_LATCH_PIN;
    PORTE = PORTE & ~HAUS_SCHIEBEREGISTER_LATCH_PIN;
}
/******************************************************************************\
* setPropertyRgb
*
* Setzt die RGB-Werte einer bestimmten Eigenschaft auf dem Spielfeld basierend
* auf der Feldnummer und der Spieler-Nummer.
*
* Parameter:
* FeldNummer = Nummer des Feldes auf dem Spielfeld
* spielerNr = Nummer des Spielers, dessen RGB-Werte gesetzt werden sollen
*
* Rückgabewert: Keine Rückgabe
*
\******************************************************************************/
void setPropertyRgb(uint8_t FeldNummer, uint8_t spielerNr)
{
    uint8_t rot,gruen,blau = 0; //Initialisierung der RGB Varîablen
    switch (spielerNr)//switch case zur bestimmung der RGB Werte anhand der Spielernummer
    {
        case 0://0 --> RGB Aus
        rot = 0;
        gruen = 0;
        blau = 0;
        break;
        case 1://1 --> RGB Rot
        rot = 50;
        gruen = 0;
        blau = 0;
    	break;
        case 2://2 --> RGB Grün
        rot = 30;
        gruen = 50;
        blau = 3;
        break;
        case 3://3 --> RGB Gelb
        rot = 50;
        gruen = 20;
        blau = 0;
        break;
        case 4://0 --> RGB Blau
        rot = 0;
        gruen = 0;
        blau = 50;
        break;
    }
    leds[FeldNummer] = (rgb_color){rot,gruen,blau}; //Setzt die RGB werte im leds Array
    for (uint8_t i = 0; i < LED_COUNT; i = i + 1)   //die for loop übermittelt die Daten an die WS2812 RGB Leds
    {
        led_strip_write(leds); //Ausgabe
    }
    
}

/******************************************************************************\
* setPlayerPosition
*
* Setzt die Position eines Spielers auf dem Spielfeld und aktualisiert die LED-Position.
*
* Parameter:
* feld = Neue Position des Spielers auf dem Spielfeld
* spielerNummer = Nummer des Spielers (1-4)
*
* Rückgabewert: Keine Rückgabe
*
\******************************************************************************/
void setPlayerPosition(uint8_t feld, uint8_t spielerNummer)
{
    uint8_t spielerRegister, startLed, spielerPositionAlt = 0;
    int8_t fehlerausgleich = 0;
    
    //Deaktiviert die LED der alten Position des Spielers
    spielerRegister = (spielerPos[spielerNummer - 1] * 4) / 8;
    startLed = (spielerPos[spielerNummer - 1] * 4) % 8;
    fehlerausgleich = spielerPosFehlerAusgleich(spielerNummer);
    spieler[spielerRegister][startLed + (spielerNummer - 1) + fehlerausgleich] = 0;
    
    //Aktualisiert die Spielerposition im Array
    spielerPos[spielerNummer - 1] = feld;

    //Aktiviert die LED der neuen Position des Spielers
    spielerRegister = (feld * 4) / 8;
    startLed = (feld * 4) % 8;
    fehlerausgleich = spielerPosFehlerAusgleich(spielerNummer);
    spieler[spielerRegister][startLed + (spielerNummer - 1) + fehlerausgleich] = 1;

    uint8_t transmitdata = 0;
    //Sendet die Position des Spielers über SPI, 8 Bits pro Durchgang
    for (uint8_t i = 0; i < 21; i = i + 1)
    {
        transmitdata = 0;
        for (uint8_t j = 0; j < 8; j = j + 1)
        {
            transmitdata = transmitdata << 1;
            transmitdata = (transmitdata | spieler[20 - i][7 - j]);
        }
        //Überträgt die Daten über SPI
        Send2SPI(transmitdata);
        //Kurze Verzögerung für SPI-Übertragung
        _delay_us(500);
    }

    //Latch, um die Position auf der Anzeige zu fixieren
    PORTB = PORTB |  SPIELER_POSITION_LATCH_PIN;
    PORTB = PORTB & ~SPIELER_POSITION_LATCH_PIN;
}


/******************************************************************************\
* spielerPosFehlerAusgleich
*
* Berechnet den Fehlerausgleich für die Spielerposition basierend auf der
* Spieler-Nummer, um Hardware zu korrigieren.
*
* Parameter:
* spielerNummer = Nummer des Spielers (1-4), für den der Fehlerausgleich berechnet wird
*
* Rückgabewert: Fehlerausgleichswert für den Spieler (int8_t)
*
\******************************************************************************/
int8_t spielerPosFehlerAusgleich(uint8_t spielerNummer)
{
    // Variable zur Speicherung des Fehlerausgleichs
    int8_t fehlerausgleich = 0;

    // Fall 1: Die Spielerposition ist im Bereich 0-9 (keine Korrektur notwendig)
    if(spielerPos[spielerNummer - 1] < 10)
    {
        fehlerausgleich = 0; // Keine Korrektur erforderlich
    }
    // Fall 2: Die Spielerposition liegt im Bereich 10-19
    else if((spielerPos[spielerNummer - 1] > 9) && (spielerPos[spielerNummer - 1] < 20))
    {
        switch (spielerNummer)
        {
            case 1:
            fehlerausgleich = 1; // Spieler 1 muss um 1 positioniert werden
            break;
            case 2:
            fehlerausgleich = 2; // Spieler 2 muss um 2 positioniert werden
            break;
            case 3:
            fehlerausgleich = -2; // Spieler 3 muss um -2 positioniert werden
            break;
            case 4:
            fehlerausgleich = -1; // Spieler 4 muss um -1 positioniert werden
            break;
        }
    }
    // Fall 3: Die Spielerposition liegt im Bereich 20-29
    else if((spielerPos[spielerNummer - 1] > 19) && (spielerPos[spielerNummer - 1] < 30))
    {
        switch (spielerNummer)
        {
            case 1:
            fehlerausgleich = 3; // Spieler 1 muss um 3 positioniert werden
            break;
            case 2:
            fehlerausgleich = 1; // Spieler 2 muss um 1 positioniert werden
            break;
            case 3:
            fehlerausgleich = -1; // Spieler 3 muss um -1 positioniert werden
            break;
            case 4:
            fehlerausgleich = -3; // Spieler 4 muss um -3 positioniert werden
            break;
        }
    }
    // Fall 4: Die Spielerposition liegt im Bereich 30-39
    else if((spielerPos[spielerNummer - 1] > 29) && (spielerPos[spielerNummer - 1] < 40))
    {
        switch (spielerNummer)
        {
            case 1:
            fehlerausgleich = 2; // Spieler 1 muss um 2 positioniert werden
            break;
            case 2:
            fehlerausgleich = -1; // Spieler 2 muss um -1 positioniert werden
            break;
            case 3:
            fehlerausgleich = 1; // Spieler 3 muss um 1 positioniert werden
            break;
            case 4:
            fehlerausgleich = -2; // Spieler 4 muss um -2 positioniert werden
            break;
        }
    }

    //Rückgabe des Fehlerausgleichs (Korrekturwert)
    return fehlerausgleich;
}

//Definition der Segmente des 7-Segment-Displays für die Anzeige von Ziffern
#define SEG_A       (1<<0)    //Segment A
#define SEG_B       (1<<1)    //Segment B
#define SEG_C       (1<<2)    //Segment C
#define SEG_D       (1<<3)    //Segment D
#define SEG_E       (1<<4)    //Segment E
#define SEG_F       (1<<5)    //Segment F
#define SEG_G       (1<<6)    //Segment G
#define DOPPELPUNKT (1<<7)    //Doppelpunkt (nicht in diesem Code verwendet)

//Array, das die Ziffern von 0 bis 9 für das 7-Segment-Display speichert.
//Jede Zahl ist eine Kombination der Segmente A-G, die für die Anzeige erforderlich sind.
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
    (SEG_A | SEG_B | SEG_C | SEG_F | SEG_G | SEG_D),                            //9
    (0)                                                                         //10 (unsichtbar, z.B. für ein Leerzeichen oder Null)
};


/******************************************************************************\
* setGeld
*
* Setzt den Geldbetrag eines Spielers und steuert das Siebensegment-Display,
* je nachdem, ob es ein- oder ausgeschaltet wird.
*
* Parameter:
* geld = Der Geldbetrag des Spielers, der gesetzt werden soll
* spieler = Die Nummer des Spielers (1-4), dessen Geldbetrag gesetzt wird
* siebensegmentOnOff = Steuerung des Siebensegment-Displays (1 = an, 0 = aus)
*
* Rückgabewert: Keine Rückgabe
*
\******************************************************************************/
void setGeld(uint16_t geld, uint8_t spieler, uint8_t siebensegmentOnOff)
{
    //Variablen zur Berechnung der einzelnen Ziffern (Tausender, Hunderter, Zehner, Einer)
    uint8_t tausender, hunderter, zehner, einer, transmitdata = 0;

    //Berechnen der einzelnen Ziffern
    tausender = (geld / 1000) % 10;    // Tausender
    hunderter = (geld / 100) % 10;     // Hunderter
    zehner    = (geld / 10) % 10;      // Zehner
    einer     = geld % 10;             // Einer

    //Wenn das Siebensegment-Display eingeschaltet werden soll (siebensegmentOnOff = 1)
    if (siebensegmentOnOff)
    {
        //Die entsprechenden Ziffern für den Spieler im Array "siebensegment" setzen
        siebensegment[((spieler - 1) * 4)]      = ziffer[tausender]; // Tausender
        siebensegment[((spieler - 1) * 4) + 1]  = ziffer[hunderter]; // Hunderter
        siebensegment[((spieler - 1) * 4) + 2]  = ziffer[zehner];    // Zehner
        siebensegment[((spieler - 1) * 4) + 3]  = ziffer[einer];     // Einer
    }
    else
    {
        //Wenn das Siebensegment-Display ausgeschaltet werden soll (siebensegmentOnOff = 0),
        //alle Ziffern auf den Wert "10" setzen (unsichtbar, also keine Anzeige)
        siebensegment[((spieler - 1) * 4)]      = ziffer[10];  //Unsichtbar
        siebensegment[((spieler - 1) * 4) + 1]  = ziffer[10];  //Unsichtbar
        siebensegment[((spieler - 1) * 4) + 2]  = ziffer[10];  //Unsichtbar
        siebensegment[((spieler - 1) * 4) + 3]  = ziffer[10];  //Unsichtbar
    }

    //Ausgabe der Daten an das Siebensegment-Display
    //Es wird von hinten nach vorne durch das "siebensegment" Array iteriert (16 bis 0)
    //Dies könnte genutzt werden, um die Anzeige der Segmente zu steuern, z.B. mit SPI oder einem anderen Kommunikationsprotokoll
    for (uint8_t i = 0; i < 17; i++)
    {
        //Die Daten an das Display senden (hier über USART_Transmit mit Indexierung des Arrays)
        USART_Transmit(USART_GELD_SCHIEBEREGISTER, siebensegment[16 - i]);
        _delay_us(500);  //Kurze Verzögerung, um sicherzustellen, dass die Daten korrekt übertragen werden
    }

    // Latch (Synchronisation des Displays)
    PORTH = PORTH | GELD_SCHIEBEREGISTER_LATCH_PIN;  // Setzt das Latch-Signal auf 1
    PORTH = PORTH & ~GELD_SCHIEBEREGISTER_LATCH_PIN; // Setzt das Latch-Signal auf 0, um die Änderungen anzuwenden
}

/******************************************************************************\
* updateKontostand
*
* Aktualisiert den Kontostand der Spieler basierend auf den übergebenen
* Spielerinformationen und aktiviert oder deaktiviert die Anzeige auf einem
* Siebensegment-Display je nach Spieleraktivität.
*
* Parameter:
* anzahlSpieler = Anzahl der aktiven Spieler im Spiel
* spielerInfo = Array von Spielern, das die aktuellen Informationen (einschließlich Kontostand) enthält
*
* Rückgabewert: Keine Rückgabe (void)
*
\******************************************************************************/
void updateKontostand(uint8_t anzahlSpieler, Spieler spielerInfo[5])
{
    //Iteriere über alle aktiven Spieler (bis anzahlSpieler)
    for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
    {
        //Setze den Geldbetrag für den Spieler i auf den Wert von spielerInfo[i].geld
        //Der dritte Parameter ist 1, was möglicherweise eine Aktivierung bedeutet
        setGeld(spielerInfo[i].geld, i, 1);
    }

    //Iteriere über die restlichen Spieler (die nicht aktiv sind)
    for (uint8_t i = anzahlSpieler + 1; i <= 4; i = i + 1)
    {
        //Setze den Geldbetrag für den Spieler i auf den Wert von spielerInfo[i].geld
        //Der dritte Parameter ist 0, was möglicherweise eine Deaktivierung bedeutet
        setGeld(spielerInfo[i].geld, i, 0);
    }
}

/******************************************************************************\
* wuerfel
*
* Generiert eine Zufallszahl im Bereich von 1 bis 6, die einen Würfelerwurf 
  simuliert.
*
* Rückgabewert: Zufallszahl (1 bis 6)
*
\******************************************************************************/
uint8_t wuerfel(void)
{
    uint8_t zufallszahl = 0;
    //Generiert eine Zufallszahl zwischen 1 und 6
    zufallszahl = (rand() % 6) + 1;
    return zufallszahl;
}

/******************************************************************************\
* sibensegmentWuerfel
*
* Simuliert eine Würfeln-Animation, indem es mehrfach Zufallszahlen generiert,
* diese anzeigt und am Ende die finalen Würfelergebnisse auf dem Display darstellt.
*
* Parameter: Keine
*
* Rückgabewert: Keine Rückgabe (void)
*
\******************************************************************************/
void sibensegmentWuerfel(void)
{
    char buffer[16];
    uint8_t zufallszahl1, zufallszahl2 = 0;
    
    //Simuliert mehrere Würfeln-Animationen
    for (uint8_t i = 0; i < 50; i = i + 1)
    {
        zufallszahl1 = wuerfel(); //Erste Zufallszahl
        zufallszahl2 = wuerfel(); //Zweite Zufallszahl
        
        //Zeigt die erste Zufallszahl auf dem Display an
        sprintf(buffer, "%u", zufallszahl1);
        writeText(0, 9, buffer);
        
        //Zeigt die zweite Zufallszahl auf dem Display an
        sprintf(buffer, "%u", zufallszahl2);
        writeText(1, 9, buffer);
        
        //Sendet die zweite Zufallszahl an das Display
        USART_Transmit(3, ziffer[zufallszahl2]);
        _delay_us(500);
        
        //Sendet die erste Zufallszahl an das Display
        USART_Transmit(3, ziffer[zufallszahl1]);
        _delay_us(500);
        
        //Aktualisiert die Anzeige
        PORTJ = PORTJ | 0x08;
        PORTJ = PORTJ & ~0x08;
        
        //Verzögert den nächsten Durchgang
        _delay_ms(15 + i * 3);
    }
    
    //Würfeln der finalen Zufallszahlen
    zufallszahl1 = wuerfel();
    zufallszahl2 = wuerfel();
    
    //Sendet die finale zweite Zufallszahl an das Display
    USART_Transmit(3, ziffer[zufallszahl2]);
    _delay_us(500);
    
    //Sendet die finale erste Zufallszahl an das Display
    USART_Transmit(3, ziffer[zufallszahl1]);
    _delay_us(500);
    
    //Aktualisiert die Anzeige
    PORTJ = PORTJ | 0x08;
    PORTJ = PORTJ & ~0x08;
    
    //Speichert die finalen Zufallszahlen im Array
    wuerfelArray[0] = zufallszahl1;
    wuerfelArray[1] = zufallszahl2;
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