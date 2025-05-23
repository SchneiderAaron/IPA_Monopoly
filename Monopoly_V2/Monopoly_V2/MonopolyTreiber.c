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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "MonopolyTreiber.h"
#include "SPI.h"
#include "ws2812.h"
#include "LCD.h"
//#pragma GCC optimize 0
/*--- #define-Konstanten und Makros -----------------------------------------*/
#define USART_HAUS_SCHIEBEREGISTER 0
#define USART_GELD_SCHIEBEREGISTER 2
#define HAUS_SCHIEBEREGISTER_LATCH_PIN 0b00001000
#define GELD_SCHIEBEREGISTER_LATCH_PIN 0x08
#define MAX_ANZAHL_HAEUSER 5
#define MAX_ANZAHL_SPIELER 4
#define ANZAHL_HAUS_SCHIEBEREGISTER 14
#define SPIELER_POSITION_LATCH_PIN 0x10
#define SPIELER_START_FELD 40 //Alle Spieler werden ausgeblendet
#define UNSICHTBARES_FELD 40 //Auf diesem Feld werden die Spieler nicht angezeigt
#define ANZAHL_GRUNDSTUEKE 28
#define SIEBENSEGMENT_OFF 10
#define ANZAHL_FELDER 40
#define BLAULICHT_OFF      ~0b11000000
#define BLAULICHT_LED1_ON   0b10000000
#define BLAULICHT_LED1_OFF ~0b10000000
#define BLAULICHT_LED2_ON   0b01000000
#define BLAULICHT_LED2_OFF ~0b01000000
#define ANZAHL_KONTO_SIEBENSEGMENTE 17

//Taster an PORT K
#define TASTE1     (1<<0)
#define TASTE2     (1<<1)
#define TASTE3     (1<<2)
#define TASTE4     (1<<3)
#define TASTE5     (1<<4)
#define TASTE6     (1<<5)
#define TASTE7     (1<<6)
#define TASTE8     (1<<7)

//Taster an PORT L
#define TASTE9     (1<<8)
#define TASTE10    (1<<9)
#define TASTE11    (1<<10)
#define TASTE12    (1<<11)
#define TASTE13    (1<<12)
#define TASTE14    (1<<13)
#define TASTE15    (1<<14)
#define TASTE16    (1<<15)

#define TASTE_A TASTE9  //Taste A
#define TASTE_B TASTE10 //Taste B
#define TASTE_C TASTE11 //Taste C

#define TASTE_O TASTE12 //Taste Hoch
#define TASTE_S TASTE13 //Taste Select
#define TASTE_U TASTE16 //Taste Runter
#define TASTE_L TASTE14 //Taste Links
#define TASTE_R TASTE15 //Taste Rechts

#define TASTE_X1 TASTE2 //Taste X Spieler 1
#define TASTE_X2 TASTE3 //Taste X Spieler 2
#define TASTE_X3 TASTE6 //Taste X Spieler 3
#define TASTE_X4 TASTE8 //Taste X Spieler 4

#define TASTE_Y1 TASTE1 //Taste Y Spieler 1
#define TASTE_Y2 TASTE4 //Taste Y Spieler 2
#define TASTE_Y3 TASTE5 //Taste Y Spieler 3
#define TASTE_Y4 TASTE7 //Taste Y Spieler 4
#define MAX_SCHRITTGROESSE 10
#define ANZAHL_FARBGRUPPEN 8
#define ANZAHL_FELDER_IN_FARBGRUPPE 3
#define ANZAHL_BIETER_INFORMATIONEN 6
#define GROSSVERSTEIGERUNG_SCHRITT_GROESSE 10
#define HOECHSTBIETENDER_SPIELER 5
#define ZURUECKGETRETENE_SPIELER 4
#define ANZAHL_KARTEN 17
#define FELDNUMER_WORKSHOP 10
//#define SIEBENSEGMENT_OFF 0
/*--- Datentypen (typedef) --------------------------------------------------*/
rgb_color leds[LED_COUNT];

/*--- Globale Konstanten ----------------------------------------------------*/
/*--- Globale Variablen -----------------------------------------------------*/
/*--- Modullokale Konstanten ------------------------------------------------*/
/*--- Modullokale Variablen -------------------------------------------------*/
pleite_t pleiteZustand = GENUG_GELD; 
uint8_t zufallsNummer = 0;
uint8_t anzahlPleiteSpieler = 0;
/*--- Prototypen modullokaler Funktionen ------------------------------------*/
/*--- Funktionsdefinitionen -------------------------------------------------*/
 
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
    setzeSpielerPosition(SPIELER_START_FELD, 1);
    setzeSpielerPosition(SPIELER_START_FELD, 2);
    setzeSpielerPosition(SPIELER_START_FELD, 3);
    setzeSpielerPosition(SPIELER_START_FELD, 4);

    //Setze alle Grundstücke auf RGB-Wert 0 (zurücksetzen)
    for (uint8_t i = 0; i < ANZAHL_GRUNDSTUEKE; i = i + 1)
    {
        setPropertyRgb(i, 0);
    }
    for (uint8_t i = 1; i <= MAX_ANZAHL_SPIELER; i = i + 1)
    {
        //Schaltet den Output aller Geld Schieberegister aus
        setGeld(0,i,SIEBENSEGMENT_OFF);
    }
    //Schaltet beide Würfel Siebensegmente aus
    wuerfelTransmit(SIEBENSEGMENT_OFF, SIEBENSEGMENT_OFF);
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
    uint8_t anzahlLeds = 0;
    uint8_t startRegister = 0;
    uint8_t startLed = 0;
    uint8_t hausWert= 0;
    
    //Berechnet die Anzahl der LEDs für das angegebene Feld
    anzahlLeds = FeldNr * 5;
    //Berechnet das Schieberegister, über das die LEDs angesteuert werden
    startRegister = anzahlLeds / 8;
    //Berechnet die Position des ersten Haus-Bits im Schieberegister
    startLed = (anzahlLeds % 8);
    
    if (anzahlHaus == 6)//Feld verpfändet
    {
        hausWert = 0x1F;
    }
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


void setHausAnimation(uint8_t FeldNr, uint8_t anzahlHaus, uint8_t ausgabe)
{
    uint8_t anzahlLeds = 0;
    uint8_t startRegister = 0;
    uint8_t startLed = 0;
    uint8_t hausWert= 0;
    if (!ausgabe)
    {
        //Berechnet die Anzahl der LEDs für das angegebene Feld
        anzahlLeds = FeldNr * 5;
        //Berechnet das Schieberegister, über das die LEDs angesteuert werden
        startRegister = anzahlLeds / 8;
        //Berechnet die Position des ersten Haus-Bits im Schieberegister
        startLed = (anzahlLeds % 8);
        
        if (anzahlHaus == 6)//Feld verpfändet
        {
            hausWert = 0x1F;
        }
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
    }
    else
    {
        writeHaus(hausRegister);
    }
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
    uint8_t rot = 0; //Initialisierung der RGB Varîablen
    uint8_t gruen = 0;
    uint8_t blau = 0;
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
        case 4://4 --> RGB Blau
        rot = 0;
        gruen = 0;
        blau = 50;
        break;
        case 5://5 --> verpfändet
        rot = 50;
        gruen = 50;
        blau = 50;
        break;
        case 9://9 --> Bank
        rot = 50;
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
void setzeSpielerPosition(uint8_t feld, uint8_t spielerNummer)
{
    uint8_t spielerRegister = 0;
    uint8_t startLed = 0;
    //uint8_t spielerPositionAlt = 0;
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
uint8_t pruefeSpielerPosition(uint8_t feld, uint8_t spielerNummer)
{
    uint8_t spielerRegister = 0;
    uint8_t startLed = 0;
    int8_t fehlerausgleich = 0;
    uint8_t transmitdata = 0;

   //Aktualisiert die Spielerposition im Array
   spielerPos[spielerNummer - 1] = feld;

   //Aktiviert die LED der neuen Position des Spielers
   spielerRegister = (feld * 4) / 8;
   startLed = (feld * 4) % 8;
   fehlerausgleich = spielerPosFehlerAusgleich(spielerNummer);
   return (spieler[spielerRegister][startLed + (spielerNummer - 1) + fehlerausgleich]);
}   
void setzeSpielerPositionAnimation(uint8_t feld, uint8_t spielerNummer, uint8_t onOff, uint8_t ausgabe)
{
    uint8_t spielerRegister = 0;
    uint8_t startLed = 0;
    //uint8_t spielerPositionAlt = 0;
    int8_t fehlerausgleich = 0;
    uint8_t transmitdata = 0;
    //Deaktiviert die LED der alten Position des Spielers
   /* spielerRegister = (spielerPos[spielerNummer - 1] * 4) / 8;
    startLed = (spielerPos[spielerNummer - 1] * 4) % 8;
    fehlerausgleich = spielerPosFehlerAusgleich(spielerNummer);
    spieler[spielerRegister][startLed + (spielerNummer - 1) + fehlerausgleich] = 0;
    */
   if (!ausgabe)
   {
       //Aktualisiert die Spielerposition im Array
       spielerPos[spielerNummer - 1] = feld;

       //Aktiviert die LED der neuen Position des Spielers
       spielerRegister = (feld * 4) / 8;
       startLed = (feld * 4) % 8;
       fehlerausgleich = spielerPosFehlerAusgleich(spielerNummer);
       spieler[spielerRegister][startLed + (spielerNummer - 1) + fehlerausgleich] = onOff;
   }
   else
   {
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
}

uint8_t animationAbbrechen(uint8_t status)
{
    //Flankenerkennung
    if (!status)
    {
        tasteAlt = tasteNeu;
        tasteNeu = 0;
        tasteNeu = (PINL << 8) | PINK;
        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        if (positiveFlanke)
        {
            return(1);
        }
        return(0);
    }
    else
    {
        return(1);
    }
}

void spielfeldBlinken(uint8_t spielerNr)
{
    uint8_t transmitdata = 0;
    //Sendet die Position des Spielers über SPI, 8 Bits pro Durchgang
    for(uint8_t h = 0; h < 6; h = h + 1)
    {
        for (uint8_t i = 0; i < 21; i = i + 1)
        {
            transmitdata = 0;
            for (uint8_t j = 0; j < 8; j = j + 1)
            {
                transmitdata = transmitdata << 1;
                transmitdata = (transmitdata | spieler[20 - i][7 - j]);
                transmitdata = transmitdata * ((h + 2) % 2);
            }
            //Überträgt die Daten über SPI
            Send2SPI(transmitdata);
            //Kurze Verzögerung für SPI-Übertragung
            _delay_us(500);
        }
        //Latch, um die Position auf der Anzeige zu fixieren
        PORTB = PORTB |  SPIELER_POSITION_LATCH_PIN;
        PORTB = PORTB & ~SPIELER_POSITION_LATCH_PIN;
        _delay_ms(250);
    }
    
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
#define PUNKT (1<<7)    //Doppelpunkt (nicht in diesem Code verwendet)

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
    (0),                                                                        //10 (unsichtbar, z.B. für ein Leerzeichen oder Null)
    (PUNKT),                                                                    //11
    (SEG_G),                                                                    //12 - (---- = Spieler ist aus versteigerung zurückgetreten)
    (0),                                                                        //13
    (0),                                                                        //14
    (0),                                                                        //15
    (0),                                                                        //16
    (0),                                                                        //17
    (0),                                                                        //18
    (0),                                                                        //19
    (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | PUNKT),                    //20 --> 0.
    (SEG_B | SEG_C | PUNKT),                                                    //21 --> 1.
    (SEG_A | SEG_B | SEG_G | SEG_E | SEG_D | PUNKT),                            //22 --> 2.
    (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G | PUNKT),                            //23 --> 3.
    (SEG_F | SEG_G | SEG_B | SEG_C | PUNKT),                                    //24 --> 4.
    (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G | PUNKT),                            //25 --> 5.
    (SEG_F | SEG_G | SEG_C | SEG_D | SEG_E | SEG_A | PUNKT),                    //26 --> 6.
    (SEG_A | SEG_B | SEG_C | PUNKT),                                            //27 --> 7.
    (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | PUNKT),            //28 --> 8.
    (SEG_A | SEG_B | SEG_C | SEG_F | SEG_G | SEG_D | PUNKT),                    //29 --> 9.
    
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
    //variabeln tausender, hunderter, zehner und einer auf 0 setzen
    uint8_t tausender       = 0;
    uint8_t hunderter       = 0;
    uint8_t zehner          = 0;
    uint8_t einer           = 0;
    //uint8_t transmitdata    = 0;

    //Berechnen der einzelnen Ziffern
    //tausender ziffer berechnen und in variabel tausender speichern
    tausender = (geld / 1000) % 10;    // Tausender
    //hunderter ziffer berechnen und in variabel hunderter speichern
    hunderter = (geld / 100) % 10;     // Hunderter
    //zehner ziffer berechnen und in variabel zehner speichern
    zehner    = (geld / 10) % 10;      // Zehner
    //einer ziffer berechnen und in variabel einer speichern
    einer     = geld % 10;             // Einer

    //Wenn das Siebensegment-Display eingeschaltet werden soll (siebensegmentOnOff = 1)
    if (siebensegmentOnOff == 1)//ON
    {
        //Die entsprechenden Ziffern für den Spieler im Array "siebensegment" setzen
        //bestimmt das darzustellende symbol für die Tausender ziffer und speichert sie im siebensegment array
        siebensegment[((spieler - 1) * 4)]      = ziffer[tausender]; // Tausender
        //bestimmt das darzustellende symbol für die Hunderter ziffer und speichert sie im siebensegment array
        siebensegment[((spieler - 1) * 4) + 1]  = ziffer[hunderter]; // Hunderter
        //bestimmt das darzustellende symbol für die Zehner ziffer und speichert sie im siebensegment array
        siebensegment[((spieler - 1) * 4) + 2]  = ziffer[zehner];    // Zehner
        //bestimmt das darzustellende symbol für die Einer ziffer und speichert sie im siebensegment array
        siebensegment[((spieler - 1) * 4) + 3]  = ziffer[einer];     // Einer
    }
    else if(!siebensegmentOnOff)
    {
        //Wenn das Siebensegment-Display ausgeschaltet werden soll (siebensegmentOnOff = 0),
        //alle Ziffern auf den Wert "10" setzen (unsichtbar, also keine Anzeige)
        
        //lädt ein unsichtbares symbol für die Tausender ziffer und speichert es im siebensegment array
        siebensegment[((spieler - 1) * 4)]      = ziffer[10];  //Unsichtbar
        //lädt ein unsichtbares symbol für die Hunderter ziffer und speichert es im siebensegment array
        siebensegment[((spieler - 1) * 4) + 1]  = ziffer[10];  //Unsichtbar
        //lädt ein unsichtbares symbol für die Zehner ziffer und speichert es im siebensegment array
        siebensegment[((spieler - 1) * 4) + 2]  = ziffer[10];  //Unsichtbar
        //lädt ein unsichtbares symbol für die Einer ziffer und speichert es im siebensegment array
        siebensegment[((spieler - 1) * 4) + 3]  = ziffer[10];  //Unsichtbar
    }
    else if(siebensegmentOnOff == 2)//Markierung um anzuzeigen wer aus der versteigerung zurückgetreten ist
    {
        //lädt ein unsichtbares symbol für die Tausender ziffer und speichert es im siebensegment array
        siebensegment[((spieler - 1) * 4)]      = ziffer[12];  //Unsichtbar
        //lädt ein unsichtbares symbol für die Hunderter ziffer und speichert es im siebensegment array
        siebensegment[((spieler - 1) * 4) + 1]  = ziffer[12];  //Unsichtbar
        //lädt ein unsichtbares symbol für die Zehner ziffer und speichert es im siebensegment array
        siebensegment[((spieler - 1) * 4) + 2]  = ziffer[12];  //Unsichtbar
        //lädt ein unsichtbares symbol für die Einer ziffer und speichert es im siebensegment array
        siebensegment[((spieler - 1) * 4) + 3]  = ziffer[12];  //Unsichtbar
    }
    else if (siebensegmentOnOff == 3)//Geld und SpielerAmZug ausgeben
    {
        //Die entsprechenden Ziffern für den Spieler im Array "siebensegment" setzen
        //bestimmt das darzustellende symbol für die Tausender ziffer und speichert sie im siebensegment array
        siebensegment[((spieler - 1) * 4)]      = ziffer[tausender + 20]; //Tausender
        //bestimmt das darzustellende symbol für die Hunderter ziffer und speichert sie im siebensegment array
        siebensegment[((spieler - 1) * 4) + 1]  = ziffer[hunderter + 20]; //Hunderter
        //bestimmt das darzustellende symbol für die Zehner ziffer und speichert sie im siebensegment array
        siebensegment[((spieler - 1) * 4) + 2]  = ziffer[zehner + 20];    //Zehner
        //bestimmt das darzustellende symbol für die Einer ziffer und speichert sie im siebensegment array
        siebensegment[((spieler - 1) * 4) + 3]  = ziffer[einer + 20];     //Einer
    }

    //Ausgabe der Daten an das Siebensegment-Display
    for (uint8_t i = 0; i < ANZAHL_KONTO_SIEBENSEGMENTE; i = i + 1)
    {
        //Datenausgabe an USART schnittstelle
        USART_Transmit(USART_GELD_SCHIEBEREGISTER, siebensegment[16 - i]);
        _delay_us(500);  //500?s warten um korrekte übermittlung zu gewährleisten
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
void updateKontostand(uint8_t anzahlSpieler, Spieler spielerInfo[5], uint8_t spielerAmZug)
{
    //Iteriere über alle aktiven Spieler (bis anzahlSpieler)
    for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
    {
        //Setze den Geldbetrag für den Spieler i auf den Wert von spielerInfo[i].geld
        //Der dritte Parameter ist 1, was bedeutet, dass das Siebensegment eingeschaltet ist
        //Wenn der Spieler nicht Pleite ist
        if (i == spielerAmZug)
        {
            //Kontostand auf und Spieler am Zug am Siebensegment anzeigen
            setGeld(spielerInfo[i].geld, i, 3);
        }
        else if (!spielerInfo[i].pleite)
        {
            //Kontostand auf Siebensegment anzeigen
            setGeld(spielerInfo[i].geld, i, 1);
        }
        else
        {
            //Siebensegment ausschalten
            setGeld(spielerInfo[i].geld, i, 0);
        }
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
* zufallsgenerator
*
* Generiert eine Zufallszahl im Bereich von 1 bis 6, die einen Würfelerwurf 
  simuliert.
*
* Rückgabewert: Zufallszahl (1 bis 6)
*
\******************************************************************************/
uint8_t zufallsGenerator(void)
{
    //Variabel zufallszahl initialisieren
    uint8_t zufallszahl = 0;
    //zufällige Zahl zwischen 1 und 6 generieren
    zufallszahl = (rand() % 6) + 1;
    //zufallszahl zurückgeben
    return zufallszahl;
}

/******************************************************************************\
* wuerfel
*
* Simuliert eine Würfeln-Animation, indem es mehrfach Zufallszahlen generiert,
* diese anzeigt und am Ende die finalen Würfelergebnisse auf dem Display darstellt.
*
* Parameter: Keine
*
* Rückgabewert: Keine Rückgabe (void)
*
\******************************************************************************/
void wuerfelAlt(void)
{
    //char buffer[16];
    uint8_t zufallszahl1 = 0;
    uint8_t zufallszahl2 = 0;
    
    //Simuliert mehrere Würfeln-Animationen
    for (uint8_t i = 0; i < 50; i = i + 1)
    {
        zufallszahl1 = zufallsGenerator(); //Erste Zufallszahl
        zufallszahl2 = zufallsGenerator(); //Zweite Zufallszahl
        
        //Zeigt die erste Zufallszahl auf dem Display an
        /*sprintf(buffer, "%u", zufallszahl1);
        writeText(0, 9, buffer);*/
        
        //Zeigt die zweite Zufallszahl auf dem Display an
        /*sprintf(buffer, "%u", zufallszahl2);
        writeText(1, 9, buffer);*/
        
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
    zufallszahl1 = zufallsGenerator();
    zufallszahl2 = zufallsGenerator();
    
    wuerfelTransmit(zufallszahl1, zufallszahl2);
    
    //Speichert die finalen Zufallszahlen im Array
    wuerfelArray[0] = zufallszahl1;
    wuerfelArray[1] = zufallszahl2;
}

/******************************************************************************\
* wuerfel
*
* Diese Funktion simuliert das Würfeln mit zwei Würfeln und zeigt die Ergebnisse
* auf Siebensegment-Anzeigen an.
*
* Parameter:
* wuerfelNummer = Nummer des Würfels (1 = A, 2 = B)
* flagWuerfel1  = Statusflag, ob Würfel A bereits gewürfelt wurde
* flagWuerfel2  = Statusflag, ob Würfel B bereits gewürfelt wurde
*
* Rückgabewert: keiner
*
\******************************************************************************/
void wuerfel(uint8_t wuerfelNummer, uint8_t flagWuerfel1, uint8_t flagWuerfel2)
{
    //Variabeln für zufallszahl 1 und zufallszahl 2 initialisieren
    uint8_t zufallszahl1 = 10;
    uint8_t zufallszahl2 = 10;
    //wenn mit würfel A gewürfelt werden soll und
    //bereits mit würfel B gewürfelt wurde
    if((wuerfelNummer == 1) && flagWuerfel2)
    {
        //Simuliert mehrere Würfeln-Animationen
        //erhöhe i um 1 solange i < 50 ist. Starte mit i = 1
        for (uint8_t i = 0; i < 50; i = i + 1)
        {
            //zufällige Zahl generieren und als zufallszahl1 speichern
            zufallszahl1 = zufallsGenerator(); //Erste Zufallszahl
            //zufallszahl1 und 2 auf Siebensegment des Würfels A und B ausgeben
            wuerfelTransmit(zufallszahl1,wuerfelArray[1]);
            //Verzögert den nächsten Durchgang
            //warte zwischen 15ms und 115ms
            _delay_ms(15 + i * 2);
        }
        //letzte generierte zufallszahl speichern
        wuerfelArray[0] = zufallszahl1;
    }
    //wenn mit würfel A gewürfelt werden soll und
    //noch nicht mit würfel B gewürfelt wurde
    else if((wuerfelNummer == 1) && !flagWuerfel2)
    {
        //Simuliert mehrere Würfeln-Animationen
        //erhöhe i um 1 solange i < 50 ist. Starte mit i = 1
        for (uint8_t i = 0; i < 50; i = i + 1)
        {
            //zufällige Zahl generieren und als zufallszahl1 speichern
            zufallszahl1 = zufallsGenerator(); //Erste Zufallszahl
            //zufallszahl1 auf Siebensegment des Würfels A ausgeben
            //auf Siebensegment des Würfels B nichts anzeigen
            wuerfelTransmit(zufallszahl1,SIEBENSEGMENT_OFF);
            //Verzögert den nächsten Durchgang
            _delay_ms(15 + i * 2);
        }
        wuerfelArray[0] = zufallszahl1;
    }
    //wenn mit würfel B gewürfelt werden soll und
    //bereits mit würfel A gewürfelt wurde
    else if((wuerfelNummer == 2) && flagWuerfel1)
    {
        //Simuliert mehrere Würfeln-Animationen
        //erhöhe i um 1 solange i < 50 ist. Starte mit i = 1
        for (uint8_t i = 0; i < 50; i = i + 1)
        {
            //zufällige Zahl generieren und als zufallszahl2 speichern
            zufallszahl2 = zufallsGenerator(); //Erste Zufallszahl
            //zufallszahl1 und 2 auf Siebensegment des Würfels A und B ausgeben
            wuerfelTransmit(wuerfelArray[0],zufallszahl2);
            //Verzögert den nächsten Durchgang
            _delay_ms(15 + i * 3);
        }
        wuerfelArray[1] = zufallszahl2;
    }
    //wenn mit würfel B gewürfelt werden soll und
    //noch nicht mit würfel A gewürfelt wurde
    else if((wuerfelNummer == 2) && !flagWuerfel1)
    {
        //Simuliert mehrere Würfeln-Animationen
        //erhöhe i um 1 solange i < 50 ist. Starte mit i = 1
        for (uint8_t i = 0; i < 50; i = i + 1)
        {
            //zufällige Zahl generieren und als zufallszahl2 speichern
            zufallszahl2 = zufallsGenerator(); //Erste Zufallszahl
            //zufallszahl2 auf Siebensegment des Würfels B ausgeben
            //auf Siebensegment des Würfels A nichts anzeigen
            wuerfelTransmit(SIEBENSEGMENT_OFF,zufallszahl2);
            //Verzögert den nächsten Durchgang
            _delay_ms(15 + i * 2);
        }
        wuerfelArray[1] = zufallszahl2;
    }
}


/******************************************************************************\
* wuerfel
*
* Diese Funktion simuliert das Würfeln mit zwei Würfeln und zeigt die Ergebnisse
* auf Siebensegment-Anzeigen an.
*
* Parameter:
* wuerfelNummer = Nummer des Würfels (1 = A, 2 = B)
* flagWuerfel1  = Statusflag, ob Würfel A bereits gewürfelt wurde
* flagWuerfel2  = Statusflag, ob Würfel B bereits gewürfelt wurde
*
* Rückgabewert: keiner
*
\******************************************************************************/
void wuerfelAB(void)
{
    //Variabeln für zufallszahl 1 und zufallszahl 2 initialisieren
    uint8_t zufallszahl1 = 10;
    uint8_t zufallszahl2 = 10;
    for (uint8_t i = 0; i < 50; i = i + 1)
    {
        //zufällige Zahl generieren und als zufallszahl2 speichern
        zufallszahl1 = zufallsGenerator(); //Zufallszahl für Würfel A
        zufallszahl2 = zufallsGenerator(); //Zufallszahl für Würfel B
        //Zufallszahlen auf Siebensegmenten ausgeben
        //auf Siebensegment des Würfels A nichts anzeigen
        wuerfelTransmit(zufallszahl1, zufallszahl2);
        //Verzögert den nächsten Durchgang
        _delay_ms(15 + i * 2);
    }
    wuerfelArray[0] = zufallszahl1;
    wuerfelArray[1] = zufallszahl2;
}


/******************************************************************************\
* wuerfelTransmit
*
* Diese Funktion überträgt die Zahlen der beiden Würfel (zahl1 und zahl2)
* an ein Display über die USART-Schnittstelle und aktualisiert die Anzeige.
*
* Parameter:
* zahl1 = Zahl für Würfel A (0-9)
* zahl2 = Zahl für Würfel B (0-9)
*
* Rückgabewert: keiner
*
\******************************************************************************/

void wuerfelTransmit(uint8_t zahl1, uint8_t zahl2)
{
    //Ausgabe der Zahl für Würfel B über USART Schnitstelle
    USART_Transmit(3, ziffer[zahl2]);
    //500?s warten um korrekte übermittlung zu garantieren
    _delay_us(500);
    
    //Sendet die finale erste zahl1 an das Display
    USART_Transmit(3, ziffer[zahl1]);
    _delay_us(500);
    
    //Aktualisiert die Anzeige
    PORTJ = PORTJ | 0x08;
    PORTJ = PORTJ & ~0x08;
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

/******************************************************************************\
* abInsGefaengnis
*
* Platziert den Spieler im Gefängnis und lässt das Blaulicht aufläuchten
* - Setzt die Spielerposition auf das Gefängnis feld
* 
* 
*
* Parameter:
* spielerNr = Die Nummer des Spielers, der ins gefängnis soll (1-4)
*
*
* Rückgabewert: kein Rückgabewert (void)
*
\******************************************************************************/
void abInsGefaengnis(uint8_t spielerNr)
{
    spielerInfo[spielerNr].gefaengnis = 1;
    //spielerImGefaengnis[spielerNr] = 1;
    blaulicht(100,20);
    spielerInfo[spielerNr].position = 10;
    setzeSpielerPosition(10,spielerNr);
}

/******************************************************************************\
* blaulicht
*
* Diese Funktion simuliert ein Blaulicht durch das abwechselnde Ein- und Ausschalten
* von zwei LEDs für eine bestimmte Anzahl an Wiederholungen.
*
* Parameter:
* delay = Wartezeit in Millisekunden zwischen den Blinks
* anzahlWiederholungen = Anzahl der Blinkzyklen
*
* Rückgabewert: keiner
*
\******************************************************************************/
void blaulicht(uint8_t delay, uint8_t anzahlWiederholungen)
{
    for (uint8_t i = 0; i < anzahlWiederholungen; i = i + 1)
    {
        PORTC |=  BLAULICHT_LED1_ON;
        PORTC &= BLAULICHT_LED2_OFF;
        _delay_ms(delay);
        PORTC |=  BLAULICHT_LED2_ON;
        PORTC &= BLAULICHT_LED1_OFF;
        _delay_ms(delay);
    }
    PORTC &= BLAULICHT_OFF;
}

/******************************************************************************\
* PortInitialisierung
*
* Diese Funktion initialisiert die Ports des Mikrocontrollers.
* Dabei werden die Richtungen der Ports (Eingang oder Ausgang) sowie
* die Anfangszustände der Ausgangsports festgelegt.
*
* Parameter: keine
* Rückgabewert: keiner
*
\******************************************************************************/
void PortInitialisierung(void)
{
    DDRA = 0xFF;		// Port A auf Ausgang initialisieren (alle Pins)
    DDRB = 0xFF;		// Port B auf Ausgang initialisieren (alle Pins)
    PORTB = 0b00100000; //Setzt Clear der Spieler Schieberegister auf 1
    DDRC = 0xFF;		// Port C auf Ausgang initialisieren (alle Pins)
    PORTC = 0x3F;
    DDRD = 0xFF;		// Port D auf Ausgang initialisieren (alle Pins)
    PORTD = 0x00;
    DDRE = 0xFF;		// Port E auf Ausgang initialisieren (alle Pins)
    PORTE = 0b00010000; //Setzt Clear der Häuser schieberegister auf 1
    DDRF = 0xFE;		// Port F auf Ausgang initialisieren (alle Pins)
    DDRH = 0xFF;		// Port H auf Ausgang initialisieren (alle Pins)
    PORTH = 0x10;       //Setzt Clear der Siebensegmente schieberegister auf 1
    DDRJ = 0xFF;		// Port J auf Ausgang initialisieren (alle Pins)
    PORTJ = 0x10;       //Setzt Clear der Würfel schieberegister auf 1
    DDRK = 0x00;		// Port K auf Eingang initialisieren (alle Pins)
    DDRL = 0x00;		// Port L auf Eingang initialisieren (alle Pins)
}


/******************************************************************************\
* startGeldAnimation
*
* Diese Funktion verteilt das Startkapital in Form von Banknoten an alle Spieler.
* Die Animation simuliert das schrittweise Verteilen der Banknoten und aktualisiert
* den Kontostand der Spieler nach jeder Banknote.
*
* Parameter:
*   - anzahlSpieler: Anzahl der Spieler im Spiel (1 bis max. Spieleranzahl)
*
* Rückgabewert: keiner
*
\******************************************************************************/
void startGeldAnimation(uint8_t anzahlSpieler)
{
    //Die Zahlen im Array sind die banknoten die man als starkapital erhält
    //die Banknoten 500,500,100,100,100,100,50,20,10,10,5,1,1,1,1,1 werden im startGeld array hinterlegt
    uint16_t startgeld[16] = {500,500,100,100,100,100,50,20,10,10,5,1,1,1,1,1};
    //die 16 bezieht sich auf die anzahl banknoten die man bekommt
    //erhöhe i um 1 solange i < 16 ist. Starte mit i = 0
    for (uint8_t i = 0; i < 16; i = i + 1)
    {
        //verteilt die Banknoten an die Spieler
        //erhöhe j um 1 solange j <= anzahlSpieler ist. Starte mit j = 1
        for (uint8_t j = 1; j <= anzahlSpieler; j = j + 1)
        {
            //erhöhe den Kontostand von spieler j um den betrag der an position i im startGeld array hinterlegt ist
            spielerInfo[j].geld = spielerInfo[j].geld + startgeld[i];
            //warte 75ms 
            _delay_ms(75); //Delay dient zu animationszwecken
            //aktualisiere den Kontostand
            updateKontostand(anzahlSpieler,spielerInfo,0); 
        }
        //warte 50 ms
        _delay_ms(50); //Delay dient zu animationszwecken
    }
}


/******************************************************************************\
* initialisiereSpielfeld
*
* Initialisiert die Spielfelder des Monopoly-ähnlichen Spiels.
*
* Parameter:
*   - spielfeld[]: Array von Feldern, das die Spielfeld-Datenstruktur speichert
*
* Rückgabewert: keiner
\******************************************************************************/
void initialisiereSpielfeld(Feld spielfeld[])
{
    //Eigenschaften des Feldes: Los
    strcpy(spielfeld[0].name, "Los");
    spielfeld[0].typ = FREIPARKEN;



    //Eigenschaften des Feldes: Eingangshalle
    strcpy(spielfeld[1].name, "Eingangshalle");
    spielfeld[1].typ = STRASSE;
    spielfeld[1].preis = 60;
    spielfeld[1].mieten[0] = 2;     //Feld einzeln
    spielfeld[1].mieten[1] = 10;    //Feld mit 1 Haus
    spielfeld[1].mieten[2] = 30;    //Feld mit 2 Häuser
    spielfeld[1].mieten[3] = 90;    //Feld mit 3 Häuser
    spielfeld[1].mieten[4] = 160;   //Feld mit 4 Häuser
    spielfeld[1].mieten[5] = 250;   //Feld mit 1 Hotel
    spielfeld[1].mieten[6] = 4;     //Feld mit Farbgrupp
    spielfeld[1].besitzer = 0;
    spielfeld[1].farbGruppe = BRAUN;
    spielfeld[1].farbgruppenFelder[0] = 1;
    spielfeld[1].farbgruppenFelder[1] = 3;
    spielfeld[1].farbgruppenFelder[2] = 0;
    spielfeld[1].hausnummer = 0;
    spielfeld[1].anzahlHaeuser = 0;
    spielfeld[1].kostenHaus = 50;
    spielfeld[1].rgbNummer = 0;
    spielfeld[1].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Kanzlei
    strcpy(spielfeld[2].name, "Kanzlei1");
    spielfeld[2].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Velokeller
    strcpy(spielfeld[3].name, "Velokeller");
    spielfeld[3].typ = STRASSE;
    spielfeld[3].preis = 60;
    spielfeld[3].preis = 60;
    spielfeld[3].mieten[0] = 4;     //Feld einzeln
    spielfeld[3].mieten[1] = 20;    //Feld mit 1 Haus
    spielfeld[3].mieten[2] = 60;    //Feld mit 2 Häuser
    spielfeld[3].mieten[3] = 180;   //Feld mit 3 Häuser
    spielfeld[3].mieten[4] = 320;   //Feld mit 4 Häuser
    spielfeld[3].mieten[5] = 450;   //Feld mit 1 Hotel
    spielfeld[3].mieten[6] = 4;     //Feld mit Farbgruppe
    spielfeld[3].besitzer = 0;
    spielfeld[3].farbGruppe = BRAUN;
    spielfeld[3].farbgruppenFelder[0] = 1;
    spielfeld[3].farbgruppenFelder[1] = 3;
    spielfeld[3].farbgruppenFelder[2] = 0;
    spielfeld[3].hausnummer = 1;
    spielfeld[3].anzahlHaeuser = 0;
    spielfeld[3].kostenHaus = 50;
    spielfeld[3].rgbNummer = 1;
    spielfeld[3].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    
    //Eigenschaften des Feldes: Laptopgebühr
    strcpy(spielfeld[4].name, "Laptopgeb"UE"hr");
    spielfeld[4].typ = STEUERFELD;
    spielfeld[4].preis = 200;
    
    //Eigenschaften des Feldes: Zeughausstrasse
    strcpy(spielfeld[5].name, "Zeughausstrasse");
    spielfeld[5].typ = HALTESTELLE;
    spielfeld[5].preis = 200;
    spielfeld[5].mieten[0] = 25;    //wenn man 1 Bahn besitzt
    spielfeld[5].mieten[1] = 50;    //wenn man 2 Bahnen besitzt
    spielfeld[5].mieten[2] = 100;   //wenn man 3 Bahnen besitzt
    spielfeld[5].mieten[3] = 200;   //wenn man 4 Bahnen besitzt
    spielfeld[5].besitzer = 0;
    spielfeld[5].farbGruppe = FARBLOS;
    spielfeld[5].farbgruppenFelder[0] = 5;
    spielfeld[5].farbgruppenFelder[1] = 15;
    spielfeld[5].farbgruppenFelder[2] = 25;
    spielfeld[5].farbgruppenFelder[3] = 35;
    spielfeld[5].rgbNummer = 2;
    spielfeld[5].feldBelastet = 0;  //wenn das Feld belastet ist = 1

    
    //Eigenschaften des Feldes: Raucherzelt
    strcpy(spielfeld[6].name, "Raucherzelt");
    spielfeld[6].typ = STRASSE;
    spielfeld[6].preis = 100;
    spielfeld[6].mieten[0] = 6;     //Feld einzeln
    spielfeld[6].mieten[1] = 30;    //Feld mit 1 Haus
    spielfeld[6].mieten[2] = 90;    //Feld mit 2 Häuser
    spielfeld[6].mieten[3] = 270;   //Feld mit 3 Häuser
    spielfeld[6].mieten[4] = 400;   //Feld mit 4 Häuser
    spielfeld[6].mieten[5] = 550;   //Feld mit 5 Häuser
    spielfeld[6].mieten[6] = 12;    //Feld mit Farbgruppe
    spielfeld[6].besitzer = 0;
    spielfeld[6].farbGruppe = HELLBLAU;
    spielfeld[6].farbgruppenFelder[0] = 6;
    spielfeld[6].farbgruppenFelder[1] = 8;
    spielfeld[6].farbgruppenFelder[2] = 9;
    spielfeld[6].hausnummer = 2;
    spielfeld[6].anzahlHaeuser = 0;
    spielfeld[6].kostenHaus = 50;
    spielfeld[6].rgbNummer = 3;
    spielfeld[6].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    
    //Eigenschaften des Feldes: Chance
    strcpy(spielfeld[7].name, "Chance");
    spielfeld[7].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Pausenraum
    strcpy(spielfeld[8].name, "Pausenraum");
    spielfeld[8].typ = STRASSE;
    spielfeld[8].preis = 100;
    spielfeld[8].mieten[0] = 6;     //Feld einzeln
    spielfeld[8].mieten[1] = 30;    //Feld mit 1 Haus
    spielfeld[8].mieten[2] = 90;    //Feld mit 2 Häuser
    spielfeld[8].mieten[3] = 270;   //Feld mit 3 Häuser
    spielfeld[8].mieten[4] = 400;   //Feld mit 4 Häuser
    spielfeld[8].mieten[5] = 550;   //Feld mit 5 Häuser
    spielfeld[8].mieten[6] = 12;    //Feld mit Farbgruppe
    spielfeld[8].besitzer = 0;
    spielfeld[8].farbGruppe = HELLBLAU;
    spielfeld[8].farbgruppenFelder[0] = 6;
    spielfeld[8].farbgruppenFelder[1] = 8;
    spielfeld[8].farbgruppenFelder[2] = 9;
    spielfeld[8].hausnummer = 3;
    spielfeld[8].anzahlHaeuser = 0;
    spielfeld[8].kostenHaus = 50;
    spielfeld[8].rgbNummer = 4;
    spielfeld[8].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    

    
    //Eigenschaften des Feldes: Garderobe
    strcpy(spielfeld[9].name, "Garderobe");
    spielfeld[9].typ = STRASSE;
    spielfeld[9].preis = 120;
    spielfeld[9].mieten[0] = 8;     //Feld einzeln
    spielfeld[9].mieten[1] = 40;    //Feld mit 1 Haus
    spielfeld[9].mieten[2] = 100;   //Feld mit 2 Häuser
    spielfeld[9].mieten[3] = 300;   //Feld mit 3 Häuser
    spielfeld[9].mieten[4] = 450;   //Feld mit 4 Häuser
    spielfeld[9].mieten[5] = 600;   //Feld mit 5 Häuser
    spielfeld[9].mieten[6] = 16;    //Feld mit Farbgruppe
    spielfeld[9].besitzer = 0;
    spielfeld[9].farbGruppe = HELLBLAU;
    spielfeld[9].farbgruppenFelder[0] = 6;
    spielfeld[9].farbgruppenFelder[1] = 8;
    spielfeld[9].farbgruppenFelder[2] = 9;
    spielfeld[9].hausnummer = 4;
    spielfeld[9].anzahlHaeuser = 0;
    spielfeld[9].kostenHaus = 50;
    spielfeld[9].rgbNummer = 5;
    spielfeld[9].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Gefängnis
    strcpy(spielfeld[10].name, "Gef"AE"ngnis");
    spielfeld[10].typ = GEFAENGNIS;
    
    
    
    
    //Eigenschaften des Feldes: Herren Wc
    strcpy(spielfeld[11].name, "Herren WC");
    spielfeld[11].typ = STRASSE;
    spielfeld[11].preis = 140;
    spielfeld[11].mieten[0] = 10;   //Feld einzeln
    spielfeld[11].mieten[1] = 50;   //Feld mit 1 Haus
    spielfeld[11].mieten[2] = 150;  //Feld mit 2 Häuser
    spielfeld[11].mieten[3] = 450;  //Feld mit 3 Häuser
    spielfeld[11].mieten[4] = 625;  //Feld mit 4 Häuser
    spielfeld[11].mieten[5] = 750;  //Feld mit 5 Häuser
    spielfeld[11].mieten[6] = 20;   //Feld mit Farbgruppe
    spielfeld[11].besitzer = 0;
    spielfeld[11].farbGruppe = ROSA;
    spielfeld[11].farbgruppenFelder[0] = 11;
    spielfeld[11].farbgruppenFelder[1] = 13;
    spielfeld[11].farbgruppenFelder[2] = 14;
    spielfeld[11].hausnummer = 5;
    spielfeld[11].anzahlHaeuser = 0;
    spielfeld[11].kostenHaus = 100;
    spielfeld[11].rgbNummer = 6;
    spielfeld[11].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Informatikdienst
    strcpy(spielfeld[12].name, "Informatikdienst");
    spielfeld[12].typ = WERK;
    spielfeld[12].preis = 150;
    spielfeld[12].besitzer = 0;
    spielfeld[12].farbGruppe = FARBLOS;
    spielfeld[12].rgbNummer = 7;
    spielfeld[12].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Frauen WC
    strcpy(spielfeld[13].name, "Frauen WC");
    spielfeld[13].typ = STRASSE;
    spielfeld[13].preis = 140;
    spielfeld[13].mieten[0] = 10;   //Feld einzeln
    spielfeld[13].mieten[1] = 50;   //Feld mit 1 Haus
    spielfeld[13].mieten[2] = 150;  //Feld mit 2 Häuser
    spielfeld[13].mieten[3] = 450;  //Feld mit 3 Häuser
    spielfeld[13].mieten[4] = 625;  //Feld mit 4 Häuser
    spielfeld[13].mieten[5] = 750;  //Feld mit 5 Häuser
    spielfeld[13].mieten[6] = 20;   //Feld mit Farbgruppe
    spielfeld[13].besitzer = 0;
    spielfeld[13].farbGruppe = ROSA;
    spielfeld[13].farbgruppenFelder[0] = 11;
    spielfeld[13].farbgruppenFelder[1] = 13;
    spielfeld[13].farbgruppenFelder[2] = 14;
    spielfeld[13].hausnummer = 6;
    spielfeld[13].anzahlHaeuser = 0;
    spielfeld[13].kostenHaus = 100;
    spielfeld[13].rgbNummer = 8;
    spielfeld[13].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Lehrer WC
    strcpy(spielfeld[14].name, "Lehrer WC");
    spielfeld[14].typ = STRASSE;
    spielfeld[14].preis = 160;
    spielfeld[14].mieten[0] = 12;   //Feld einzeln
    spielfeld[14].mieten[1] = 60;   //Feld mit 1 Haus
    spielfeld[14].mieten[2] = 180;  //Feld mit 2 Häuser
    spielfeld[14].mieten[3] = 500;  //Feld mit 3 Häuser
    spielfeld[14].mieten[4] = 700;  //Feld mit 4 Häuser
    spielfeld[14].mieten[5] = 900;  //Feld mit 5 Häuser
    spielfeld[14].mieten[6] = 24;   //Feld mit Farbgruppe
    spielfeld[14].besitzer = 0;
    spielfeld[14].farbGruppe = ROSA;
    spielfeld[14].farbgruppenFelder[0] = 11;
    spielfeld[14].farbgruppenFelder[1] = 13;
    spielfeld[14].farbgruppenFelder[2] = 14;
    spielfeld[14].hausnummer = 7;
    spielfeld[14].anzahlHaeuser = 0;
    spielfeld[14].kostenHaus = 100;
    spielfeld[14].rgbNummer = 9;
    spielfeld[14].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Fotozentrum
    strcpy(spielfeld[15].name, "Fotozentrum");
    spielfeld[15].typ = HALTESTELLE;
    spielfeld[15].preis = 200;
    spielfeld[15].mieten[0] = 25;   //wenn man 1 Bahn besitzt
    spielfeld[15].mieten[1] = 50;   //wenn man 2 Bahnen besitzt
    spielfeld[15].mieten[2] = 100;  //wenn man 3 Bahnen besitzt
    spielfeld[15].mieten[3] = 200;  //wenn man 4 Bahnen besitzt
    spielfeld[15].besitzer = 0;
    spielfeld[15].farbGruppe = FARBLOS;
    spielfeld[15].farbgruppenFelder[0] = 5;
    spielfeld[15].farbgruppenFelder[1] = 15;
    spielfeld[15].farbgruppenFelder[2] = 25;
    spielfeld[15].farbgruppenFelder[3] = 35;
    spielfeld[15].rgbNummer = 10;
    spielfeld[15].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: BFS Polimechaniker
    strcpy(spielfeld[16].name, "BFS PM");
    spielfeld[16].typ = STRASSE;
    spielfeld[16].preis = 180;
    spielfeld[16].mieten[0] = 14;   //Feld einzeln
    spielfeld[16].mieten[1] = 70;   //Feld mit 1 Haus
    spielfeld[16].mieten[2] = 200;  //Feld mit 2 Häuser
    spielfeld[16].mieten[3] = 550;  //Feld mit 3 Häuser
    spielfeld[16].mieten[4] = 750;  //Feld mit 4 Häuser
    spielfeld[16].mieten[5] = 950;  //Feld mit 5 Häuser
    spielfeld[16].mieten[6] = 28;   //Feld mit Farbgruppe
    spielfeld[16].besitzer = 0;
    spielfeld[16].farbGruppe = ORANGE;
    spielfeld[16].farbgruppenFelder[0] = 16;
    spielfeld[16].farbgruppenFelder[1] = 18;
    spielfeld[16].farbgruppenFelder[2] = 19;
    spielfeld[16].hausnummer = 8;
    spielfeld[16].anzahlHaeuser = 0;
    spielfeld[16].kostenHaus = 100;
    spielfeld[16].rgbNummer = 11;
    spielfeld[16].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Kanzlei
    strcpy(spielfeld[17].name, "Kanzlei");
    spielfeld[17].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: BFS Automatiker
    strcpy(spielfeld[18].name, "BFS AU");
    spielfeld[18].typ = STRASSE;
    spielfeld[18].preis = 180;
    spielfeld[18].mieten[0] = 14;   //Feld einzeln
    spielfeld[18].mieten[1] = 70;   //Feld mit 1 Haus
    spielfeld[18].mieten[2] = 200;  //Feld mit 2 Häuser
    spielfeld[18].mieten[3] = 550;  //Feld mit 3 Häuser
    spielfeld[18].mieten[4] = 750;  //Feld mit 4 Häuser
    spielfeld[18].mieten[5] = 950;  //Feld mit 5 Häuser
    spielfeld[18].mieten[6] = 28;   //Feld mit Farbgruppe
    spielfeld[18].besitzer = 0;
    spielfeld[18].farbGruppe = ORANGE;
    spielfeld[18].farbgruppenFelder[0] = 16;
    spielfeld[18].farbgruppenFelder[1] = 18;
    spielfeld[18].farbgruppenFelder[2] = 19;
    spielfeld[18].hausnummer = 9;
    spielfeld[18].anzahlHaeuser = 0;
    spielfeld[18].kostenHaus = 100;
    spielfeld[18].rgbNummer = 12;
    spielfeld[18].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: BFS Elektroniker
    strcpy(spielfeld[19].name, "BFS EK");
    spielfeld[19].typ = STRASSE;
    spielfeld[19].preis = 200;
    spielfeld[19].mieten[0] = 16;   //Feld einzeln
    spielfeld[19].mieten[1] = 80;   //Feld mit 1 Haus
    spielfeld[19].mieten[2] = 220;  //Feld mit 2 Häuser
    spielfeld[19].mieten[3] = 600;  //Feld mit 3 Häuser
    spielfeld[19].mieten[4] = 800;  //Feld mit 4 Häuser
    spielfeld[19].mieten[5] = 1000; //Feld mit 5 Häuser
    spielfeld[19].mieten[6] = 32;   //Feld mit Farbgruppe
    spielfeld[19].besitzer = 0;
    spielfeld[19].farbGruppe = ORANGE;
    spielfeld[19].farbgruppenFelder[0] = 16;
    spielfeld[19].farbgruppenFelder[1] = 18;
    spielfeld[19].farbgruppenFelder[2] = 19;
    spielfeld[19].hausnummer = 10;
    spielfeld[19].anzahlHaeuser = 0;
    spielfeld[19].kostenHaus = 100;
    spielfeld[19].rgbNummer = 13;
    spielfeld[19].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Freiparken
    strcpy(spielfeld[20].name, "Freiparken");
    spielfeld[20].typ = FREIPARKEN;
    
    //Eigenschaften des Feldes: Grundausbildung Automatiker
    strcpy(spielfeld[21].name, "GA AU");
    spielfeld[21].typ = STRASSE;
    spielfeld[21].preis = 220;
    spielfeld[21].mieten[0] = 18;   //Feld einzeln
    spielfeld[21].mieten[1] = 90;   //Feld mit 1 Haus
    spielfeld[21].mieten[2] = 250;  //Feld mit 2 Häuser
    spielfeld[21].mieten[3] = 700;  //Feld mit 3 Häuser
    spielfeld[21].mieten[4] = 875;  //Feld mit 4 Häuser
    spielfeld[21].mieten[5] = 1050; //Feld mit 5 Häuser
    spielfeld[21].mieten[6] = 36;   //Feld mit Farbgruppe
    spielfeld[21].besitzer = 0;
    spielfeld[21].farbGruppe = ROT;
    spielfeld[21].farbgruppenFelder[0] = 21;
    spielfeld[21].farbgruppenFelder[1] = 23;
    spielfeld[21].farbgruppenFelder[2] = 24;
    spielfeld[21].hausnummer = 11;
    spielfeld[21].anzahlHaeuser = 0;
    spielfeld[21].kostenHaus = 150;
    spielfeld[21].rgbNummer = 14;
    spielfeld[21].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Chance
    strcpy(spielfeld[22].name, "Chance");
    spielfeld[22].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Produkton Automatiker
    strcpy(spielfeld[23].name, "Produktion AU");
    spielfeld[23].typ = STRASSE;
    spielfeld[23].preis = 220;
    spielfeld[23].mieten[0] = 18;   //Feld einzeln
    spielfeld[23].mieten[1] = 90;   //Feld mit 1 Haus
    spielfeld[23].mieten[2] = 250;  //Feld mit 2 Häuser
    spielfeld[23].mieten[3] = 700;  //Feld mit 3 Häuser
    spielfeld[23].mieten[4] = 875;  //Feld mit 4 Häuser
    spielfeld[23].mieten[5] = 1050; //Feld mit 5 Häuser
    spielfeld[23].mieten[6] = 36;   //Feld mit Farbgruppe
    spielfeld[23].besitzer = 0;
    spielfeld[23].farbGruppe = ROT;
    spielfeld[23].farbgruppenFelder[0] = 21;
    spielfeld[23].farbgruppenFelder[1] = 23;
    spielfeld[23].farbgruppenFelder[2] = 24;
    spielfeld[23].hausnummer = 12;
    spielfeld[23].anzahlHaeuser = 0;
    spielfeld[23].kostenHaus = 150;
    spielfeld[23].rgbNummer = 15;
    spielfeld[23].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Mechatronik Labor
    strcpy(spielfeld[24].name, "Mechatr. Labor");
    spielfeld[24].typ = STRASSE;
    spielfeld[24].preis = 240;
    spielfeld[24].mieten[0] = 20;   //Feld einzeln
    spielfeld[24].mieten[1] = 100;  //Feld mit 1 Haus
    spielfeld[24].mieten[2] = 300;  //Feld mit 2 Häuser
    spielfeld[24].mieten[3] = 750;  //Feld mit 3 Häuser
    spielfeld[24].mieten[4] = 925;  //Feld mit 4 Häuser
    spielfeld[24].mieten[5] = 1100; //Feld mit 5 Häuser
    spielfeld[24].mieten[6] = 40;   //Feld mit Farbgruppe
    spielfeld[24].besitzer = 0;
    spielfeld[24].farbGruppe = ROT;
    spielfeld[24].farbgruppenFelder[0] = 21;
    spielfeld[24].farbgruppenFelder[1] = 23;
    spielfeld[24].farbgruppenFelder[2] = 24;
    spielfeld[24].hausnummer = 13;
    spielfeld[24].anzahlHaeuser = 0;
    spielfeld[24].kostenHaus = 150;
    spielfeld[24].rgbNummer = 16;
    spielfeld[24].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Technikum
    strcpy(spielfeld[25].name, "Technikum");
    spielfeld[25].typ = HALTESTELLE;
    spielfeld[25].preis = 200;
    spielfeld[25].mieten[0] = 25;   //wenn man 1 Bahn besitzt
    spielfeld[25].mieten[1] = 50;   //wenn man 2 Bahnen besitzt
    spielfeld[25].mieten[2] = 100;  //wenn man 3 Bahnen besitzt
    spielfeld[25].mieten[3] = 200;  //wenn man 4 Bahnen besitzt
    spielfeld[25].besitzer = 0;
    spielfeld[25].farbGruppe = FARBLOS;
    spielfeld[25].farbgruppenFelder[0] = 5;
    spielfeld[25].farbgruppenFelder[1] = 15;
    spielfeld[25].farbgruppenFelder[2] = 25;
    spielfeld[25].farbgruppenFelder[3] = 35;
    spielfeld[25].rgbNummer = 17;
    spielfeld[25].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Lager Polymechaniker
    strcpy(spielfeld[26].name, "Lager PM");
    spielfeld[26].typ = STRASSE;
    spielfeld[26].preis = 260;
    spielfeld[26].mieten[0] = 22;   //Feld einzeln
    spielfeld[26].mieten[1] = 110;  //Feld mit 1 Haus
    spielfeld[26].mieten[2] = 330;  //Feld mit 2 Häuser
    spielfeld[26].mieten[3] = 800;  //Feld mit 3 Häuser
    spielfeld[26].mieten[4] = 975;  //Feld mit 4 Häuser
    spielfeld[26].mieten[5] = 1150; //Feld mit 5 Häuser
    spielfeld[26].mieten[6] = 44;   //Feld mit Farbgruppe
    spielfeld[26].besitzer = 0;
    spielfeld[26].farbGruppe = GELB;
    spielfeld[26].farbgruppenFelder[0] = 26;
    spielfeld[26].farbgruppenFelder[1] = 27;
    spielfeld[26].farbgruppenFelder[2] = 29;
    spielfeld[26].hausnummer = 14;
    spielfeld[26].anzahlHaeuser = 0;
    spielfeld[26].kostenHaus = 150;
    spielfeld[26].rgbNummer = 18;
    spielfeld[26].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Lager Automatiker
    strcpy(spielfeld[27].name, "Lager AU");
    spielfeld[27].typ = STRASSE;
    spielfeld[27].preis = 260;
    spielfeld[27].mieten[0] = 22;   //Feld einzeln
    spielfeld[27].mieten[1] = 110;  //Feld mit 1 Haus
    spielfeld[27].mieten[2] = 330;  //Feld mit 2 Häuser
    spielfeld[27].mieten[3] = 800;  //Feld mit 3 Häuser
    spielfeld[27].mieten[4] = 975;  //Feld mit 4 Häuser
    spielfeld[27].mieten[5] = 1150; //Feld mit 5 Häuser
    spielfeld[27].mieten[6] = 44;   //Feld mit Farbgruppe
    spielfeld[27].besitzer = 0;
    spielfeld[27].farbGruppe = GELB;
    spielfeld[27].farbgruppenFelder[0] = 26;
    spielfeld[27].farbgruppenFelder[1] = 27;
    spielfeld[27].farbgruppenFelder[2] = 29;
    spielfeld[27].hausnummer = 15;
    spielfeld[27].anzahlHaeuser = 0;
    spielfeld[27].kostenHaus = 150;
    spielfeld[27].rgbNummer = 19;
    spielfeld[27].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Putzdienst
    strcpy(spielfeld[28].name, "Putzdienst");
    spielfeld[28].typ = WERK;
    spielfeld[28].preis = 150;
    spielfeld[28].besitzer = 0;
    spielfeld[28].farbGruppe = FARBLOS;
    spielfeld[28].rgbNummer = 20;
    spielfeld[28].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Lager Elektroniker
    strcpy(spielfeld[29].name, "Lager EK");
    spielfeld[29].typ = STRASSE;
    spielfeld[29].preis = 280;
    spielfeld[29].mieten[0] = 24;   //Feld einzeln
    spielfeld[29].mieten[1] = 120;  //Feld mit 1 Haus
    spielfeld[29].mieten[2] = 360;  //Feld mit 2 Häuser
    spielfeld[29].mieten[3] = 850;  //Feld mit 3 Häuser
    spielfeld[29].mieten[4] = 1025; //Feld mit 4 Häuser
    spielfeld[29].mieten[5] = 1200; //Feld mit 5 Häuser
    spielfeld[29].mieten[6] = 48;   //Feld mit Farbgruppe
    spielfeld[29].besitzer = 0;
    spielfeld[29].farbGruppe = GELB;
    spielfeld[29].farbgruppenFelder[0] = 26;
    spielfeld[29].farbgruppenFelder[1] = 27;
    spielfeld[29].farbgruppenFelder[2] = 29;
    spielfeld[29].hausnummer = 16;
    spielfeld[29].anzahlHaeuser = 0;
    spielfeld[29].kostenHaus = 150;
    spielfeld[29].rgbNummer = 21;
    spielfeld[29].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Geh ins Gefängnis
    strcpy(spielfeld[30].name, "Geh ins Gefaengnis");
    spielfeld[30].typ = GEH_INS_GEFAENGNIS;
    
    //Eigenschaften des Feldes: Grundausbildung Elektroniker
    strcpy(spielfeld[31].name, "GA EK");
    spielfeld[31].typ = STRASSE;
    spielfeld[31].preis = 300;
    spielfeld[31].mieten[0] = 26;   //Feld einzeln
    spielfeld[31].mieten[1] = 130;  //Feld mit 1 Haus
    spielfeld[31].mieten[2] = 390;  //Feld mit 2 Häuser
    spielfeld[31].mieten[3] = 900;  //Feld mit 3 Häuser
    spielfeld[31].mieten[4] = 1100; //Feld mit 4 Häuser
    spielfeld[31].mieten[5] = 1275; //Feld mit 5 Häuser
    spielfeld[31].mieten[6] = 52;   //Feld mit Farbgruppe
    spielfeld[31].besitzer = 0;
    spielfeld[31].farbGruppe = GRUEN;
    spielfeld[31].farbgruppenFelder[0] = 31;
    spielfeld[31].farbgruppenFelder[1] = 32;
    spielfeld[31].farbgruppenFelder[2] = 34;
    spielfeld[31].hausnummer = 17;
    spielfeld[31].anzahlHaeuser = 0;
    spielfeld[31].kostenHaus = 200;
    spielfeld[31].rgbNummer = 22;
    spielfeld[31].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Produktion Elektroniker
    strcpy(spielfeld[32].name, "Produktion EK");
    spielfeld[32].typ = STRASSE;
    spielfeld[32].preis = 300;
    spielfeld[32].mieten[0] = 26;   //Feld einzeln
    spielfeld[32].mieten[1] = 130;  //Feld mit 1 Haus
    spielfeld[32].mieten[2] = 390;  //Feld mit 2 Häuser
    spielfeld[32].mieten[3] = 900;  //Feld mit 3 Häuser
    spielfeld[32].mieten[4] = 1100; //Feld mit 4 Häuser
    spielfeld[32].mieten[5] = 1275; //Feld mit 5 Häuser
    spielfeld[32].mieten[6] = 52;   //Feld mit Farbgruppe
    spielfeld[32].besitzer = 0;
    spielfeld[32].farbGruppe = GRUEN;
    spielfeld[32].farbgruppenFelder[0] = 31;
    spielfeld[32].farbgruppenFelder[1] = 32;
    spielfeld[32].farbgruppenFelder[2] = 34;
    spielfeld[32].hausnummer = 18;
    spielfeld[32].anzahlHaeuser = 0;
    spielfeld[32].kostenHaus = 200;
    spielfeld[32].rgbNummer = 23;
    spielfeld[32].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Kanzlei
    strcpy(spielfeld[33].name, "Kanzlei");
    spielfeld[33].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Entwicklung Elektroniker
    strcpy(spielfeld[34].name, "Entwicklung EK");
    spielfeld[34].typ = STRASSE;
    spielfeld[34].preis = 320;
    spielfeld[34].mieten[0] = 28;   //Feld einzeln
    spielfeld[34].mieten[1] = 150;  //Feld mit 1 Haus
    spielfeld[34].mieten[2] = 450;  //Feld mit 2 Häuser
    spielfeld[34].mieten[3] = 1000; //Feld mit 3 Häuser
    spielfeld[34].mieten[4] = 1200; //Feld mit 4 Häuser
    spielfeld[34].mieten[5] = 1400; //Feld mit 5 Häuser
    spielfeld[34].mieten[6] = 56;   //Feld mit Farbgruppe
    spielfeld[34].besitzer = 0;
    spielfeld[34].farbGruppe = GRUEN;
    spielfeld[34].farbgruppenFelder[0] = 31;
    spielfeld[34].farbgruppenFelder[1] = 32;
    spielfeld[34].farbgruppenFelder[2] = 34;
    spielfeld[34].hausnummer = 19;
    spielfeld[34].anzahlHaeuser = 0;
    spielfeld[34].kostenHaus = 200;
    spielfeld[34].rgbNummer = 24;
    spielfeld[34].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Gewerbeschule
    strcpy(spielfeld[35].name, "Gewerbeschule");
    spielfeld[35].typ = HALTESTELLE;
    spielfeld[35].preis = 200;
    spielfeld[35].mieten[0] = 25;   //wenn man 1 Bahn besitzt
    spielfeld[35].mieten[1] = 50;   //wenn man 2 Bahnen besitzt
    spielfeld[35].mieten[2] = 100;  //wenn man 3 Bahnen besitzt
    spielfeld[35].mieten[3] = 200;  //wenn man 4 Bahnen besitzt
    spielfeld[35].besitzer = 0;
    spielfeld[35].farbGruppe = FARBLOS;
    spielfeld[35].farbgruppenFelder[0] = 5;
    spielfeld[35].farbgruppenFelder[1] = 15;
    spielfeld[35].farbgruppenFelder[2] = 25;
    spielfeld[35].farbgruppenFelder[3] = 35;
    spielfeld[35].rgbNummer = 25;
    spielfeld[35].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Chance
    strcpy(spielfeld[36].name, "Chance");
    spielfeld[36].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Grundausbildung Polymechaniker
    strcpy(spielfeld[37].name, "GA PM");
    spielfeld[37].typ = STRASSE;
    spielfeld[37].preis = 350;
    spielfeld[37].mieten[0] = 35;   //Feld einzeln
    spielfeld[37].mieten[1] = 175;  //Feld mit 1 Haus
    spielfeld[37].mieten[2] = 500;  //Feld mit 2 Häuser
    spielfeld[37].mieten[3] = 1100; //Feld mit 3 Häuser
    spielfeld[37].mieten[4] = 1300; //Feld mit 4 Häuser
    spielfeld[37].mieten[5] = 1500; //Feld mit 5 Häuser
    spielfeld[37].mieten[6] = 70;   //Feld mit Farbgruppe
    spielfeld[37].besitzer = 0;
    spielfeld[37].farbGruppe = BLAU;
    spielfeld[37].farbgruppenFelder[0] = 37;
    spielfeld[37].farbgruppenFelder[1] = 39;
    spielfeld[37].farbgruppenFelder[2] = 0;
    spielfeld[37].hausnummer = 20;
    spielfeld[37].anzahlHaeuser = 0;
    spielfeld[37].kostenHaus = 200;
    spielfeld[37].rgbNummer = 26;
    spielfeld[37].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Schulmaterialkosten
    strcpy(spielfeld[38].name, "Schulmaterial");
    spielfeld[38].typ = STEUERFELD;
    spielfeld[38].preis = 100;
    
    //Eigenschaften des Feldes: Produktion Polymechaniker
    strcpy(spielfeld[39].name, "Produktion PM");
    spielfeld[39].typ = STRASSE;
    spielfeld[39].preis = 400;
    spielfeld[39].mieten[0] = 50;   //Feld einzeln
    spielfeld[39].mieten[1] = 200;  //Feld mit 1 Haus
    spielfeld[39].mieten[2] = 600;  //Feld mit 2 Häuser
    spielfeld[39].mieten[3] = 1400; //Feld mit 3 Häuser
    spielfeld[39].mieten[4] = 1700; //Feld mit 4 Häuser
    spielfeld[39].mieten[5] = 2000; //Feld mit 5 Häuser
    spielfeld[39].mieten[6] = 100;   //Feld mit Farbgruppe
    spielfeld[39].besitzer = 0;
    spielfeld[39].farbGruppe = BLAU;
    spielfeld[39].farbgruppenFelder[0] = 37;
    spielfeld[39].farbgruppenFelder[1] = 37;
    spielfeld[39].farbgruppenFelder[2] = 37;
    spielfeld[39].farbgruppenFelder[3] = 39;
    spielfeld[39].hausnummer = 21;
    spielfeld[39].anzahlHaeuser = 0;
    spielfeld[39].kostenHaus = 200;
    spielfeld[39].rgbNummer = 27;
    spielfeld[39].feldBelastet = 0;  //wenn das Feld belastet ist = 1
}


void initialisiereSpielfeldTest(Feld spielfeld[])
{
    //Eigenschaften des Feldes: Los
    strcpy(spielfeld[0].name, "Los");
    spielfeld[0].typ = FREIPARKEN;



    //Eigenschaften des Feldes: Eingangshalle
    strcpy(spielfeld[1].name, "Eingangshalle");
    spielfeld[1].typ = STRASSE;
    spielfeld[1].preis = 60;
    spielfeld[1].mieten[0] = 2;     //Feld einzeln
    spielfeld[1].mieten[1] = 10;    //Feld mit 1 Haus
    spielfeld[1].mieten[2] = 30;    //Feld mit 2 Häuser
    spielfeld[1].mieten[3] = 90;    //Feld mit 3 Häuser
    spielfeld[1].mieten[4] = 160;   //Feld mit 4 Häuser
    spielfeld[1].mieten[5] = 250;   //Feld mit 1 Hotel
    spielfeld[1].mieten[6] = 4;     //Feld mit Farbgrupp
    spielfeld[1].besitzer = 0;
    spielfeld[1].farbGruppe = BRAUN;
    spielfeld[1].farbgruppenFelder[0] = 1;
    spielfeld[1].farbgruppenFelder[1] = 3;
    spielfeld[1].farbgruppenFelder[2] = 0;
    spielfeld[1].hausnummer = 0;
    spielfeld[1].anzahlHaeuser = 0;
    spielfeld[1].kostenHaus = 50;
    spielfeld[1].rgbNummer = 0;
    spielfeld[1].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Kanzlei
    strcpy(spielfeld[2].name, "Kanzlei1");
    spielfeld[2].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Velokeller
    strcpy(spielfeld[3].name, "Velokeller");
    spielfeld[3].typ = STRASSE;
    spielfeld[3].preis = 60;
    spielfeld[3].preis = 60;
    spielfeld[3].mieten[0] = 4;     //Feld einzeln
    spielfeld[3].mieten[1] = 20;    //Feld mit 1 Haus
    spielfeld[3].mieten[2] = 60;    //Feld mit 2 Häuser
    spielfeld[3].mieten[3] = 180;   //Feld mit 3 Häuser
    spielfeld[3].mieten[4] = 320;   //Feld mit 4 Häuser
    spielfeld[3].mieten[5] = 450;   //Feld mit 1 Hotel
    spielfeld[3].mieten[6] = 4;     //Feld mit Farbgruppe
    spielfeld[3].besitzer = 2;
    spielfeld[3].farbGruppe = BRAUN;
    spielfeld[3].farbgruppenFelder[0] = 1;
    spielfeld[3].farbgruppenFelder[1] = 3;
    spielfeld[3].farbgruppenFelder[2] = 0;
    spielfeld[3].hausnummer = 1;
    spielfeld[3].anzahlHaeuser = 0;
    spielfeld[3].kostenHaus = 50;
    spielfeld[3].rgbNummer = 1;
    spielfeld[3].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    
    //Eigenschaften des Feldes: Laptopgebühr
    strcpy(spielfeld[4].name, "Laptopgeb"UE"hr");
    spielfeld[4].typ = STEUERFELD;
    spielfeld[4].preis = 200;
    
    //Eigenschaften des Feldes: Zeughausstrasse
    strcpy(spielfeld[5].name, "Zeughausstrasse");
    spielfeld[5].typ = HALTESTELLE;
    spielfeld[5].preis = 200;
    spielfeld[5].mieten[0] = 25;    //wenn man 1 Bahn besitzt
    spielfeld[5].mieten[1] = 50;    //wenn man 2 Bahnen besitzt
    spielfeld[5].mieten[2] = 100;   //wenn man 3 Bahnen besitzt
    spielfeld[5].mieten[3] = 200;   //wenn man 4 Bahnen besitzt
    spielfeld[5].besitzer = 1;
    spielfeld[5].farbGruppe = FARBLOS;
    spielfeld[5].farbgruppenFelder[0] = 5;
    spielfeld[5].farbgruppenFelder[1] = 15;
    spielfeld[5].farbgruppenFelder[2] = 25;
    spielfeld[5].farbgruppenFelder[3] = 35;
    spielfeld[5].rgbNummer = 2;
    spielfeld[5].feldBelastet = 0;  //wenn das Feld belastet ist = 1

    
    //Eigenschaften des Feldes: Raucherzelt
    strcpy(spielfeld[6].name, "Raucherzelt");
    spielfeld[6].typ = STRASSE;
    spielfeld[6].preis = 100;
    spielfeld[6].mieten[0] = 6;     //Feld einzeln
    spielfeld[6].mieten[1] = 30;    //Feld mit 1 Haus
    spielfeld[6].mieten[2] = 90;    //Feld mit 2 Häuser
    spielfeld[6].mieten[3] = 270;   //Feld mit 3 Häuser
    spielfeld[6].mieten[4] = 400;   //Feld mit 4 Häuser
    spielfeld[6].mieten[5] = 550;   //Feld mit 5 Häuser
    spielfeld[6].mieten[6] = 12;    //Feld mit Farbgruppe
    spielfeld[6].besitzer = 2;
    spielfeld[6].farbGruppe = HELLBLAU;
    spielfeld[6].farbgruppenFelder[0] = 6;
    spielfeld[6].farbgruppenFelder[1] = 8;
    spielfeld[6].farbgruppenFelder[2] = 9;
    spielfeld[6].hausnummer = 2;
    spielfeld[6].anzahlHaeuser = 5;
    spielfeld[6].kostenHaus = 50;
    spielfeld[6].rgbNummer = 3;
    spielfeld[6].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    
    //Eigenschaften des Feldes: Chance
    strcpy(spielfeld[7].name, "Chance");
    spielfeld[7].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Pausenraum
    strcpy(spielfeld[8].name, "Pausenraum");
    spielfeld[8].typ = STRASSE;
    spielfeld[8].preis = 100;
    spielfeld[8].mieten[0] = 6;     //Feld einzeln
    spielfeld[8].mieten[1] = 30;    //Feld mit 1 Haus
    spielfeld[8].mieten[2] = 90;    //Feld mit 2 Häuser
    spielfeld[8].mieten[3] = 270;   //Feld mit 3 Häuser
    spielfeld[8].mieten[4] = 400;   //Feld mit 4 Häuser
    spielfeld[8].mieten[5] = 550;   //Feld mit 5 Häuser
    spielfeld[8].mieten[6] = 12;    //Feld mit Farbgruppe
    spielfeld[8].besitzer = 2;
    spielfeld[8].farbGruppe = HELLBLAU;
    spielfeld[8].farbgruppenFelder[0] = 6;
    spielfeld[8].farbgruppenFelder[1] = 8;
    spielfeld[8].farbgruppenFelder[2] = 9;
    spielfeld[8].hausnummer = 3;
    spielfeld[8].anzahlHaeuser = 5;
    spielfeld[8].kostenHaus = 50;
    spielfeld[8].rgbNummer = 4;
    spielfeld[8].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    

    
    //Eigenschaften des Feldes: Garderobe
    strcpy(spielfeld[9].name, "Garderobe");
    spielfeld[9].typ = STRASSE;
    spielfeld[9].preis = 120;
    spielfeld[9].mieten[0] = 8;     //Feld einzeln
    spielfeld[9].mieten[1] = 40;    //Feld mit 1 Haus
    spielfeld[9].mieten[2] = 100;   //Feld mit 2 Häuser
    spielfeld[9].mieten[3] = 300;   //Feld mit 3 Häuser
    spielfeld[9].mieten[4] = 450;   //Feld mit 4 Häuser
    spielfeld[9].mieten[5] = 600;   //Feld mit 5 Häuser
    spielfeld[9].mieten[6] = 16;    //Feld mit Farbgruppe
    spielfeld[9].besitzer = 2;
    spielfeld[9].farbGruppe = HELLBLAU;
    spielfeld[9].farbgruppenFelder[0] = 6;
    spielfeld[9].farbgruppenFelder[1] = 8;
    spielfeld[9].farbgruppenFelder[2] = 9;
    spielfeld[9].hausnummer = 4;
    spielfeld[9].anzahlHaeuser = 5;
    spielfeld[9].kostenHaus = 50;
    spielfeld[9].rgbNummer = 5;
    spielfeld[9].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Gefängnis
    strcpy(spielfeld[10].name, "Gef"AE"ngnis");
    spielfeld[10].typ = GEFAENGNIS;
    
    
    
    
    //Eigenschaften des Feldes: Herren Wc
    strcpy(spielfeld[11].name, "Herren WC");
    spielfeld[11].typ = STRASSE;
    spielfeld[11].preis = 140;
    spielfeld[11].mieten[0] = 10;   //Feld einzeln
    spielfeld[11].mieten[1] = 50;   //Feld mit 1 Haus
    spielfeld[11].mieten[2] = 150;  //Feld mit 2 Häuser
    spielfeld[11].mieten[3] = 450;  //Feld mit 3 Häuser
    spielfeld[11].mieten[4] = 625;  //Feld mit 4 Häuser
    spielfeld[11].mieten[5] = 750;  //Feld mit 5 Häuser
    spielfeld[11].mieten[6] = 20;   //Feld mit Farbgruppe
    spielfeld[11].besitzer = 1;
    spielfeld[11].farbGruppe = ROSA;
    spielfeld[11].farbgruppenFelder[0] = 11;
    spielfeld[11].farbgruppenFelder[1] = 13;
    spielfeld[11].farbgruppenFelder[2] = 14;
    spielfeld[11].hausnummer = 5;
    spielfeld[11].anzahlHaeuser = 2;
    spielfeld[11].kostenHaus = 100;
    spielfeld[11].rgbNummer = 6;
    spielfeld[11].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Informatikdienst
    strcpy(spielfeld[12].name, "Informatikdienst");
    spielfeld[12].typ = WERK;
    spielfeld[12].preis = 150;
    spielfeld[12].besitzer = 3;
    spielfeld[12].farbGruppe = FARBLOS;
    spielfeld[12].rgbNummer = 7;
    spielfeld[12].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Frauen WC
    strcpy(spielfeld[13].name, "Frauen WC");
    spielfeld[13].typ = STRASSE;
    spielfeld[13].preis = 140;
    spielfeld[13].mieten[0] = 10;   //Feld einzeln
    spielfeld[13].mieten[1] = 50;   //Feld mit 1 Haus
    spielfeld[13].mieten[2] = 150;  //Feld mit 2 Häuser
    spielfeld[13].mieten[3] = 450;  //Feld mit 3 Häuser
    spielfeld[13].mieten[4] = 625;  //Feld mit 4 Häuser
    spielfeld[13].mieten[5] = 750;  //Feld mit 5 Häuser
    spielfeld[13].mieten[6] = 20;   //Feld mit Farbgruppe
    spielfeld[13].besitzer = 1;
    spielfeld[13].farbGruppe = ROSA;
    spielfeld[13].farbgruppenFelder[0] = 11;
    spielfeld[13].farbgruppenFelder[1] = 13;
    spielfeld[13].farbgruppenFelder[2] = 14;
    spielfeld[13].hausnummer = 6;
    spielfeld[13].anzahlHaeuser = 2;
    spielfeld[13].kostenHaus = 100;
    spielfeld[13].rgbNummer = 8;
    spielfeld[13].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Lehrer WC
    strcpy(spielfeld[14].name, "Lehrer WC");
    spielfeld[14].typ = STRASSE;
    spielfeld[14].preis = 160;
    spielfeld[14].mieten[0] = 12;   //Feld einzeln
    spielfeld[14].mieten[1] = 60;   //Feld mit 1 Haus
    spielfeld[14].mieten[2] = 180;  //Feld mit 2 Häuser
    spielfeld[14].mieten[3] = 500;  //Feld mit 3 Häuser
    spielfeld[14].mieten[4] = 700;  //Feld mit 4 Häuser
    spielfeld[14].mieten[5] = 900;  //Feld mit 5 Häuser
    spielfeld[14].mieten[6] = 24;   //Feld mit Farbgruppe
    spielfeld[14].besitzer = 1;
    spielfeld[14].farbGruppe = ROSA;
    spielfeld[14].farbgruppenFelder[0] = 11;
    spielfeld[14].farbgruppenFelder[1] = 13;
    spielfeld[14].farbgruppenFelder[2] = 14;
    spielfeld[14].hausnummer = 7;
    spielfeld[14].anzahlHaeuser = 2;
    spielfeld[14].kostenHaus = 100;
    spielfeld[14].rgbNummer = 9;
    spielfeld[14].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Fotozentrum
    strcpy(spielfeld[15].name, "Fotozentrum");
    spielfeld[15].typ = HALTESTELLE;
    spielfeld[15].preis = 200;
    spielfeld[15].mieten[0] = 25;   //wenn man 1 Bahn besitzt
    spielfeld[15].mieten[1] = 50;   //wenn man 2 Bahnen besitzt
    spielfeld[15].mieten[2] = 100;  //wenn man 3 Bahnen besitzt
    spielfeld[15].mieten[3] = 200;  //wenn man 4 Bahnen besitzt
    spielfeld[15].besitzer = 2;
    spielfeld[15].farbGruppe = FARBLOS;
    spielfeld[15].farbgruppenFelder[0] = 5;
    spielfeld[15].farbgruppenFelder[1] = 15;
    spielfeld[15].farbgruppenFelder[2] = 25;
    spielfeld[15].farbgruppenFelder[3] = 35;
    spielfeld[15].rgbNummer = 10;
    spielfeld[15].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: BFS Polimechaniker
    strcpy(spielfeld[16].name, "BFS PM");
    spielfeld[16].typ = STRASSE;
    spielfeld[16].preis = 180;
    spielfeld[16].mieten[0] = 14;   //Feld einzeln
    spielfeld[16].mieten[1] = 70;   //Feld mit 1 Haus
    spielfeld[16].mieten[2] = 200;  //Feld mit 2 Häuser
    spielfeld[16].mieten[3] = 550;  //Feld mit 3 Häuser
    spielfeld[16].mieten[4] = 750;  //Feld mit 4 Häuser
    spielfeld[16].mieten[5] = 950;  //Feld mit 5 Häuser
    spielfeld[16].mieten[6] = 28;   //Feld mit Farbgruppe
    spielfeld[16].besitzer = 4;
    spielfeld[16].farbGruppe = ORANGE;
    spielfeld[16].farbgruppenFelder[0] = 16;
    spielfeld[16].farbgruppenFelder[1] = 18;
    spielfeld[16].farbgruppenFelder[2] = 19;
    spielfeld[16].hausnummer = 8;
    spielfeld[16].anzahlHaeuser = 0;
    spielfeld[16].kostenHaus = 100;
    spielfeld[16].rgbNummer = 11;
    spielfeld[16].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Kanzlei
    strcpy(spielfeld[17].name, "Kanzlei");
    spielfeld[17].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: BFS Automatiker
    strcpy(spielfeld[18].name, "BFS AU");
    spielfeld[18].typ = STRASSE;
    spielfeld[18].preis = 180;
    spielfeld[18].mieten[0] = 14;   //Feld einzeln
    spielfeld[18].mieten[1] = 70;   //Feld mit 1 Haus
    spielfeld[18].mieten[2] = 200;  //Feld mit 2 Häuser
    spielfeld[18].mieten[3] = 550;  //Feld mit 3 Häuser
    spielfeld[18].mieten[4] = 750;  //Feld mit 4 Häuser
    spielfeld[18].mieten[5] = 950;  //Feld mit 5 Häuser
    spielfeld[18].mieten[6] = 28;   //Feld mit Farbgruppe
    spielfeld[18].besitzer = 2;
    spielfeld[18].farbGruppe = ORANGE;
    spielfeld[18].farbgruppenFelder[0] = 16;
    spielfeld[18].farbgruppenFelder[1] = 18;
    spielfeld[18].farbgruppenFelder[2] = 19;
    spielfeld[18].hausnummer = 9;
    spielfeld[18].anzahlHaeuser = 0;
    spielfeld[18].kostenHaus = 100;
    spielfeld[18].rgbNummer = 12;
    spielfeld[18].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: BFS Elektroniker
    strcpy(spielfeld[19].name, "BFS EK");
    spielfeld[19].typ = STRASSE;
    spielfeld[19].preis = 200;
    spielfeld[19].mieten[0] = 16;   //Feld einzeln
    spielfeld[19].mieten[1] = 80;   //Feld mit 1 Haus
    spielfeld[19].mieten[2] = 220;  //Feld mit 2 Häuser
    spielfeld[19].mieten[3] = 600;  //Feld mit 3 Häuser
    spielfeld[19].mieten[4] = 800;  //Feld mit 4 Häuser
    spielfeld[19].mieten[5] = 1000; //Feld mit 5 Häuser
    spielfeld[19].mieten[6] = 32;   //Feld mit Farbgruppe
    spielfeld[19].besitzer = 2;
    spielfeld[19].farbGruppe = ORANGE;
    spielfeld[19].farbgruppenFelder[0] = 16;
    spielfeld[19].farbgruppenFelder[1] = 18;
    spielfeld[19].farbgruppenFelder[2] = 19;
    spielfeld[19].hausnummer = 10;
    spielfeld[19].anzahlHaeuser = 0;
    spielfeld[19].kostenHaus = 100;
    spielfeld[19].rgbNummer = 13;
    spielfeld[19].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Freiparken
    strcpy(spielfeld[20].name, "Freiparken");
    spielfeld[20].typ = FREIPARKEN;
    
    //Eigenschaften des Feldes: Grundausbildung Automatiker
    strcpy(spielfeld[21].name, "GA AU");
    spielfeld[21].typ = STRASSE;
    spielfeld[21].preis = 220;
    spielfeld[21].mieten[0] = 18;   //Feld einzeln
    spielfeld[21].mieten[1] = 90;   //Feld mit 1 Haus
    spielfeld[21].mieten[2] = 250;  //Feld mit 2 Häuser
    spielfeld[21].mieten[3] = 700;  //Feld mit 3 Häuser
    spielfeld[21].mieten[4] = 875;  //Feld mit 4 Häuser
    spielfeld[21].mieten[5] = 1050; //Feld mit 5 Häuser
    spielfeld[21].mieten[6] = 36;   //Feld mit Farbgruppe
    spielfeld[21].besitzer = 3;
    spielfeld[21].farbGruppe = ROT;
    spielfeld[21].farbgruppenFelder[0] = 21;
    spielfeld[21].farbgruppenFelder[1] = 23;
    spielfeld[21].farbgruppenFelder[2] = 24;
    spielfeld[21].hausnummer = 11;
    spielfeld[21].anzahlHaeuser = 0;
    spielfeld[21].kostenHaus = 150;
    spielfeld[21].rgbNummer = 14;
    spielfeld[21].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Chance
    strcpy(spielfeld[22].name, "Chance");
    spielfeld[22].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Produkton Automatiker
    strcpy(spielfeld[23].name, "Produktion AU");
    spielfeld[23].typ = STRASSE;
    spielfeld[23].preis = 220;
    spielfeld[23].mieten[0] = 18;   //Feld einzeln
    spielfeld[23].mieten[1] = 90;   //Feld mit 1 Haus
    spielfeld[23].mieten[2] = 250;  //Feld mit 2 Häuser
    spielfeld[23].mieten[3] = 700;  //Feld mit 3 Häuser
    spielfeld[23].mieten[4] = 875;  //Feld mit 4 Häuser
    spielfeld[23].mieten[5] = 1050; //Feld mit 5 Häuser
    spielfeld[23].mieten[6] = 36;   //Feld mit Farbgruppe
    spielfeld[23].besitzer = 3;
    spielfeld[23].farbGruppe = ROT;
    spielfeld[23].farbgruppenFelder[0] = 21;
    spielfeld[23].farbgruppenFelder[1] = 23;
    spielfeld[23].farbgruppenFelder[2] = 24;
    spielfeld[23].hausnummer = 12;
    spielfeld[23].anzahlHaeuser = 0;
    spielfeld[23].kostenHaus = 150;
    spielfeld[23].rgbNummer = 15;
    spielfeld[23].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Mechatronik Labor
    strcpy(spielfeld[24].name, "Mechatr. Labor");
    spielfeld[24].typ = STRASSE;
    spielfeld[24].preis = 240;
    spielfeld[24].mieten[0] = 20;   //Feld einzeln
    spielfeld[24].mieten[1] = 100;  //Feld mit 1 Haus
    spielfeld[24].mieten[2] = 300;  //Feld mit 2 Häuser
    spielfeld[24].mieten[3] = 750;  //Feld mit 3 Häuser
    spielfeld[24].mieten[4] = 925;  //Feld mit 4 Häuser
    spielfeld[24].mieten[5] = 1100; //Feld mit 5 Häuser
    spielfeld[24].mieten[6] = 40;   //Feld mit Farbgruppe
    spielfeld[24].besitzer = 3;
    spielfeld[24].farbGruppe = ROT;
    spielfeld[24].farbgruppenFelder[0] = 21;
    spielfeld[24].farbgruppenFelder[1] = 23;
    spielfeld[24].farbgruppenFelder[2] = 24;
    spielfeld[24].hausnummer = 13;
    spielfeld[24].anzahlHaeuser = 0;
    spielfeld[24].kostenHaus = 150;
    spielfeld[24].rgbNummer = 16;
    spielfeld[24].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Technikum
    strcpy(spielfeld[25].name, "Technikum");
    spielfeld[25].typ = HALTESTELLE;
    spielfeld[25].preis = 200;
    spielfeld[25].mieten[0] = 25;   //wenn man 1 Bahn besitzt
    spielfeld[25].mieten[1] = 50;   //wenn man 2 Bahnen besitzt
    spielfeld[25].mieten[2] = 100;  //wenn man 3 Bahnen besitzt
    spielfeld[25].mieten[3] = 200;  //wenn man 4 Bahnen besitzt
    spielfeld[25].besitzer = 1;
    spielfeld[25].farbGruppe = FARBLOS;
    spielfeld[25].farbgruppenFelder[0] = 5;
    spielfeld[25].farbgruppenFelder[1] = 15;
    spielfeld[25].farbgruppenFelder[2] = 25;
    spielfeld[25].farbgruppenFelder[3] = 35;
    spielfeld[25].rgbNummer = 17;
    spielfeld[25].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Lager Polymechaniker
    strcpy(spielfeld[26].name, "Lager PM");
    spielfeld[26].typ = STRASSE;
    spielfeld[26].preis = 260;
    spielfeld[26].mieten[0] = 22;   //Feld einzeln
    spielfeld[26].mieten[1] = 110;  //Feld mit 1 Haus
    spielfeld[26].mieten[2] = 330;  //Feld mit 2 Häuser
    spielfeld[26].mieten[3] = 800;  //Feld mit 3 Häuser
    spielfeld[26].mieten[4] = 975;  //Feld mit 4 Häuser
    spielfeld[26].mieten[5] = 1150; //Feld mit 5 Häuser
    spielfeld[26].mieten[6] = 44;   //Feld mit Farbgruppe
    spielfeld[26].besitzer = 4;
    spielfeld[26].farbGruppe = GELB;
    spielfeld[26].farbgruppenFelder[0] = 26;
    spielfeld[26].farbgruppenFelder[1] = 27;
    spielfeld[26].farbgruppenFelder[2] = 29;
    spielfeld[26].hausnummer = 14;
    spielfeld[26].anzahlHaeuser = 1;
    spielfeld[26].kostenHaus = 150;
    spielfeld[26].rgbNummer = 18;
    spielfeld[26].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Lager Automatiker
    strcpy(spielfeld[27].name, "Lager AU");
    spielfeld[27].typ = STRASSE;
    spielfeld[27].preis = 260;
    spielfeld[27].mieten[0] = 22;   //Feld einzeln
    spielfeld[27].mieten[1] = 110;  //Feld mit 1 Haus
    spielfeld[27].mieten[2] = 330;  //Feld mit 2 Häuser
    spielfeld[27].mieten[3] = 800;  //Feld mit 3 Häuser
    spielfeld[27].mieten[4] = 975;  //Feld mit 4 Häuser
    spielfeld[27].mieten[5] = 1150; //Feld mit 5 Häuser
    spielfeld[27].mieten[6] = 44;   //Feld mit Farbgruppe
    spielfeld[27].besitzer = 4;
    spielfeld[27].farbGruppe = GELB;
    spielfeld[27].farbgruppenFelder[0] = 26;
    spielfeld[27].farbgruppenFelder[1] = 27;
    spielfeld[27].farbgruppenFelder[2] = 29;
    spielfeld[27].hausnummer = 15;
    spielfeld[27].anzahlHaeuser = 1;
    spielfeld[27].kostenHaus = 150;
    spielfeld[27].rgbNummer = 19;
    spielfeld[27].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Putzdienst
    strcpy(spielfeld[28].name, "Putzdienst");
    spielfeld[28].typ = WERK;
    spielfeld[28].preis = 150;
    spielfeld[28].besitzer = 3;
    spielfeld[28].farbGruppe = FARBLOS;
    spielfeld[28].rgbNummer = 20;
    spielfeld[28].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Lager Elektroniker
    strcpy(spielfeld[29].name, "Lager EK");
    spielfeld[29].typ = STRASSE;
    spielfeld[29].preis = 280;
    spielfeld[29].mieten[0] = 24;   //Feld einzeln
    spielfeld[29].mieten[1] = 120;  //Feld mit 1 Haus
    spielfeld[29].mieten[2] = 360;  //Feld mit 2 Häuser
    spielfeld[29].mieten[3] = 850;  //Feld mit 3 Häuser
    spielfeld[29].mieten[4] = 1025; //Feld mit 4 Häuser
    spielfeld[29].mieten[5] = 1200; //Feld mit 5 Häuser
    spielfeld[29].mieten[6] = 48;   //Feld mit Farbgruppe
    spielfeld[29].besitzer = 4;
    spielfeld[29].farbGruppe = GELB;
    spielfeld[29].farbgruppenFelder[0] = 26;
    spielfeld[29].farbgruppenFelder[1] = 27;
    spielfeld[29].farbgruppenFelder[2] = 29;
    spielfeld[29].hausnummer = 16;
    spielfeld[29].anzahlHaeuser = 1;
    spielfeld[29].kostenHaus = 150;
    spielfeld[29].rgbNummer = 21;
    spielfeld[29].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Geh ins Gefängnis
    strcpy(spielfeld[30].name, "Geh ins Gefaengnis");
    spielfeld[30].typ = GEH_INS_GEFAENGNIS;
    
    //Eigenschaften des Feldes: Grundausbildung Elektroniker
    strcpy(spielfeld[31].name, "GA EK");
    spielfeld[31].typ = STRASSE;
    spielfeld[31].preis = 300;
    spielfeld[31].mieten[0] = 26;   //Feld einzeln
    spielfeld[31].mieten[1] = 130;  //Feld mit 1 Haus
    spielfeld[31].mieten[2] = 390;  //Feld mit 2 Häuser
    spielfeld[31].mieten[3] = 900;  //Feld mit 3 Häuser
    spielfeld[31].mieten[4] = 1100; //Feld mit 4 Häuser
    spielfeld[31].mieten[5] = 1275; //Feld mit 5 Häuser
    spielfeld[31].mieten[6] = 52;   //Feld mit Farbgruppe
    spielfeld[31].besitzer = 1;
    spielfeld[31].farbGruppe = GRUEN;
    spielfeld[31].farbgruppenFelder[0] = 31;
    spielfeld[31].farbgruppenFelder[1] = 32;
    spielfeld[31].farbgruppenFelder[2] = 34;
    spielfeld[31].hausnummer = 17;
    spielfeld[31].anzahlHaeuser = 0;
    spielfeld[31].kostenHaus = 200;
    spielfeld[31].rgbNummer = 22;
    spielfeld[31].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Produktion Elektroniker
    strcpy(spielfeld[32].name, "Produktion EK");
    spielfeld[32].typ = STRASSE;
    spielfeld[32].preis = 300;
    spielfeld[32].mieten[0] = 26;   //Feld einzeln
    spielfeld[32].mieten[1] = 130;  //Feld mit 1 Haus
    spielfeld[32].mieten[2] = 390;  //Feld mit 2 Häuser
    spielfeld[32].mieten[3] = 900;  //Feld mit 3 Häuser
    spielfeld[32].mieten[4] = 1100; //Feld mit 4 Häuser
    spielfeld[32].mieten[5] = 1275; //Feld mit 5 Häuser
    spielfeld[32].mieten[6] = 52;   //Feld mit Farbgruppe
    spielfeld[32].besitzer = 1;
    spielfeld[32].farbGruppe = GRUEN;
    spielfeld[32].farbgruppenFelder[0] = 31;
    spielfeld[32].farbgruppenFelder[1] = 32;
    spielfeld[32].farbgruppenFelder[2] = 34;
    spielfeld[32].hausnummer = 18;
    spielfeld[32].anzahlHaeuser = 0;
    spielfeld[32].kostenHaus = 200;
    spielfeld[32].rgbNummer = 23;
    spielfeld[32].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Kanzlei
    strcpy(spielfeld[33].name, "Kanzlei");
    spielfeld[33].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Entwicklung Elektroniker
    strcpy(spielfeld[34].name, "Entwicklung EK");
    spielfeld[34].typ = STRASSE;
    spielfeld[34].preis = 320;
    spielfeld[34].mieten[0] = 28;   //Feld einzeln
    spielfeld[34].mieten[1] = 150;  //Feld mit 1 Haus
    spielfeld[34].mieten[2] = 450;  //Feld mit 2 Häuser
    spielfeld[34].mieten[3] = 1000; //Feld mit 3 Häuser
    spielfeld[34].mieten[4] = 1200; //Feld mit 4 Häuser
    spielfeld[34].mieten[5] = 1400; //Feld mit 5 Häuser
    spielfeld[34].mieten[6] = 56;   //Feld mit Farbgruppe
    spielfeld[34].besitzer = 1;
    spielfeld[34].farbGruppe = GRUEN;
    spielfeld[34].farbgruppenFelder[0] = 31;
    spielfeld[34].farbgruppenFelder[1] = 32;
    spielfeld[34].farbgruppenFelder[2] = 34;
    spielfeld[34].hausnummer = 19;
    spielfeld[34].anzahlHaeuser = 0;
    spielfeld[34].kostenHaus = 200;
    spielfeld[34].rgbNummer = 24;
    spielfeld[34].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Gewerbeschule
    strcpy(spielfeld[35].name, "Gewerbeschule");
    spielfeld[35].typ = HALTESTELLE;
    spielfeld[35].preis = 200;
    spielfeld[35].mieten[0] = 25;   //wenn man 1 Bahn besitzt
    spielfeld[35].mieten[1] = 50;   //wenn man 2 Bahnen besitzt
    spielfeld[35].mieten[2] = 100;  //wenn man 3 Bahnen besitzt
    spielfeld[35].mieten[3] = 200;  //wenn man 4 Bahnen besitzt
    spielfeld[35].besitzer = 2;
    spielfeld[35].farbGruppe = FARBLOS;
    spielfeld[35].farbgruppenFelder[0] = 5;
    spielfeld[35].farbgruppenFelder[1] = 15;
    spielfeld[35].farbgruppenFelder[2] = 25;
    spielfeld[35].farbgruppenFelder[3] = 35;
    spielfeld[35].rgbNummer = 25;
    spielfeld[35].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Chance
    strcpy(spielfeld[36].name, "Chance");
    spielfeld[36].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Grundausbildung Polymechaniker
    strcpy(spielfeld[37].name, "GA PM");
    spielfeld[37].typ = STRASSE;
    spielfeld[37].preis = 350;
    spielfeld[37].mieten[0] = 35;   //Feld einzeln
    spielfeld[37].mieten[1] = 175;  //Feld mit 1 Haus
    spielfeld[37].mieten[2] = 500;  //Feld mit 2 Häuser
    spielfeld[37].mieten[3] = 1100; //Feld mit 3 Häuser
    spielfeld[37].mieten[4] = 1300; //Feld mit 4 Häuser
    spielfeld[37].mieten[5] = 1500; //Feld mit 5 Häuser
    spielfeld[37].mieten[6] = 70;   //Feld mit Farbgruppe
    spielfeld[37].besitzer = 3;
    spielfeld[37].farbGruppe = BLAU;
    spielfeld[37].farbgruppenFelder[0] = 37;
    spielfeld[37].farbgruppenFelder[1] = 39;
    spielfeld[37].farbgruppenFelder[2] = 0;
    spielfeld[37].hausnummer = 20;
    spielfeld[37].anzahlHaeuser = 0;
    spielfeld[37].kostenHaus = 200;
    spielfeld[37].rgbNummer = 26;
    spielfeld[37].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Schulmaterialkosten
    strcpy(spielfeld[38].name, "Schulmaterial");
    spielfeld[38].typ = STEUERFELD;
    spielfeld[38].preis = 100;
    
    //Eigenschaften des Feldes: Produktion Polymechaniker
    strcpy(spielfeld[39].name, "Produktion PM");
    spielfeld[39].typ = STRASSE;
    spielfeld[39].preis = 400;
    spielfeld[39].mieten[0] = 50;   //Feld einzeln
    spielfeld[39].mieten[1] = 200;  //Feld mit 1 Haus
    spielfeld[39].mieten[2] = 600;  //Feld mit 2 Häuser
    spielfeld[39].mieten[3] = 1400; //Feld mit 3 Häuser
    spielfeld[39].mieten[4] = 1700; //Feld mit 4 Häuser
    spielfeld[39].mieten[5] = 2000; //Feld mit 5 Häuser
    spielfeld[39].mieten[6] = 100;   //Feld mit Farbgruppe
    spielfeld[39].besitzer = 3;
    spielfeld[39].farbGruppe = BLAU;
    spielfeld[39].farbgruppenFelder[0] = 37;
    spielfeld[39].farbgruppenFelder[1] = 37;
    spielfeld[39].farbgruppenFelder[2] = 37;
    spielfeld[39].farbgruppenFelder[3] = 39;
    spielfeld[39].hausnummer = 21;
    spielfeld[39].anzahlHaeuser = 0;
    spielfeld[39].kostenHaus = 200;
    spielfeld[39].rgbNummer = 27;
    spielfeld[39].feldBelastet = 0;  //wenn das Feld belastet ist = 1
}
/******************************************************************************\
* initialisiereKarten
*
* Initialisiert die Chance- und Kanzleikarten.
*
* Parameter:
*   - chanceKanzlei[]: Array von Karten
*
* Rückgabewert: keiner
\******************************************************************************/
void initialisiereKarten(Karte chanceKanzlei[])
{
    chanceKanzlei[0].typ = WORKSHOP;
    
    chanceKanzlei[1].typ = FREIKARTE;
    
    chanceKanzlei[2].typ = BEWEGEN;
    chanceKanzlei[2].zielFeldTyp = WERK;
    chanceKanzlei[2].bewegung = 0;
    
    chanceKanzlei[3].typ = GELD_AN_BANK;
    chanceKanzlei[3].geld = 50;
    
    chanceKanzlei[4].typ = TELEPORTIEREN;
    chanceKanzlei[4].zielFeld = 39;
    
    chanceKanzlei[5].typ = BEWEGEN;
    chanceKanzlei[5].zielFeldTyp = HALTESTELLE;
    chanceKanzlei[5].bewegung = 0;
    
    chanceKanzlei[6].typ = BEWEGEN;
    chanceKanzlei[6].zielFeldTyp = HALTESTELLE;
    chanceKanzlei[6].bewegung = 0;
    
    chanceKanzlei[7].typ = TELEPORTIEREN;
    chanceKanzlei[7].zielFeld = 31;
    
    chanceKanzlei[8].typ = TELEPORTIEREN;
    chanceKanzlei[8].zielFeld = 24;
    
    chanceKanzlei[9].typ = TELEPORTIEREN;
    chanceKanzlei[9].zielFeld = 0;
    
    chanceKanzlei[10].typ = TELEPORTIEREN;
    chanceKanzlei[10].zielFeld = 25;
    
    chanceKanzlei[11].typ = GELD_VON_BANK;
    chanceKanzlei[11].geld = 70;
    
    chanceKanzlei[12].typ = GELD_VON_BANK;
    chanceKanzlei[12].geld = 140;
    
    chanceKanzlei[13].typ = BEWEGEN;
    chanceKanzlei[13].bewegung = -3;
    
    chanceKanzlei[14].typ = RENOVIEREN;
    chanceKanzlei[14].geld = 25;
    chanceKanzlei[14].geld2 = 100;
    
    chanceKanzlei[15].typ = GELD_AN_MITSPIELER;
    chanceKanzlei[15].geld = 50;
    
    chanceKanzlei[16].typ = GELD_AN_BANK;
    chanceKanzlei[16].geld = 5;
    
    //Kanzlei Karten
    chanceKanzlei[17].typ = WORKSHOP;
    
    chanceKanzlei[18].typ = FREIKARTE;
    
    chanceKanzlei[19].typ = RENOVIEREN;
    chanceKanzlei[19].geld = 40;
    chanceKanzlei[19].geld2 = 115;
    
    chanceKanzlei[20].typ = GELD_AN_BANK;
    chanceKanzlei[20].geld = 100;
    
    chanceKanzlei[21].typ = GELD_AN_BANK;
    chanceKanzlei[21].geld = 50;
    
    chanceKanzlei[22].typ = GELD_AN_BANK;
    chanceKanzlei[22].geld = 50;
    
    chanceKanzlei[23].typ = GELD_VON_MITSPIELER;
    chanceKanzlei[23].geld = 10;
    
    chanceKanzlei[24].typ = GELD_VON_BANK;
    chanceKanzlei[24].geld = 100;
    
    chanceKanzlei[25].typ = GELD_VON_BANK;
    chanceKanzlei[25].geld = 10;
    
    chanceKanzlei[26].typ = GELD_VON_BANK;
    chanceKanzlei[26].geld = 50;
    
    chanceKanzlei[27].typ = GELD_VON_BANK;
    chanceKanzlei[27].geld = 100;
    
    chanceKanzlei[28].typ = GELD_VON_BANK;
    chanceKanzlei[28].geld = 25;
    
    chanceKanzlei[29].typ = GELD_VON_BANK;
    chanceKanzlei[29].geld = 20;
    
    chanceKanzlei[30].typ = GELD_VON_BANK;
    chanceKanzlei[30].geld = 100;
    
    chanceKanzlei[31].typ = GELD_VON_BANK;
    chanceKanzlei[31].geld = 100;
    
    chanceKanzlei[32].typ = TELEPORTIEREN;
    chanceKanzlei[32].zielFeld = 0;
    chanceKanzlei[32].geld = 200;
    
    chanceKanzlei[33].typ = TELEPORTIEREN;
    chanceKanzlei[33].zielFeld = 32;
    
}

const char kartenArray[][200] PROGMEM =
{
    //17x chance Karten
    "Oli mag dich    nicht.          Gehe zum        Workshop",
    "Freikarte",
    "R"UE"cke vor bis   zum N"AE"chsten    Werk",
    "Du hast zu vieleAbsenzen        Zahle 50",
    "R"UE"cke vor bis   zur Produktion  Polymechaniker",
    "Ziehe vor bis   zur n"AE"chsten    Haltestelle",
    "Ziehe vor bis   zur n"AE"chsten    Haltestelle",
    "Ziehe vor bis   Grundausbildung Elektroniker",
    "Ziehe vor bis   Mechatronik     Labor",
    "R"UE"cke vor auf   Start",
    "Heute InformiertIhr euch "UE"ber   Studieng"AE"nge    R"UE"cke vor bis   Technikum",
    "Es ist der 25.  Du erh"AE"ltst     deinen Lohn. von70",
    "Es ist der 25.  Du erh"AE"ltst     deinen Lohn. von140",
    "Gehe 3 Felder   zur"UE"ck",
    "Du renovierst   deine  H"AE"user   Zahle f"UE"r jedes Haus 25 und f"UE"r jedes Hotel 100",
    "Du wurdest zum  KlassenvertreterGew"AE"hlt. Zahle  jedem Spieler 50",
    "Du hast eine    Wette gegen     Peter verloren  du schuldest ihmeinen Kaffee    zahle 5",
    
    //17x Kanzlei Karten
    "Du hast bei     einem Kunden    eine schlechte  Bewertung       hinterlassen    gehe in den     Workshop",
    "Freikarte",
    "Du renovierst   deine  H"AE"user   Zahle f"UE"r jedes Haus 40 und f"UE"r jedes Hotel 115",
    "In der          Versetzung ist  dir ein Fr"AE"ser  zerbrochen.     Zahle 100",
    "Du g"OE"nnst dir   etwas beim      Snackautomat    zahle 50",
    "Ihr macht eine     Exkursion       zahle 50",
    "Du hast die     Abschlussreise  organisiert,    jeder Spieler   zahlt dir 10",
    "Elektroniker    des Monats      Du erh"AE"ltst 100",
    "Du hilfst deinemMitsch"UE"ler beim lernen.         Du erh"AE"ltst 10",
    "Du erh"AE"lst 50",
    "Du erh"AE"lst 100",
    "Du erh"AE"lst 25",
    "Du erh"AE"lst 20",
    "Du erh"AE"lst 100",
    "Du erh"AE"lst 100", 
    "Du bist         Weltmeister.    R"UE"cke vor bis   Start ziehe den doppelten       Betrag ein",
    "Du hast gegen   die Handyregel  verstossen.     R"UE"cke vor bis   Produktion      Elektroniker" 
};

/******************************************************************************\
* read_string
*
* Liest eine Zeichenkette aus dem Flash-Speicher und kopiert sie in den RAM.
*
* Parameter:
*   - buf: Zeiger auf den Puffer, in den der String kopiert wird
*   - i: Index des Strings im Flash-Speicher-Array `kartenArray`
*
* Rückgabewert: keiner
\******************************************************************************/
void read_string(char *buf, size_t i) 
{
    // Kopiere direkt aus dem Flash ins RAM
    strcpy_P(buf, kartenArray[i]);
}


/******************************************************************************\
* ereignisFeld
*
* Diese Funktion verarbeitet die Ereigniskarten und Kanzleikarten.
* Sie steuert die Aktionen, die der Spieler je nach gezogener Karte ausführt.
*
* Parameter:
* kanzlei        = 1, wenn eine Kanzleikarte gezogen werden soll, 0 für Ereigniskarte
* spielerAmZug   = Der aktuelle Spieler, der an der Reihe ist
* schritt        = Zeile welche auf dem LCD angezeigt werden soll
* flagWeiter     = Signalisiert, ob der Spieler die Aktion bestätigt hat
* chanceKanzlei  = Array von Kartenstrukturen, die die möglichen Karten enthalten
*
* Rückgabewert: 1, wenn die Aktion erfolgreich abgeschlossen wurde, ansonsten 0 
*
\******************************************************************************/
uint8_t ereignisFeld(uint8_t kanzlei, uint8_t spielerAmZug, uint8_t schritt, uint8_t flagWeiter, Karte chanceKanzlei[])
{
    //Variablen initialisieren
    static char text[200] = {0};
    //static uint8_t zufallsNummer = 0;
    static uint8_t rueckgabewert = 0;
    
    //initialisiert variabeln
    uint16_t hausBetrag = 0;
    uint16_t hotelBetrag = 0;
    uint8_t anzahlFelder = 0;
    uint8_t ausgangsPosition = 0;
    uint8_t neuePosition = 0;
    uint16_t startGeld = 0;
    //Wenn die Variable schritt 0 ist
    if (!schritt)
    {
        //rueckgabewert auf 0 setzen
        rueckgabewert = 0;
        //Zufällige Nummer generieren und in der Variablen zufallsNummer speichern
        zufallsNummer = (rand() % 16);
        //Wenn eine Kanzleikarte gezoggen werden soll
        if (kanzlei)
        {
            //Die Zufallsnummer um ANZAHL_KARTEN erhöhen
            //um zu den Kanzleikarten zu gelangen
            zufallsNummer += ANZAHL_KARTEN;
        }
        //Den Text des zufälligen Ereignisses aus dem Speicher auslesen
        read_string(text, zufallsNummer);  //Liest den Text aus dem Flash speicher
    }
    
    //Wenn flagWeiter gesetzt wurde und der Spieler somit bestätigt hat.
    if (flagWeiter) //wenn der Spieler bestätigt hat.
    {
        //Verarbeitung des Kartentyps
        switch(chanceKanzlei[zufallsNummer].typ)
        {
            case WORKSHOP:
            //Den Spieler in den Workshop setzen
            spielerInfo[spielerAmZug].position = FELDNUMER_WORKSHOP; //setzt die Position des spielers auf gefängnis
            //Den Spieler als häftling markieren
            spielerInfo[spielerAmZug].gefaengnis = 1;
            //Die runden im Workshop auf 0 zurücksetzen
            spielerInfo[spielerAmZug].rundenImGefaengnis = 0;
            //Neue Position an den LEDs ausgeben
            setzeSpielerPosition(FELDNUMER_WORKSHOP, spielerAmZug);
            break;
            case FREIKARTE:
            //Dem Spieler eine Freikarte geben
            spielerInfo[spielerAmZug].freikarte = 1; //gibt dem Spieler eine Freikarte
            break;
            case RENOVIEREN:
            //berechet den Preis für die Häuser und die hotels
            //Anzahl Häuser die der Spieler Besitzt auslesen und mit dem Betrag pro Haus
            //multiplizieren
            hausBetrag = spielerInfo[spielerAmZug].haeuser * chanceKanzlei[zufallsNummer].geld;
            //Anzahl Hotels die der Spieler Besitzt auslesen und mit dem Betrag pro Hotel
            //multiplizieren
            hotelBetrag = spielerInfo[spielerAmZug].hotels * chanceKanzlei[zufallsNummer].geld2;
            //Die Summe beider Beträge vom Kontostand abziehen
            geldUeberweisen(spielerAmZug,0,(hausBetrag + hotelBetrag));
            break;
            case GELD_AN_BANK:
            //Betrag an die Bank überweisen
            geldUeberweisen(spielerAmZug,0,chanceKanzlei[zufallsNummer].geld);
            break;
            case GELD_VON_BANK:
            //Betrag an den Spieler überweisen
            geldUeberweisen(0,spielerAmZug,chanceKanzlei[zufallsNummer].geld);
            break;
            case GELD_AN_MITSPIELER:
            //Erhöhe i um 1, solange i <= anzahlSpieler ist. Starte mit i = 1
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //Wenn "i" nicht = spielerAmZug ist
                if (!(i == spielerAmZug))
                {
                    //Betrag dem Spieler "i" überweisen
                    geldUeberweisen(spielerAmZug,i,chanceKanzlei[zufallsNummer].geld);
                }
                
            }
            break;
            case GELD_VON_MITSPIELER:
            //Erhöhe i um 1, solange i <= anzahlSpieler ist. Starte mit i = 1
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //Wenn "i" nicht = spielerAmZug ist
                if (!(i == spielerAmZug))
                {
                    //Spieler "i" überweist dem Spieler am Zug den Betrag
                    geldUeberweisen(i,spielerAmZug,chanceKanzlei[zufallsNummer].geld);
                }
            }
            break;
            case BEWEGEN:
            //Wen vorgegeben ist wie viele Felder gefahren werden
            if (chanceKanzlei[zufallsNummer].bewegung)
            {
                //Aktuelle position in der Variablen ausgangsPosition speichern
                ausgangsPosition = spielerInfo[spielerAmZug].position;
                //ausgangsPosition + Anzahl Felder die gefahren werden
                //in neuePosition speichern
                neuePosition = ausgangsPosition + chanceKanzlei[zufallsNummer].bewegung;
                setzeSpielerPosition(neuePosition, spielerAmZug);
            }
            else
            {
                ausgangsPosition = spielerInfo[spielerAmZug].position;
                for (uint8_t i = ausgangsPosition; !(chanceKanzlei[zufallsNummer].zielFeldTyp == spielfeld[i % 40].typ); i = i + 1)
                {
                    //Die Variable anzahlFelder auf "i" setzen
                    anzahlFelder = i;
                    //neuePosition erhöhen
                    neuePosition = (i % 40) + 1;
                    //Den Spieler auf die neue Position setzen
                    setzeSpielerPosition(neuePosition, spielerAmZug);
                    //Wenn der Spieler sich auf dem Feld "Los" befindet
                    if (neuePosition == 0)
                    {
                        //Dem Spieler das Rundengeld bezahlen
                        geldUeberweisen(0,spielerAmZug,200);
                    }
                    //Programm für 100ms blokieren, damit der Spieler sich animier bewegt
                    _delay_ms(100);
                }
            }
            //setzt die neue Position
            //Die neue Position des Spielers speichern
            spielerInfo[spielerAmZug].position = neuePosition;
            //setzt den spieler auf das richtige Feld
            //Den Spieler auf die neue Position setzen
            setzeSpielerPosition(spielerInfo[spielerAmZug].position,spielerAmZug);
            break;
            case TELEPORTIEREN:
            //wenn die aktuelle spition des Spielers + wüfelsumme grösser gleich 40 is
            // erhält der spieler 200 CHF
            //Wenn der Spieler über "Los" kommen wird
            if (spielerInfo[spielerAmZug].position > chanceKanzlei[zufallsNummer].zielFeld)
            {
                //berechnet den betrag, den man auf start erhält
                //Den Betrag berechnen, welcher auf "Los" überwiesen wird
                startGeld = 200 + chanceKanzlei[zufallsNummer].geld;
                //animiert die fortbewegung des spielers bis feld Los
                //Erhöhe i um 1, bis der spieler auf dem Feld "Los" ist. Starte mit i = ausgangsPosition
                for (uint8_t i = spielerInfo[spielerAmZug].position; i <= 40; i = i + 1)
                {
                    //Setze den Spieler auf die neue Position
                    setzeSpielerPosition(i % 40,spielerAmZug);
                    //Programm für 100ms blokieren, damit der Spieler sich animier bewegt
                    _delay_ms(100); //delay dient zu animationszwecken
                }
                //Dem Spieler den berechneten betrag bezahlen
                setzeSpielerPosition(0,spielerAmZug);
                spielerInfo[spielerAmZug].position = 0;
                geldUeberweisen(0,spielerAmZug,startGeld);
                
            }
            //Erhöhe i um 1, bis der spieler auf dem Zielfeld ist. Starte mit i = ausgangsposition
            for (uint8_t i = spielerInfo[spielerAmZug].position; i < chanceKanzlei[zufallsNummer].zielFeld; i = i + 1)
            {
                setzeSpielerPosition(i % 40,spielerAmZug);
                _delay_ms(100); //delay dient zu animationszwecken
            }
            //Die neue Position des Spielers speichern
            spielerInfo[spielerAmZug].position = chanceKanzlei[zufallsNummer].zielFeld;
            //setzt den spieler auf das richtige Feld
            setzeSpielerPosition(spielerInfo[spielerAmZug].position,spielerAmZug);
            break;
        }
        
        //Den Wert 1 zurückgeben um zu Signalisieren, dass das Ereignis erfolgreich war
        return 1;
    }
    
    //Wenn die Variable rueckgabewert = 0 ist
    if (!rueckgabewert)
    {
        //Text auf dem LCD anzeigen und anzeigen mit welcher Taste man scrollt und mit welcher man bestätigt
        rueckgabewert = lcdLauftext(text,schritt); //schreibt den Text auf das LCD
        writeText(0,0,"X OK     Weiter"PFEIL_R);
    }
    else
    {
        //Text auf dem LCD anzeigen und mit welcher Taste man bestätigt
        writeText(0,0,"X OK            ");
    }
    //Den Wert 0 zurückgeben
    return 0;
    
}

/******************************************************************************\
* ueberweisungsSchritt
*
* Bestimmt die schrittgrösse in denen ein Betrag überwiesen wird
* 
*
* Parameter:
* betrag:       Der Parameter Betrag ist der Betrag, 
                welcher überwiesen werden soll
*
* Rückgabewert: Gibt den grössten geeigneten Teiler zurück
*
\******************************************************************************/
uint8_t ueberweisungsSchritt(uint16_t betrag)
{
    //Die Maximale Schrittgrösse ist schritt -1
    uint8_t schritt = MAX_SCHRITTGROESSE; //Die Variable schritt wird auf 10 initialisiert
    uint8_t rest = 1;//Die Variable rest wird auf 1 initialisiert
    //Solange rest grösser als 0 ist
    while (rest)
    {
        //schritt um 1 verkleinern
        schritt -= 1;
        rest = betrag % schritt;
    }
    return schritt;
}


/******************************************************************************\
* geldUeberweisen
*
* Überweist Geld zwischen zwei Spielern oder Swischen der BAnk und einem Spieler
    Wenn ein Spieler nicht genug Geld hat, wird automatisch das
    geldBeschaffen verfahren eingeleitet
* 
*
* Parameter:
* zahler:       Der Parameter zahler bestimmt von wem das Geld abgezogen wird
                0 = Geld kommt von der Bank
                1 - 4 = Geld kommt von einem Spieler 1 - 4
                
* empfaenger:   Der Parameter empfänger bestimmt wer das Geld erhält
                0 = Geld geht an die Bank
                1 - 4 = Geld geht an einen Spieler 1 - 4
                
* betrag:       Der Parameter Betrag bestimmt die Summe, welche überwiesen wird
                Wert muss im 16 Bit bereich liegen
*
* Rückgabewert: 1: Zahlung erfolgreich / abgeschlossen
*
\******************************************************************************/
uint8_t geldUeberweisen(uint8_t zahler, uint8_t empfaenger, uint16_t betrag)
{
    uint16_t restBetrag = 0; //Variabel restBetrag auf 0 initialisieren
    uint8_t schritt = 0; //Variabel schritt auf 0 initialisieren
    schritt = ueberweisungsSchritt(betrag); //Schrittgrösse bestimmen, in der überwiesen werden soll
    if (empfaenger && zahler) //Wenn das Geld an einen anderen Spieler übewiesen werden soll
    {
        //Wenn der zahlende Spieler genug Geld hat
        if (spielerInfo[zahler].geld >= betrag)
        {
            //erhöhe i um Schrittgrösse, solange i kleiner als der zu bezahlende Betrag ist . Starte mit i = 0
            for (uint16_t i = 0; i < betrag; i = i + schritt)
            {
                //Kontostand des zahlenden Spielers um Schrittgrösse verkleinern
                spielerInfo[zahler].geld -= schritt;
                //Kontostand des empfangenden Spielers um Schrittgrösse vergrössern
                spielerInfo[empfaenger].geld += schritt;
                //Kontostand aktualisieren
                updateKontostand(anzahlSpieler,spielerInfo,0);
                //Programm für 10ms blockieren, damit die Überweisung als Animation wahrgenommen wird
                _delay_ms(10);
            }
            return 1; //Zahlung erfolgreich rückgeben
        }
        else //wenn es sich der Spieler nicht leisten kann
        {
            flagGeldBeschaffen = 1;//flagGeldBeschaffen auf 1 setzen
            //Den restlichen Betrag anhand von zu bezahlendem Betrag und Kontostand des Spielers bestimmen
            //und in der Variabel restBetrag speichern
            restBetrag = betrag - spielerInfo[zahler].geld;
            //Gesamter Kontostand des Spielers überweisen
            geldUeberweisen(zahler,empfaenger,spielerInfo[zahler].geld);
            //Den Spieler das restliche Geld beschaffen lassen
            geldBeschaffen(zahler, empfaenger, restBetrag);//restbetrag beschaffen
            return 1; //Zahlung erfolgreich rückgeben
            //geldUeberweisen(zahler,empfaenger,betrag,1);//restlichesGeld überweisen
            //return 2; //zahlung fehlgeschlagen
        }
    }
    else if (!empfaenger) //Wenn das Geld an die Bank überwiesen werden soll
    {
        //Wenn der zahlende Spieler genug Geld hat
        if (spielerInfo[zahler].geld >= betrag)
        {
            //erhöhe i um Schrittgrösse, solange i kleiner als der zu bezahlende Betrag ist . Starte mit i = 0
            for (uint16_t i = 0; i < betrag; i = i + schritt)
            {
                //Kontostand des zahlenden Spielers um Schrittgrösse verkleinern
                spielerInfo[zahler].geld -= schritt;
                //Kontostand aktualisieren
                updateKontostand(anzahlSpieler,spielerInfo,0);
                //Programm für 10ms blockieren, damit die Überweisung als Animation wahrgenommen wird
                _delay_ms(10);
            }
            return 1; //Zahlung erfolgreich rückgeben
        }
        else //wenn es sich der Spieler nicht leisten kann
        {
            //flagGeldBeschaffen auf 1 setzen
            flagGeldBeschaffen = 1;
            //Den restlichen Betrag anhand von zu bezahlendem Betrag und Kontostand des Spielers bestimmen
            //und in der Variabel restBetrag speichern
            restBetrag = betrag - spielerInfo[zahler].geld;
            //Gesamter Kontostand des Spielers überweisen
            geldUeberweisen(zahler,empfaenger,spielerInfo[zahler].geld);
            //Den Spieler das restliche Geld beschaffen lassen
            geldBeschaffen(zahler, empfaenger, restBetrag);
            return 1;//Zahlung erfolgreich rückgeben
            //return 2; //zahlung fehlgeschlagen
        }
    }
    else if (!zahler)//Wenn die Bank Geld an einen Spieler überweist
    {
        //erhöhe i um Schrittgrösse, solange i kleiner als der zu bezahlende Betrag ist . Starte mit i = 0
        for (uint16_t i = 0; i < betrag; i = i + schritt)
        {
            //Kontostand des empfangenden Spielers um Schrittgrösse vergrössern
            spielerInfo[empfaenger].geld += schritt;
            //Kontostand aktualisieren
            updateKontostand(anzahlSpieler,spielerInfo,0);
            //Programm für 10ms blockieren, damit die Überweisung als Animation wahrgenommen wird
            _delay_ms(10);
        }
        return 1; //zahlung erfolgreich
    }
    //return 0;
}

/******************************************************************************\
* initialisiereHandelInventar
*
* Initialisiert das Handelinventar
* 
*
* Parameter:
* handel:   Das array, welches für das handeln verwendet wird
*
* Rückgabewert: Keine Rückgabe
*
\******************************************************************************/
void initialisiereHandelInventar(handelInventar handel[])
{
    handel[0].spielerNr = 0;
    for (uint8_t i = 0; i < 28; i = i + 1)//setzt alle felder auf 0
    {
        handel[0].feldNummern[i] = 0;
    }
    handel[0].barGeld = 0;
    handel[0].freikarte = 0;
    
    handel[1].spielerNr = 0;
    for (uint8_t i = 0; i < 28; i = i + 1)//setzt alle felder auf 0
    {
        handel[1].feldNummern[i] = 0;
    }
    handel[1].barGeld = 0;
    handel[1].freikarte = 0;
}


/******************************************************************************\
* geldBeschaffen
*
* Der SPieler muss Häuser verkaufen oder Felder belasten 
  um seine Schulden zu begleichen
* 
*
* Parameter:
* zahler:           1 - 4 Die Nummer des Spielers, der sich verschuldet hat
* empfänger:        0 - 4 Die Nummer des Spielers bei dem man verschuldet ist
* mindestbetrag:    Der kleinste Betrag, der aufgetrieben werden muss
                    um schuldfrei zu sein
* Rückgabewert: Keine Rückgabe
*
\******************************************************************************/
void geldBeschaffen(uint8_t zahler, uint8_t empfaenger, uint16_t mindestBetrag)
{
    //diverse Flags, Variablen und Arrays Initialisieren
    uint8_t flagHaeuser = 0;
    uint8_t flagBelastbar = 0;
    uint8_t flagFarbgruppe = 0;
    uint8_t flagFeldBelastet = 0;
    uint8_t feldZaehler = 0;
    uint8_t felderMitHaeuser[40] = {0}; 
    uint8_t felderBelastbar[40] = {0}; 
    uint8_t anzahlFelderMitHaeuser = 0;
    uint8_t anzahlFelderBelastbar = 0;
    uint16_t schuldBetrag = 0;
    uint16_t hypothekBetrag = 0;
    
    uint8_t flagNeuesFeld = 0;
    uint8_t bieter[6] = {0};//array zur speicherung von zurückgetretenen Spielern und höchstbieter
    uint16_t hoechstGebot = 0;
    uint8_t inventar[40] = {0}; //array dient zur speicherung von den Feldern, welche versteigert werden
    uint8_t inventarZaehler = 0;//wird verwendet um die Felder am richtigen Ort im Inventar zu speichern
    uint8_t anzahlVersteigerteFelder = 0;//Zählt wie viele Felder bereits versteigert wurden.
    uint32_t systemZeit = 0;
    uint32_t startZeit = 0;
    
    uint8_t updateLcd = 0;
    uint8_t hypothekAufloesen = 1;
    uint8_t feldNummer = 0;
    uint16_t bezahlBetrag = 0;
    uint8_t zahlungErfolgreich = 0;
    uint8_t flagZuWenigGeld = 0;
    uint8_t flagMussHypBehalten = 0;
    uint8_t gewinner = 0;
    
    //uint8_t farbgruppenErstesFeld[8] = {1,6,11,16,21,26,31,37};
    char lcdBuffer[16];
    //Solange flageldBeschaffen gesetzt ist
    while (flagGeldBeschaffen)
    {
        //Flankenerkennung
        //Tasten einlesen und positive Flanken bestimmen
        tasteAlt = tasteNeu;
        tasteNeu = 0;
        tasteNeu = (PINL << 8) | PINK;
        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        
        switch (pleiteZustand)
        {
            //Zustand in dem geprüft wird ob der Spieler etwas besitzt, was Geld einbringt
            case INVENTAR_PRUEFEN://überprüft ob der Spieler etwas besitzt, das man verkaufen kann
            anzahlFelderMitHaeuser = 0; //anzahlFelderMitHaeuser zurücksetzen
            anzahlFelderBelastbar = 0;  //anzahlFelderBelastbar zurücksetzen
            flagHaeuser = 0;            //flagHaeuser zurücksetzen
            flagBelastbar = 0;         //flagBelastebar zurücksetzen
            flagFarbgruppe = 0;         //flagFarbgruppe zurücksetzen
            //geht alle Felder durch
            //Erhöhe i um 1, solange i kleiner ANZAHL_FELDER ist. Starte mit i = 0
            for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
            {
                //Position "i" in der Liste zurücksetzen
                felderMitHaeuser[i] = 0;    //position "i" in felderMitHaeuser zurücksetzen
                felderBelastbar[i] = 0;     //position "i" in felderBelastbar zurücksetzen
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Häuser~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                //Wenn das Feld an der Position "i" dem Spieler gehört, es Häuser hat und nicht belastet ist
                if ((spielfeld[i].besitzer == zahler) && spielfeld[i].anzahlHaeuser && (!spielfeld[i].feldBelastet))
                {
                    //wenn die bedingung eintrifft, wird das Feld in die Liste aufgenommen
                    felderMitHaeuser[anzahlFelderMitHaeuser] = i;//Speichert die Feldnummer in felderMitHäuser
                    anzahlFelderMitHaeuser += 1; //anzahlFelderMitHaeuser um 1 erhöhen
                    flagHaeuser = 1;//flagHaeuser auf 1 setzen um zu signalisieren, dass es Häuser zu verkaufen gibt
                }
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~BELASTEN~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                //Wenn das Feld an der Position "i" dem Spieler gehört und es nicht belastet ist
                if ((spielfeld[i].besitzer == zahler) && !spielfeld[i].feldBelastet)
                {
                    flagFarbgruppe = 1; //flagFarbgruppe auf 1 setzen
                    //überprüft ob es auf den anderen Farbgruppenfelder Häuser hat
                    for (uint8_t j = 0; j < ANZAHL_FELDER_IN_FARBGRUPPE; j = j + 1)
                    {
                        //Wenn es auf Feld "j" der Farbgruppe ein Haus hat
                        if (spielfeld[spielfeld[i].farbgruppenFelder[j]].anzahlHaeuser)
                        {
                            //flag zurücksetzen | Feld kann erst belastet werden,
                            //wenn alle Häuser der Farbgruppe verkauft sind
                            flagFarbgruppe = 0;//flagFarbgruppe auf 0 setzen
                        }
                    }
                    //prüfe ob flagFarbgruppe noch gesetzt ist
                    //Wenn flagFarbgruppe noch gesetzt ist
                    if (flagFarbgruppe)
                    {
                        //speichert die aktuelle Feldnummer
                        felderBelastbar[anzahlFelderBelastbar] = i; //Feld "i" in felderBelastbar speichern
                        anzahlFelderBelastbar += 1; //anzahlFelderBelastbar um 1 erhöhen
                        //flagBelastbar auf 1 Setzen um zu signalisieren, dass Felder belastet werden können
                        flagBelastbar = 1; 
                    }                    
                }
            }
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~nächsten Zustand bestimmen~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //wenn flagHaeuser gesetzt ist d.h. der spieler besitzt häuser
            //Wenn flagHaueser gesetzt ist
            if (flagHaeuser)
            {
                //LCD ausgabe spieler am Zug und Navigation
                //Spieler am Zug am LCD ausgeben
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",zahler);     //spielerAmZug in den lcdBuffer laden
                writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
                writeText(1,0,"Haus verkaufen? ");  //"Haus verkaufen?" am LCD ausgeben
                writeText(2,0,"X Ja      Nein Y");  //Tastenbelegung am LCD ausgeben
                pleiteZustand = HAEUSER;
            }//wenn flagBelastebar gesetzt ist d.h. der spieler hat belastbare Felder
            //Wenn flagBelastbar gesetzt ist
            else if (flagBelastbar)
            {
                //LCD ausgabe spieler am Zug und Navigation
                //Spieler am Zug am LCD ausgeben
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",zahler);     //spielerAmZug in den lcdBuffer laden
                writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
                writeText(1,0," Feld Belasten? ");  //"Feld Belasten?" am LCD ausgeben
                writeText(2,0,"X Ja      Nein Y");  //Tastenbelegung am LCD ausgeben
                pleiteZustand = BELASTEN;
            }//wenn weder flagHaeuser noch flagBelastebar gesetzt ist
            else
            {
                //der Spieler kann nichts mehr verkaufen oder belasten!
                //der Spieler scheidet aus dem Spiel aus
                pleiteZustand = PLEITE;
            }
            break;
            //Zustand in dem der Spieler Häuser verkaufen kann
            case HAEUSER:
            //wenn Taste X betätigt wurde --> Haus verkaufen
            //Wenn der Spieler seine Taste X betätigt hat
            if (positiveFlanke & xTasten[zahler - 1])
            {
                //lässt den Spieler ein Haus verkaufen
                //der Spieler will ein Haus verkaufen
                hausKaufenVerkaufen(zahler);
                //Zum pleiteZustand GENUG_GELD wechseln
                pleiteZustand = GENUG_GELD;//zustandswechsel
            }//wenn Taste Y betätigt wurde --> Haus nicht verkaufen
            //Wenn der Spieler seine Taste Y betätigt hat
            else if (positiveFlanke & yTasten[zahler - 1])
            {
                //prüft ob der Spieler Felder Belasten kann
                //Wenn der Spieler Felder belasten kann
                if (flagBelastbar)
                {
                    //Spieler am Zug am LCD ausgeben
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);     //spielerAmZug in den lcdBuffer laden   
                    writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
                    writeText(1,0," Feld Belasten? ");  //"Feld Belasten?" am LCD ausgeben
                    writeText(2,0,"X Ja      Nein Y");  //Tastenbelegung am LCD ausgeben
                    pleiteZustand = BELASTEN;           //Zum pleiteZustand BELASTEN wechseln
                }//Wenn der Spieler kein Feld belasten kann
                else
                {
                    //Spieler am Zug am LCD ausgeben
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);         //spielerAmZug in den lcdBuffer laden   
                    writeText(0,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                    //"Du musst Häuser verkaufen!" am LCD ausgeben
                    writeText(1,0," Du must H"AE"user ");
                    writeText(2,0,"   verkaufen!   ");
                    //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                    _delay_ms(3000);
                    //Spieler am Zug am LCD ausgeben
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);         //spielerAmZug in den lcdBuffer laden
                    writeText(0,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                    writeText(1,0,"Haus verkaufen? ");      //"Haus Verkaufen?" am LCD ausgeben
                    writeText(2,0,"X Ja      Nein Y");      //Tastenbelegung am LCD ausgeben
                }
            }
            break;
            //Zustand in dem der Spieler Felder belasten kann
            case BELASTEN:
            //wenn Taste X betätigt wurde --> Feld Belasten
            //Wenn der Spieler seine Taste X betätigt hat
            if (positiveFlanke & xTasten[zahler - 1])
            {
                //Spieler am Zug am LCD ausgeben
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",zahler);                 //spielerAmZug in den lcdBuffer laden
                writeText(0,11,lcdBuffer);                      //lcdBuffer am LCD ausgeben
                //Tastenbelegung am LCD ausgeben
                writeText(1,0,PFEIL_O"Hyp. | weiter "PFEIL_R);  
                writeText(2,0,"                ");
                //Den Namen des ersten belastbaren Feldes auf dem LCD ausgeben
                writeText(2,0,spielfeld[felderBelastbar[0]].name);//Name von erstem Feld ausgeben
                //flagFeldBelastet auf 0 setzen um zu markieren, dass noch kein Feld belastet wurde
                flagFeldBelastet = 0;
                //feldZaehler auf 0 zurücksetzen
                feldZaehler = 0;
                //Wiederhole solange flagFeldBelastet 0 ist und somit noch kein Feld belastet wurde
                while (!flagFeldBelastet)
                {
                    //Flankenerkennung
                    //Tasten einlesen und positive Flanken bestimmen
                    tasteAlt = tasteNeu;
                    tasteNeu = 0;
                    tasteNeu = (PINL << 8) | PINK;
                    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                    
                    //Wenn Taste R betätigt wurde und somit das nächste Feld angezeigt werden soll
                    if (positiveFlanke & TASTE_R)
                    {
                        //prüft ob es noch belastbare Felder in der liste hat
                        //und ob der zähler noch erhöt werden darf
                        //Wenn der Spieler noch mehr belastbare Felder hat
                        if (felderBelastbar[feldZaehler + 1] && feldZaehler < ANZAHL_FELDER - 1)
                        {
                            feldZaehler += 1; //feldZaehler um 1 erhöhen
                            writeText(2,0,"                ");
                            //Name vom nächsten Feld am LCD ausgeben
                            writeText(2,0,spielfeld[felderBelastbar[feldZaehler]].name);
                        }
                        else //wenn das aktuelle Feld das letzte in der liste ist
                        {
                            feldZaehler = 0;
                            writeText(2,0,"                ");
                            //Name vom ersten Feld am LCD ausgeben
                            writeText(2,0,spielfeld[felderBelastbar[feldZaehler]].name);
                        }
                    }
                    //Wenn Taste O betätigt wurde und das Feld somit Belastet werden soll
                    else if (positiveFlanke & TASTE_O)
                    {
                        //Das momentane Feld als belastet markieren
                        spielfeld[felderBelastbar[feldZaehler]].feldBelastet = 1;
                        //Den Wert des Feldes berechnen, indem der Kaufpreis durch 2 dividiert wird
                        hypothekBetrag = spielfeld[felderBelastbar[feldZaehler]].preis / 2;
                        //Dem Spieler den Wert des Feldes überweisen
                        geldUeberweisen(0,zahler,hypothekBetrag);
                        
                        if (spielfeld[felderBelastbar[feldZaehler]].typ == STRASSE)//wenn es eine Strase ist
                        {
                            //Das Feld als belastet markieren, in dem alle 5 Haus LEDs eingeschaltet werden
                            setHaus(spielfeld[felderBelastbar[feldZaehler]].hausnummer,6);
                        }
                        else //wenn es keine strasse ist
                        {
                            //Das Feld als belastet markieren, in dem die Feld RGB LED auf weiss gesetzt wird
                            setPropertyRgb(spielfeld[felderBelastbar[feldZaehler]].rgbNummer,5);
                        }
                        //flagFeldBelastet auf 1 setzen um dem programm zu Signalisieren, dass ein Feld belastet wurde
                        flagFeldBelastet = 1;
                    }
                }
                //Zum pleiteZustand GENUG_GELD wechseln
                pleiteZustand = GENUG_GELD; // Zustandswechsel
            }//wenn Taste Y betätigt wurde --> Feld nicht belasten
            else if (positiveFlanke & yTasten[zahler - 1])
            {
                //Wenn der Spieler Häuser verkaufen kann
                if (flagHaeuser)
                {
                    //Spieler am Zug am LCD ausgeben
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);     //spielerAmZug in den lcdBuffer laden
                    writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
                    //"Haus verkaufen?" am LCD ausgeben
                    writeText(1,0,"Haus verkaufen? "); 
                    //Tastenbelegung am LCD ausgeben 
                    writeText(2,0,"X Ja      Nein Y");
                    //Zum pleiteZustand HAEUSER wechseln
                    pleiteZustand = HAEUSER;
                }//Wenn der Spieler kein Feld belasten kann
                else
                {
                    //Spieler am Zug am LCD ausgeben
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);     //spielerAmZug in den lcdBuffer laden
                    writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
                    //"Du musst Felder belasten!" am LCD ausgeben
                    writeText(1,0," Du musst Felder");  
                    writeText(2,0,"   belasten!    ");
                    //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                    _delay_ms(3000);
                    //Spieler am Zug am LCD ausgeben
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);     //spielerAmZug in den lcdBuffer laden
                    writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
                    //"Feld belasten?" am LCD ausgeben
                    writeText(1,0," Feld belasten? ");
                    //Tastenbelegung am LCD ausgeben
                    writeText(2,0,"X Ja      Nein Y");
                }
            }
            break;
            //Zustand in dem geprüft wird ob der Spieler genug Geld aufgetrieben hat
            //um seine Schulden zu begleichen
            case GENUG_GELD:
            //prüft ob der Spieler das minimum auftreiben konnte
            //Wenn der Spieler bereits den restlichen Betrag auftreiben konnte
            if (spielerInfo[zahler].geld >= mindestBetrag)
            {
                //Den restlichen Betrag an den Spieler, bei dem man sich verschuldet hat, überweisen
                geldUeberweisen(zahler,empfaenger,mindestBetrag);
                //Spieler am Zug am LCD ausgeben
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",zahler);     //spielerAmZug in den lcdBuffer laden
                writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
                writeText(1,0,"                ");
                //"Schuld beglichen" am LCD ausgeben
                writeText(2,0,"Schuld beglichen");
                //Programm für 5 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                _delay_ms(5000);
                //flagGeldBeschaffen auf 0 setzen um dem Programm zu Signalisieren, dass die Schuld beglichen wurde
                flagGeldBeschaffen = 0;
            }
            else
            {
                //Berechnet den Betrag, der noch aufgetrieben werden muss
                schuldBetrag = mindestBetrag - spielerInfo[zahler].geld;
                //Spieler am Zug am LCD ausgeben
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",zahler);         //spielerAmZug in den lcdBuffer laden
                writeText(0,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                
                //Am LCD ausgeben, wie viel Geld noch aufgetrieben werden muss
                writeText(1,0,"  Du schuldest  ");
                writeText(2,0,"                ");
                sprintf(lcdBuffer,"%4u",schuldBetrag);  //schuldBetrag in den lcdBuffer laden
                writeText(2,6,lcdBuffer);               //lcdBuffer am LCD ausgeben
                blaulicht(100,4);                       //Blaulicht aufleuchten lassen
                pleiteZustand = INVENTAR_PRUEFEN;       //Zum pleiteZustand INVENTAR_PRUEFEN wechseln
            }
            break;
            //Zustand in den man gelangt, wenn man pleite ist
            //und seine Schuld nicht begleichen kann
            case PLEITE:
            //Der Spieler hat alle seine Felder belastet und alle Häuser verkauft. 
            //Da der spieler nichts mehr besitzt scheidet er aus dem Spiel aus
            //Wenn der spieler schulden bei einem Mitspieler hat geht sein ganzer Besitz an
            //den Mitspieler. d.h. alle Felder und Freikarten und restliches Geld.
            //Bei Schulden bei der Bank, werden alle Felder Versteigert. Restliches Geld geht an die Bank
            
            //Spieler am Zug am LCD ausgeben
            writeText(0,0,"   Spieler      ");
            sprintf(lcdBuffer,"%u",zahler);     //spielerAmZug in den lcdBuffer laden
            writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
            writeText(1,0,"                ");
            //writeText(1,0," CRAZY  BLINKEN ");
            //"Du bist PLEITE" am LCD ausgeben
            writeText(2,0," Du bist PLEITE ");
            animationSpielerPleite(zahler);
            //Den Spieler am Zug als pleite markieren, um ihn aus dem Spiel auschliessen zu können
            spielerInfo[zahler].pleite = 1;
            
            flagFertigGewuerfelt = 1;
            flagSpielerPleite = 1;
            
            anzahlPleiteSpieler += 1;
            if ((anzahlSpieler - 1) == anzahlPleiteSpieler)
            {
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    if (!(spielerInfo[i].pleite))
                    {
                        gewinner = i;
                    }
                }
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",gewinner);
                writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
                writeText(1,0,"  HAT GEWONNEN  ");
                writeText(2,0,"----------------");

                pleiteZustand = SPIEL_BEENDET;
                break;
            }
            //Die Position des Spielers auf ein unsichtbares Feld setzen
            spielerInfo[zahler].position = UNSICHTBARES_FELD;
            //setGeld(0,zahler,0);//Siebensegment ausschalten
            //Die Spielfigur des Spielers auf das unsichtbare Feld stellen
            setzeSpielerPosition(UNSICHTBARES_FELD,zahler);//entfernt die Spielfigur vom Spielfeld
            //Programm für 5 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
            _delay_ms(500);
            
            
            //Sucht alle Felder nach Feldern ab, die dem Spieler gehörten
            //Erhöhe i um 1, solange i kleiner als ANZAHL_FELDER ist. Starte mit i = 0
            for(uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
            {
                //Setze das Inventar an der Position "i" zurück
                inventar[i] = 0;
                //Wenn das Feld "i" dem ausgeschiedenen Spieler gehört
                if (spielfeld[i].besitzer == zahler)
                {
                    //Das Feld "i" im inventar speichern
                    inventar[inventarZaehler] = i;
                    //Den inventarZaehler um 1 erhöhen
                    inventarZaehler += 1;
                    
                }
            }
            
            
            //bestimmen ob der Spieler Schulden bei der Bank oder bei einem Mitspieler hatte.
            if (!empfaenger) //Wenn der Empfänger die Bank ist
            {
                //die Hypotheken verfallen. Alle felder die versteigert werden sind automatisch nicht mehr belastet
                //Der Spieler hatte Schulden bei der Bank
                //Das gesammte Geld, dass der Spieler noch besitzt an die Bank überweisen
                geldUeberweisen(zahler,0,spielerInfo[zahler].geld);
                //Die anzahl Freikarten, die der Spieler besitzt, auf 0 zurücksetzen
                spielerInfo[zahler].freikarte = 0; 
                //Erhöhe i um 1, solange i kleiner als inventarZaehler ist. Starte mit i = 0
                for (uint8_t i = 0; i < inventarZaehler; i = i + 1)
                {
                    //Die Hypothek, des Feldes "i" aus dem inventar, entfernen
                    spielfeld[inventar[i]].feldBelastet = 0;//Hypothek vom Feld entfernen
                    //Wenn das Feld "i" im Inventar eine Strasse ist
                    if (spielfeld[inventar[i]].typ == STRASSE)
                    {
                        //wenn es eine Strasse ist ist die Hypothek mit 6 Häusern markiert
                        //Alle Haus LEDS auf dem Feld ausschalten
                        setHaus(spielfeld[inventar[i]].hausnummer,0);
                        //Die RGB LED des Feldes auf die Farbe der Bank setzen
                        setPropertyRgb(spielfeld[inventar[i]].rgbNummer,RGB_BANK);
                    }
                    else
                    {
                        //Die RGB LED des Feldes auf die Farbe der Bank setzen
                        setPropertyRgb(spielfeld[inventar[i]].rgbNummer,RGB_BANK);
                    }
                }
                //flagNeuesFeld auf 1 setzen
                flagNeuesFeld = 1;
                
                //alle Felder des Spielers werden in der Grossversteigerung versteigert.
                //Zum pleiteZustand GROSSVERSTEIGERUNG wechseln
                pleiteZustand = GROSSVERSTEIGERUNG;
            }
            else
            {
                //Der Spieler hatte Schulden bei einem Mitspieler. Sein Besitz wird dem Mitspieler übergeben
                //Das gesammte Geld, dass der Spieler noch besitzt an den Spieler, 
                //bei dem man sich verschuldet hat, überweisen
                geldUeberweisen(zahler,empfaenger,spielerInfo[zahler].geld);
                //Zum pleiteZustand FELDER_ABGEBEN wechseln
                pleiteZustand = FELDER_ABGEBEN;
            }
            break;
            case SPIEL_BEENDET:
            writeText(0,0,"   Spieler      ");
            sprintf(lcdBuffer,"%u",gewinner);     //spielerAmZug in den lcdBuffer laden
            writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
            writeText(1,0,"  HAT GEWONNEN  ");
            writeText(2,0,"----------------");
            while (1)
            {
                animationSpielerGewonnen(gewinner);
            }
            break;
            //Zustand in dem alle Felder des Spielers versteigert werden
            case GROSSVERSTEIGERUNG:
            //Wenn flagNeuesFeld gesetzt ist und somit das nächste Feld versteigert werden soll
            if(flagNeuesFeld)
            {
                for (uint8_t i = 0; i < ANZAHL_BIETER_INFORMATIONEN; i = i + 1)
                {
                    //bieter array --> speichert welche Spieler von der Versteigerung zurückgetreten sind = 0 - 3
                    //anzahl zurückgetretene Spieler = 4
                    //höchstbietender Spieler = 5
                    //Setze den Wert an der Position "i" in der Liste "bieter" auf 0
                    bieter[i] = 0; //array zurücksetzen
                }
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    //wenn der spieler Pleite ist, darf er nicht mitbieten
                    //Schalte die Konto Siebensegmente des aktuellen Spielers aus
                    setGeld(0,i,0);//Siebensegmente aller Spieler ausschalten
                    if (spielerInfo[i].pleite)
                    {
                        //"----" auf dem Konto Siebensegment des Spielers anzeigen
                        setGeld(0,i,2);//Spieler spielt nicht mehr mit => ---- anzeigen
                        //Im Speicher markieren, dass der aktuelle Spieler aus der Versteigerung zurückgetreten ist
                        bieter[i - 1] = 1; //Schliesst den Spieler aus der versteigerung aus
                        //Die Anzahl zurückgetretenen Spieler um 1 erhöhen
                        bieter[4] = bieter[4] + 1; //erhöht anzahl zurückgezogene spieler 
                    }
                }
                clear(); //Das LCD leeren
                //"VERSTEIGERUNG" am LCD ausgeben
                writeText(0,0," VERSTEIGERUNG  ");
                //Name des Feldes, dass Versteigert wird, am LCD ausgeben
                writeText(1,0,spielfeld[inventar[anzahlVersteigerteFelder]].name);
                //Tastenbelegung am LCD ausgeben
                writeText(2,0,"bieten X sonst Y");
                //Die Variable hoechstGebot auf 0 zurücksetzen
                hoechstGebot = 0;
                //flagNeuesFeld auf 0 zurücksetzen, um erneutes durchlaufen zu verhindern
                flagNeuesFeld = 0; 
            }
            //prüft alle eingaben der Spieler
            for (uint8_t i = 0; i < anzahlSpieler; i = i + 1)
            {
                //Wenn der Spieler "i" seine Taste X betätigt hat, nicht aus der Versteigerung  zurückgetreten ist,
                //und genug Geld hat um das Gebot zu erhöhen
                if ((positiveFlanke & xTasten[i]) && !bieter[i] 
                && (spielerInfo[i + 1].geld >= hoechstGebot + GROSSVERSTEIGERUNG_SCHRITT_GROESSE))
                {
                    //Die Variable hoechstGebot um die GROSSVERSTEIGERUNG_SCHRITT_GROESSE erhöhen
                    hoechstGebot += GROSSVERSTEIGERUNG_SCHRITT_GROESSE;//höchstgebot um 10 erhöhen
                    //Den Spieler "i" als Höchstbieter speichern
                    bieter[HOECHSTBIETENDER_SPIELER] = i + 1;
                    //Erhöhe j um 1, solange j <= anzahlSpieler ist. Starte mit j = 0
                    for (uint8_t j = 1; j <= anzahlSpieler; j = j + 1)
                    {
                        //Wenn der Spieler noch am bieten ist und nicht der Höchstbieter ist
                        if (!bieter[j - 1] && !(bieter[5] == j))
                        {
                            //wenn der Spieler kein Gebot abgegeben hat aber 
                            //noch mitbietet siebensegmente abschalten
                            setGeld(0,j,0);//Das Konto Siebensegment ausschalten
                        }
                    }
                    //Höchstgebot auf dem Konto LCD des Höchstbieters anzeigen
                    setGeld(hoechstGebot,i + 1,1);//Ausgabe Höchstgebot

                }
                //Wenn der Spieler "i" die Taste Y zum ersten mal in diese Versteigerungsrunde betätigt hat
                //und er nicht der höchstbieter ist
                if ((positiveFlanke & yTasten[i]) && !bieter[i] && !(positiveFlanke & yTasten[bieter[5] - 1]))
                {
                    //Den Spieler "i", als aus der Versteigerung zurückgetreten markieren
                    bieter[i] = 1; //schliesst spieler aus auktion aus
                    //Die Anzahl an aus der Versteigerung zurückgetretenen Spieler um 1 vergrössern
                    bieter[ZURUECKGETRETENE_SPIELER] = bieter[ZURUECKGETRETENE_SPIELER] + 1;
                    //"----" auf dem Konto Siebensegment des Spielers anzeigen
                    setGeld(0,i + 1,2);//---- anzeigen
                }
            }
            
            //Wenn alle Spieler ausser der Höchstbieter aus der Versteigerung zurückgetreten sind
            if (bieter[4] == anzahlSpieler - 1)
            {
                //Am LCD anzeigen, an welchen Spieler das Feld versteigert wurde
                writeText(0,0," versteigert an ");
                writeText(1,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",bieter[HOECHSTBIETENDER_SPIELER]);   //Höchstbieter in den lcdBuffer laden
                writeText(1,11,lcdBuffer);  //lcdBuffer am LCD ausgeben
                //Das Feld an den neuen Besitzer übertragen
                spielfeld[inventar[anzahlVersteigerteFelder]].besitzer = bieter[HOECHSTBIETENDER_SPIELER];
                //Die RGB des Feldes auf die Farbe des neuen Besitzers setzen
                setPropertyRgb(spielfeld[inventar[anzahlVersteigerteFelder]].rgbNummer,bieter[HOECHSTBIETENDER_SPIELER]);
                //DIe Variable anzahlVersteigerteFelder um 1 erhöhen
                anzahlVersteigerteFelder += 1;
                //Wenn es noch weitere Felder zu versteigern gibt
                if (inventar[anzahlVersteigerteFelder])
                {
                    //flagNeuesFeld auf 1 setzen um die Versteigerung des nächsten Feldes zu starten
                    flagNeuesFeld = 1;
                }
                else
                {
                    //wenn es nichts mehr zu versteigern gibt, ist die Versteigerung beendet
                    //Zum pleiteZustand ENDE_VERSTEIGERUNG wechseln
                    pleiteZustand = ENDE_VERSTEIGERUNG;
                }
                //Programm für 1 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                _delay_ms(1000);//1s warten
            }
            break;
            //Zustand in dem die Grossversteigerung abgeschlossen wird
            case ENDE_VERSTEIGERUNG:
            //"VERSTEIGERUNG BEENDET" am LCD Ausgeben
            writeText(0,0," VERSTEIGERUNG  ");
            writeText(1,0,"    BEENDET     ");
            writeText(2,0,"                ");
            //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
            _delay_ms(3000);
            //Den Kontostand aller Spieler an den Konto Siebensegmenten anzeigen
            updateKontostand(anzahlSpieler,spielerInfo,0);
            //flagGeldBeschaffen auf 0 setzen um dem Programm zu signalisieren, dass die Schuld beglichen wurde
            flagGeldBeschaffen = 0;
            //pleiteZustand auf GENUG_GELD zurücksetzen
            pleiteZustand = GENUG_GELD;
            
            
            break;
            //Zustand in dem alle Felder an einen anderen Spieler übertragen werden
            case FELDER_ABGEBEN:
            //Der Spieler hatte Schulden bei einem Mitspieler
            //Empfangender Spieler am LCD anzeigen
            writeText(0,0,"   Spieler      ");
            sprintf(lcdBuffer,"%u",empfaenger); //empfänger in den lcdBuffer laden
            writeText(0,11,lcdBuffer);          //lcdBuffer am LCD ausgeben
            //"Auswahl treffen" am LCD ausgeben
            writeText(1,0,"Auswahl treffen ");
            writeText(2,0,"                ");
            //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
            _delay_ms(3000);
            //geht alle Felder die übertragen werden durch
            //Erhöhe i um 1, solange i kleiner als inventarZaehler ist. Starte mit i = 0
            for (uint8_t i = 0; i < inventarZaehler; i = i + 1)
            {
                //flagFeldBelastet auf 1 setzen
                flagFeldBelastet = 1;
                //flagMussHypBehalten auf 0 setzen
                flagMussHypBehalten = 0;
                //flagZuWenigGeld auf 0 setzen
                flagZuWenigGeld = 0;
                //updateLcd auf 0 setzen
                updateLcd = 0;
                //Solange flagFeldBelastet 1 ist und somit noch keine Entscheidung getroffen wurde
                while(flagFeldBelastet)
                {
                    //Flankenerkennung
                    //Tasten einlesen und positive Flanken bestimmen
                    tasteAlt = tasteNeu;
                    tasteNeu = 0;
                    tasteNeu = (PINL << 8) | PINK;
                    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                    //Wenn das LCD noch nicht aktualisiert wurde
                    if (!updateLcd)
                    {
                        //Das Aktuelle Feld aus dem Inventar auslesen und in der Variable feldNummer speichern
                        feldNummer = inventar[i];
                        writeText(0,0,"                ");
                        //Name des Feldes am LCD ausgeben
                        writeText(0,0,spielfeld[feldNummer].name);
                        //"Hypothek auflösen" und dazugehörige Taste am LCD ausgeben
                        writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                        //"andere Option" und dazugehörige Taste am LCD ausgeben
                        writeText(2,0,"andere Option  "PFEIL_U);
                        //updateLCD auf 1 setzen um erneute Ausgabe zu blockieren
                        updateLcd = 1;
                    }
                    //Wenn der Spieler die Taste O betätigt hat
                    if (positiveFlanke & TASTE_O)
                    {
                        //Wenn die Hypothek aufgelöst werden soll
                        if (hypothekAufloesen)
                        {
                            //Den zu bezahlenden Betrag berechnen
                            bezahlBetrag = spielfeld[feldNummer].preis * 0.55;
                            //bezahlBetrag = (bezahlBetrag / 2) + (bezahlBetrag / 20);
                            //prüft ob der Spieler genug Geld hat um die Hypothek aufzulösen
                            //Wenn der Spieler genug Geld hat um die Hypothek aufzulösen
                            if (spielerInfo[empfaenger].geld >= bezahlBetrag)
                            {
                                //Spieler hat genug Geld
                                //Den zu bezahlenden Betrag und die dazugehörige Taste am LCD ausgeben
                                writeText(1,0,"Zahle          S");
                                sprintf(lcdBuffer,"%u",bezahlBetrag);   //bezahlBetrag in den lcdBuffer laden
                                writeText(1,6,lcdBuffer);               //lcdBuffer am LCD ausgeben
                            }
                            else
                            {
                                //der Spieler hat nicht genug Geld um die Hypothek aufzulösen
                                //"zu Wenig Geld" am LCD ausgeben
                                writeText(1,0," zu wenig Geld  ");
                                //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                                _delay_ms(3000);
                                //Den Betrag berechen, der zu bezahlen ist um die Hypothek zu behalten
                                bezahlBetrag = spielfeld[feldNummer].preis * 0.05;
                                //Wenn der Spieler genug Geld hat um die Hypothek zu behalten
                                if (spielerInfo[empfaenger].geld >= bezahlBetrag)
                                {
                                    //flagMussHypBehalten = 1;
                                    //der SPieler hat genug Geld um die Hypothek zu behalten
                                    //"Du behältst die Hypothek" am LCD ausgeben
                                    writeText(1,0,"du beh"AE"ltst Hyp.");
                                    //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                                    _delay_ms(3000);
                                    //Den zu bezahlenden Betrag und die dazugehörige Taste am LCD ausgeben
                                    writeText(1,0,"Zahle          S");
                                    sprintf(lcdBuffer,"%u",bezahlBetrag);   //bezahlBetrag in den lcdBuffer laden
                                    writeText(1,6,lcdBuffer);               //lcdBuffer am LCD ausgeben
                                }
                                else
                                {
                                    //der Spieler hat nicht genug Geld um die Hypothek zu behalten.
                                    //Am LCD ausgeben, dass das Feld an die Bank geht
                                    writeText(1,0,"geht an die Bank");
                                    writeText(2,0,"                ");
                                    //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                                    _delay_ms(3000);
                                    //flagZuWenigGeld auf 1 setzen, um dem Programm zu Signalisieren,
                                    //dass das Feld an die Bank geht
                                    flagZuWenigGeld = 1;
                                }
                            }
                            
                        }
                        else//wenn die Hypothek nicht aufgelöst werden soll
                        {
                            //Den Betrag berechen, der zu bezahlen ist um die Hypothek zu behalten
                            bezahlBetrag = spielfeld[feldNummer].preis * 0.05;
                            ///Wenn der Spieler nicht genug Geld hat um die Hypothek zu behalten
                            if (bezahlBetrag > spielerInfo[empfaenger].geld)
                            {
                                //"zu Wenig Geld" am LCD ausgeben
                                writeText(1,0," zu wenig Geld  ");
                                writeText(2,0,"                ");
                                //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                                _delay_ms(3000);
                                //Am LCD ausgeben, dass das Feld an die Bank geht
                                writeText(1,0,"geht an die Bank");
                                writeText(2,0,"                ");
                                //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                                _delay_ms(3000);
                                //flagZuWenigGeld auf 1 setzen, um dem Programm zu Signalisieren, 
                                //dass das Feld an die Bank geht
                                flagZuWenigGeld = 1;
                            }
                            else
                            {
                                //Den zu bezahlenden Betrag und die dazugehörige Taste am LCD ausgeben
                                writeText(1,0,"Zahle          S");
                                sprintf(lcdBuffer,"%u",bezahlBetrag);   //bezahlBetrag in den lcdBuffer laden
                                writeText(1,6,lcdBuffer);               //lcdBuffer am LCD ausgeben
                            }
                        }
                        //Wenn flagZuWenigGeld nicht gesetzt wurde
                        if (!flagZuWenigGeld)
                        {
                            //Warte bis der Spieler mit Taste S bestätigt hat,
                            //oder mit Taste U zurück zur Auswahl gegangen ist
                            while (!(positiveFlanke & TASTE_S) && !(positiveFlanke & TASTE_U))
                            {
                                //Flankenerkennung
                                //Tasten einlesen und positive Flanken bestimmen
                                tasteAlt = tasteNeu;
                                tasteNeu = 0;
                                tasteNeu = (PINL << 8) | PINK;
                                positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                            }
                            //Wenn der Spieler die Taste S betätigt hat
                            if (positiveFlanke & TASTE_S)
                            {
                                //Den zu bezahlenden Betrag bezahlen
                                zahlungErfolgreich = geldUeberweisen(empfaenger,0,bezahlBetrag);
                            }
                            else
                            {
                                //Zur nächsten Option wechseln
                                hypothekAufloesen = (hypothekAufloesen + 1) % 2;
                                //Wenn die neue Option Hypothek auflösen ist
                                if (hypothekAufloesen)
                                {
                                    //"Hypothek auflösen" und die dazugehörige Taste am LCD ausgeben
                                    writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                                }
                                else
                                {
                                    //"Hypothek behalten" und die dazugehörige Taste am LCD ausgeben
                                    writeText(1,0,"Hyp. Behalten  "PFEIL_O);
                                }
                            }
                            //Wenn die Zahlung erfolgreich war
                            if (zahlungErfolgreich == 1)
                            {
                                //Wenn die Hypothek aufgelöst werden soll
                                if (hypothekAufloesen)
                                {
                                    //Feld als nicht mehr belastet speichern
                                    spielfeld[feldNummer].feldBelastet = 0;
                                    //Das Feld an den neuen Besitzer übertragen
                                    spielfeld[feldNummer].besitzer = empfaenger;
                                    //Wenn das Feld eine Strasse ist
                                    if (spielfeld[feldNummer].typ == STRASSE)
                                    {
                                        //Alle Haus LEDs ausschalten
                                        setHaus(spielfeld[feldNummer].hausnummer,0);
                                    }
                                    //Feld RGB LED auf die Farbe des neuen Besitzers setzen
                                    setPropertyRgb(spielfeld[feldNummer].rgbNummer,empfaenger);
                                    
                                }
                                else
                                {
                                    //Das Feld an den neuen Besitzer übertragen
                                    spielfeld[feldNummer].besitzer = empfaenger;
                                    //Wenn das Feld eine Strasse ist
                                    if (spielfeld[feldNummer].typ == STRASSE)
                                    {
                                        //Feld RGB LED auf die Farbe des neuen Besitzers setzen
                                        setPropertyRgb(spielfeld[feldNummer].rgbNummer,empfaenger);
                                    }
                                    else
                                    {
                                        setPropertyRgb(spielfeld[feldNummer].rgbNummer,5);
                                    }
                                }
                                //flagFeldBelastet auf 0 setzen um dem Programm zu signalisieren,
                                //dass eine Entscheidung getroffen wurde
                                flagFeldBelastet = 0;
                                //zahlungErfolgreich auf 0 zurücksetzen
                                zahlungErfolgreich = 0;
                            }
                        }
                        else
                        {
                            //Der Spieler hat zuwenig Geld -> Feld geht an die Bank
                            //Das Feld an die Bank übertragen
                            spielfeld[feldNummer].besitzer = 0;
                            //Feld als nicht mehr belastet markieren
                            spielfeld[feldNummer].feldBelastet = 0;
                            //RGB LED des Feldes ausschalten
                            setPropertyRgb(spielfeld[feldNummer].rgbNummer,0);
                            //Wenn das Feld eine Strasse ist
                            if (spielfeld[feldNummer].typ == STRASSE)
                            {
                                //Alle Haus LEDs ausschalten
                                setHaus(spielfeld[feldNummer].hausnummer,0);
                            }
                            flagFeldBelastet = 0; //flag auf 0 setzen um mit nächtem Feld weiterzufahren
                        }
                        
                    }
                    //Wenn Taste U betätigt wurde
                    else if (positiveFlanke & TASTE_U)
                    {
                        //Zur nächsten Option wechseln
                        hypothekAufloesen = (hypothekAufloesen + 1) % 2;
                        //Wenn die neue Option Hypothek auflösen ist
                        if (hypothekAufloesen)
                        {
                            //"Hypothek auflösen" und die dazugehörige Taste am LCD ausgeben
                            writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                        }
                        else
                        {
                            //"Hypothek behalten" und die dazugehörige Taste am LCD ausgeben
                            writeText(1,0,"Hyp. Behalten  "PFEIL_O);
                        }
                    }
                }
            }
            //Wenn der Spieler Freikarten hatte
            if (spielerInfo[zahler].freikarte)
            {
                //Die Freikarten an den neuen Besitzer übertragen
                spielerInfo[empfaenger].freikarte += spielerInfo[zahler].freikarte;
                //Die Freikarten aus dem Inventar des alten besitzers löschen
                spielerInfo[zahler].freikarte = 0;
            }
            //Zum pleiteZustand ENDE_FELDER_ABGEBEN wechseln
            pleiteZustand = ENDE_FELDER_ABGEBEN;
            break;
            //Zustand in dem das Abgeben der Felder abgeschlossen wird
            case ENDE_FELDER_ABGEBEN:
            //"ALLE FELDER ABGEGEBEN" am LCD ausgeben
            writeText(0,0,"  ALLE FELDER   ");
            writeText(1,0,"   ABGEGEBEN    ");
            writeText(2,0,"                ");
            //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
            _delay_ms(3000);
            //flagGeldBeschaffen auf 0 setzen um dem Programm zu signalisieren, dass die Schuld beglichen wurde
            flagGeldBeschaffen = 0; // flag auf 0 setzen um aus der while Schleife rauszukommen
            //pleiteZustand auf GENUG_GELD zurücksetzen
            pleiteZustand = GENUG_GELD;//startzustand festlegen
            break;
            default:
            break;
        }
    }
}

/******************************************************************************\
* hausKaufenVerkaufen
*
* Sorgt dafür das Häuser auf Vollen Farbgruppen gleichmässig gebaut
* und abgebaut werden
*
* Parameter:
* spielerAmZug = Die Spielernummer des Spielers, welcher etwas bauen will (1 - 4)
*
* Rückgabewert: Keine Rückgabe
*
\******************************************************************************/
void hausKaufenVerkaufen(uint8_t spielerAmZug)
{
    uint8_t farbgruppenZaehler = 0;
    uint8_t updateLCD = 0;
    uint8_t minHaeuser = 5;
    uint8_t maxHaeuser = 0;
    uint8_t volleFarbgruppen[8] = {0};
    uint8_t haeuser = 0;
    uint8_t flagFarbgruppeKomplett = 0;
    uint8_t flagBauErfolgreich = 0;
    uint8_t feldNummer = 0;
    uint8_t farbgruppenErstesFeld[8] = {1,6,11,16,21,26,31,37}; //Jeweil das erste Feld einer Strassen Farbgruppe
    uint8_t anzahlHauser[3] = {0};
    uint8_t flagZurueck = 0;
    //Erhöhe i um 1, solange i kleiner als ANZAHL_FARBGRUPPEN ist. Starte mit i = 0
    for (uint8_t i = 0; i < ANZAHL_FARBGRUPPEN; i = i + 1)//Alle Farbgruppen werden als voll markiert
    {
        //Die Liste volleFarbgruppen an Position "i" auf 1 zurücksetzen
        volleFarbgruppen[i] = 1;
    }
    //Erhöhe i um 1, solange i kleiner als ANZAHL_FELDER_IN_FARBGRUPPE ist. Starte mit i = 0
    for (uint8_t i = 0; i < ANZAHL_FELDER_IN_FARBGRUPPE; i = i + 1)
    {
        //Die Liste anzahlHauser an Position "i" auf 1 zurücksetzen
        anzahlHauser[i] = 0;
    }
    for (uint8_t i = 0;  i < ANZAHL_FARBGRUPPEN; i = i + 1)
    {
        //Flankenerkennung
        //Tasten einlesen und positiveFlanken bestimmen
        tasteAlt = tasteNeu;
        tasteNeu = 0;
        tasteNeu = (PINL << 8) | PINK;
        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        //flagFarbgruppeKomplett auf 1 setzen
        flagFarbgruppeKomplett = 1;
        //Wenn das erste Feld der Farbgruppe "i" dem Spieler am Zug gehört
        if (spielfeld[farbgruppenErstesFeld[i]].besitzer == spielerAmZug)
        {
            //Erhöhe j um 1, solange j kleiner als ANZAHL_FELDER_IN_FARBGRUPPE ist. Starte mit j = 0
            for (uint8_t j = 0; j < ANZAHL_FELDER_IN_FARBGRUPPE; j = j + 1)//Prüft ob Farbgruppen tatsächlich voll sind
            {
                //Wenn das zu prüfende Feld der Farbgruppe nicht den selben Besitzer hat,
                //wie das erste Feld der Farbgruppe und das zu prüfende Feld nicht 0 ist.
                //Oder wenn das zu prüfende Feld belastet ist
                
                //Besitzer der Felder vergleichen. Wenn nicht der selbe besitzer, dann darf nicht gebaut werden
                //Sonderfall für Farbgruppen mit 2 Felder, Sobald j = 2 (3. Feld) kommt man nicht ins if rein
                //Prüfen ob das Feld belastet ist. Wenn es belastet ist, dann darf nicht gebaut werden
                if (((!(spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].besitzer == spielerAmZug))
                        && (spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j])) 
                        || spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].feldBelastet)
                {
                    //flagFarbgruppeKomplett auf 0 setzen um zu markieren, dass die Farbgruppe nicht komplett ist
                    flagFarbgruppeKomplett = 0;
                    //Die Liste volleFarbgruppen an Position "i" auf 0 zurücksetzen,
                    //um die Farbgruppe als nicht komplett zu speichern
                    volleFarbgruppen[i] = 0; //Markiert Farbgruppe als unvollständig
                }
            }
            //Wenn flagFarbgruppeKomplett noch gesetzt ist
            //und die Farbgruppe somit komplett ist
            if (flagFarbgruppeKomplett)//Wenn farbgruppe komplett
            {
                //updateLCD auf 0 setzen
                updateLCD = 0;
                //Die Nummer des ersten Feldes der aktuellen Farbgruppe, in feldNummer speichern
                feldNummer = farbgruppenErstesFeld[i];
                //flagZurück wird verwendet, wenn Geld aufgetrieben werden muss.
                //Es wird gesetzt, wenn ein Haus verkauft wurde um die while
                //schleife zu verlassen
                //Solange die Taste C nicht betätigt wurde und flagZurueck nicht gesetzt wurde
                while (!(positiveFlanke & TASTE_C) && !flagZurueck)
                {
                    //Flankenerkennung
                    //Tasten einlesen und positiveFlanken bestimmen
                    tasteAlt = tasteNeu;
                    tasteNeu = 0;
                    tasteNeu = (PINL << 8) | PINK;
                    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                    //Die Variable minHaeuser auf 255 setzen
                    minHaeuser = 255;
                    //Die Variable maxHaeuser auf 0 setzen
                    maxHaeuser = 0;
                    //Erhöhe j um 1, solange j kleiner als ANZAHL_FELDER_IN_FARBGRUPPE ist. Starte mit j = 0
                    for (uint8_t j = 0; j < ANZAHL_FELDER_IN_FARBGRUPPE; j = j + 1)
                    {
                        //Wenn das Feld "j" der Farbgruppe grösser als 0 ist 
                        //wird benötigt für die Spezialfälle der ersten und letzten Farbgruppe
                        if (spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j] > 0)
                        {
                            //Anzahl Häuser auf dem Feld j der Farbgruppe auslesen und in der Variable haeuser speichern
                            haeuser = spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].anzahlHaeuser;
                            //Wenn haeuser kleiner als minHaeuser ist
                            if ((haeuser < minHaeuser))
                            {
                                //Die Variable minHaeuser auf den Wert von haeuser setzen
                                minHaeuser = haeuser;
                            }
                            //Wenn haeuser grösser als maxHaeuser ist
                            if (haeuser > maxHaeuser)
                            {
                                //Die Variable maxHaeuser auf den Wert von haeuser setzen
                                maxHaeuser = haeuser;
                            }
                        }
                    }
                    //Erhöhe j um 1, solange j kleiner als ANZAHL_FELDER_IN_FARBGRUPPE ist. Starte mit j = 0
                    for (uint8_t j = 0; j < ANZAHL_FELDER_IN_FARBGRUPPE; j = j + 1)
                    {
                        //Anzahl Häuser auf dem Feld j der Farbgruppe auslesen und in der Variable haeuser speichern
                        haeuser = spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].anzahlHaeuser;
                        //Wenn der Wert haeuser mit dem Wert maxHaeuser übereinstimmt
                        if (haeuser == maxHaeuser)
                        {
                            //Die Liste anzahlHauser an Position "j" auf 1 setzen
                            anzahlHauser[j] = 1;
                        }
                        //Wenn der Wert haeuser mit dem Wert minHaeuser übereinstimmt
                        else if (haeuser == minHaeuser)
                        {
                            anzahlHauser[j] = 0;
                        }
                        else
                        {
                            //Die Liste anzahlHauser an Position "j" auf 1 setzen
                            anzahlHauser[j] = 1;
                        }
                    }
                    //Wenn das LCD noch nicht aktualisiert wurde     
                    if (!updateLCD)
                    {
                        //LCD leeren
                        clear();//lcd leeren
                        //Name des aktuellen Feldes auf das LCD schreiben
                        writeText(1,0,spielfeld[feldNummer].name);
                        //LCD ausgabe abhängig ob der Spieler schulden hat oder nicht
                        if (flagGeldBeschaffen)
                        {
                            //"Abbauen" und die dazugehörige Taste am LCD anzeigen
                            writeText(0,0,PFEIL_U"Abbauen        ");
                            //"weiter" und die dazugehörige Taste am LCD anzeigen
                            writeText(2,0,"         weiter"PFEIL_R);
                        }
                        else
                        {
                            //"Abbauen", "Bauen" und die dazugehörigen Tasten am LCD anzeigen
                            writeText(0,0,PFEIL_U"Abbauen  Bauen"PFEIL_O);
                            //"zurück", "weiter" und die dazugehörigen Tasten am LCD anzeigen
                            writeText(2,0,"C zur"UE"ck|weiter"PFEIL_R); 
                        }
                        /*writeText(1,0," w"UE"rfeln A / B ");
                        writeText(2,0,"    weiter C    ");*/
                        //gruppeAnzahlHaeuser = spielfeld[farbgruppenErstesFeld[i]].anzahlHaeuser; //holt die Anzahl Häuser
                        
                        //updateLCD auf 1 setzen um erneutes durchlaufen zu blockieren
                        updateLCD = 1;
                    }
                    //Wenn die Taste O betätigt wurde und der Spieler nicht verschuldet ist
                    if ((positiveFlanke & TASTE_O) && !flagGeldBeschaffen)
                    {
                        //Wenn auf allen felder gebaut wurde, flags zurücksetzten
                        //Wenn alle Elemente der Liste anzahlHauser auf 1 gesetzt sind
                        if (anzahlHauser[0] && anzahlHauser[1] && anzahlHauser[2])
                        {
                            //Alle Elemente der Liste anzahlHauser auf 0 zurücksetzen
                            anzahlHauser[0] = 0;
                            anzahlHauser[1] = 0;
                            anzahlHauser[2] = 0;
                        }
                        //Verarbeitung von farbgruppenCounter
                        switch (farbgruppenZaehler)
                        {
                            case 0:
                            //Wenn das Element 0 der Liste anzahlHauser = 0 ist
                            if (!anzahlHauser[0])
                            {
                                //Auf dem aktuellen Feld ein Haus bauen
                                flagBauErfolgreich = bauen(feldNummer, spielerAmZug);
                                //Wenn der Bau erfolgreich war
                                if (flagBauErfolgreich)
                                {
                                    //Element 0 der Liste anzahlHauser auf 1 setzen
                                    anzahlHauser[0] = 1;
                                }
                                        
                            }
                            break;
                            case 1:
                            //Wenn das Element 1 der Liste anzahlHauser = 0 ist
                            if (!anzahlHauser[1])
                            {
                                //Auf dem aktuellen Feld ein Haus bauen
                                flagBauErfolgreich = bauen(feldNummer, spielerAmZug);
                                //Wenn der Bau erfolgreich war
                                if (flagBauErfolgreich)
                                {
                                    //Element 1 der Liste anzahlHauser auf 1 setzen
                                    anzahlHauser[1] = 1;
                                }
                            }
                            break;
                            case 2:
                            //Wenn das Element 2 der Liste anzahlHauser = 0 ist
                            if (!anzahlHauser[2])
                            {
                                //Auf dem aktuellen Feld ein Haus bauen
                                flagBauErfolgreich = bauen(feldNummer, spielerAmZug);
                                //Wenn der Bau erfolgreich war
                                if (flagBauErfolgreich)
                                {
                                    //Element 2 der Liste anzahlHauser auf 1 setzen
                                    anzahlHauser[2] = 1;
                                }
                            }
                            break;
                        }
                                
                    }
                    //Wenn die Taste U betätigt wurde
                    if ((positiveFlanke & TASTE_U))//Haus Verkaufen
                    {
                        //Wenn alle Elemente der Liste anzahlHauser auf 0 gesetzt sind
                        if (!(anzahlHauser[0] || anzahlHauser[1] || anzahlHauser[2]))
                        {
                            //Alle Elemente der Liste anzahlHauser auf 1 zurücksetzen
                            anzahlHauser[0] = 1;
                            anzahlHauser[1] = 1;
                            anzahlHauser[2] = 1;
                        }
                        //Verarbeitung von farbgruppenCounter
                        switch (farbgruppenZaehler)
                        {
                            case 0:
                            //Wenn das Element 0 der Liste anzahlHauser = 1 ist
                            if (anzahlHauser[0])
                            {
                                //Auf dem aktuellen Feld ein Haus abbauen
                                flagBauErfolgreich = abBauen(feldNummer, spielerAmZug);
                                //Wenn der Abbau erfolgreich war
                                if (flagBauErfolgreich)
                                {
                                    //Element 0 der Liste anzahlHauser auf 0 setzen
                                    anzahlHauser[0] = 0;
                                } 
                            }
                            break;
                            case 1:
                            //Wenn das Element 1 der Liste anzahlHauser = 1 ist
                            if (anzahlHauser[1])
                            {
                                //Auf dem aktuellen Feld ein Haus abbauen
                                flagBauErfolgreich = abBauen(feldNummer, spielerAmZug);
                                //Wenn der Abbau erfolgreich war
                                if (flagBauErfolgreich)
                                {
                                    //Element 1 der Liste anzahlHauser auf 0 setzen
                                    anzahlHauser[1] = 0;
                                }
                            }
                            break;
                            case 2:
                            //Wenn das Element 2 der Liste anzahlHauser = 1 ist
                            if (anzahlHauser[2])
                            {
                                //Auf dem aktuellen Feld ein Haus abbauen
                                flagBauErfolgreich = abBauen(feldNummer, spielerAmZug);
                                //Wenn der Abbau erfolgreich war
                                if (flagBauErfolgreich)
                                {
                                    //Element 2 der Liste anzahlHauser auf 0 setzen
                                    anzahlHauser[2] = 0;
                                }
                            }
                            break;
                        }
                                
                    }
                    //Wenn Geld bschafft werden muss, darf nur 1 Haus aufs mal verkauft werden
                    //Wenn flagGeldBeschaffen gesetzt ist und ein Haus abgebaut wurde
                    
                    //Wenn flagGeldBeschaffen gesetzt ist und ein Haus verkauft wurde
                    if (flagGeldBeschaffen && flagBauErfolgreich)
                    {
                        //flagZurueck auf 1 setzen um die Funktion zu verlassen
                        flagZurueck = 1;
                    }
                    //Wenn Taste R betätigt wurde
                    if (positiveFlanke & TASTE_R)//nächstes Feld
                    {
                        //Zum nächsten Feld wechseln, in dem farbgruppenCounter erhöt wird
                        farbgruppenZaehler = (farbgruppenZaehler + 1) % 3;
                        //Die Nummer des neuen Feldes auslesen und in feldNummer speichern
                        feldNummer = spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[farbgruppenZaehler];
                        //Wenn feldNummer 0 ist
                        if (!feldNummer)//Sonderfall bei Farbgruppen mit nur 2 Feldern
                        {
                            //farbgruppenCounter auf 0 zurücksetzen
                            farbgruppenZaehler = 0;
                            //Die Nummer des neuen Feldes auslesen und in feldNummer speichern
                            feldNummer = spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[farbgruppenZaehler];
                            //Das Element 2 der Liste anzahlHauser auf den Gleichen wert des Elementes 1 setzen
                            anzahlHauser[2] = anzahlHauser[1];
                        }
                        //updateLCD auf 0 setzen, damit das nächste Feld am LCD angezeigt werden kann
                        updateLCD = 0;
                    }
                }
                //flagZurueck auf 0 zurücksetzen
                flagZurueck = 0;
                        
            }
                    
        }
    }

}



/******************************************************************************\
* bauen
*
* Prüft ob noch genügend Häuser / Hotels im Spiel sind um ein Weiteres zu bauen
  Plaziert die Häuser auf dem Feld
*
* Parameter:
* feldNummer = Das Feld auf dem gebaut werden soll
* spielerAmZug = Der Spieler der bauen will (1 - 4)
*
* Rückgabewert: 1 = Bau erfolgreich | 0 = Bau fehlgeschlagen
*
\******************************************************************************/
uint8_t bauen(uint8_t feldNummer, uint8_t spielerAmZug)
{
    //Die Variable kaufStatus auf 0 initialisieren
    uint8_t kaufStatus = 0;
    //wenn ein Hotel gebaut wird
    //Wenn die Anzahl Häuser auf dem Feld kleiner als 5 ist zusätzlich wird geprüft ob der SPieler genug Geld hat
    if ((spielfeld[feldNummer].anzahlHaeuser < 5) && spielerInfo[spielerAmZug].geld >= spielfeld[feldNummer].kostenHaus)
    {
        //Wenn auf dem Feld bereits 4 Häuser gebaut wurden und es noch genug Hotels im Spiel hat
        if ((spielfeld[feldNummer].anzahlHaeuser == 4) && hotelsImSpiel < MAX_ANZAHL_HOTELS_IM_SPIEL)
        {
            //Anzahl Häuser um 4 verringern
            spielerInfo[spielerAmZug].haeuser -= 4;
            //Anzahl Hotels um 1 vergrössern
            spielerInfo[spielerAmZug].hotels += 1;
            
            //Die Variable haeuserImSpiel um 4 verkleinern
            haeuserImSpiel -= 4;
            //Die Variable hotelsImSpiel um 1 vergrössern
            hotelsImSpiel += 1;
            //Den Betrag für ein Hotel überweisen
            kaufStatus = geldUeberweisen(spielerAmZug,0,spielfeld[feldNummer].kostenHaus);
            //Die Haus LEDs setzen
            setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser + 1); 
            //Neue Anzahl Häuser speichern
            spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser + 1;
            //Den Wert 1 zurückgeben um zu Signalisieren, dass der Bau erfolgreich war
            return 1;//Erfolgreich
        }
        //wenn es noch Häuser im Spiel hat
        else if (haeuserImSpiel < MAX_ANZAHL_HAEUSER_IM_SPIEL)
        {
            
            //Anzahl Häuser erhöhen
            spielerInfo[spielerAmZug].haeuser += 1;
            
            
            //häuser im spiel erhöhen
            //Die Variable haeuserImSpiel um 1 vergrössern
            haeuserImSpiel += 1;
            //geld überweisen
            //Den Betrag für ein Haus überweisen
            kaufStatus = geldUeberweisen(spielerAmZug,0,spielfeld[feldNummer].kostenHaus);
            //Die Haus LEDs setzen
            setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser + 1);
            //Neue anzahl Häuser speichern
            spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser + 1;
            //Den Wert 1 zurückgeben um zu Signalisieren, dass der Bau erfolgreich war
            return 1;//Erfolgreich
        }
        else
        {
            return 0;//Fehlgeschlagen
        }
    }
    return 0;//Sollte nicht zu problemen führen
}


/******************************************************************************\
* abBauen
*
* Entfernt Häuser und Hotels von den Feldern
*
* Parameter:
* feldNummer = Das Feld auf dem abgebaut werden soll
* spielerAmZug = Der Spieler der abauen will (1 - 4)
*
* Rückgabewert: 1 = Abbau erfolgreich | 0 = Abbau fehlgeschlagen
*
\******************************************************************************/
uint8_t abBauen(uint8_t feldNummer, uint8_t spielerAmZug)
{
    //Die Variable kaufStatus auf 0 initialisieren
    uint8_t kaufStatus = 0;
    //Wenn auf dem Feld ein Hotel steht
    if ((spielfeld[feldNummer].anzahlHaeuser == 5) && (spielfeld[feldNummer].anzahlHaeuser > 0))
    {
        //Anzahl Häuser um 4 vergrössern
        spielerInfo[spielerAmZug].haeuser += 4;
        //Anzahl Hotels um 1 verringern
        spielerInfo[spielerAmZug].hotels -= 1;
        
        //Die Variable haeuserImSpiel um 4 vergrössern
        haeuserImSpiel += 4;
        //Die Variable hotelsImSpiel um 1 verkleinern
        hotelsImSpiel -= 1;
        //Die Haus LEDs setzen
        setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser - 1);
        //Neue Anzahl Häuser speichern
        spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser - 1;
        //Den Wert für ein Hotel an den Spieler überweisen
        kaufStatus = geldUeberweisen(0,spielerAmZug,spielfeld[feldNummer].kostenHaus / 2);
        //Den Wert 1 zurückgeben um zu Signalisieren, dass der Abbau erfolgreich war
        return 1;
    }
    //Wenn auf dem Feld mindestens 1 Haus steht
    else if (spielfeld[feldNummer].anzahlHaeuser > 0)
    {
        //Anzahl Häuser verringern
        spielerInfo[spielerAmZug].haeuser -= 1;
        
        //Die Variable haeuserImSpiel um 1 verkleinern
        haeuserImSpiel -= 1;
        //Die Haus LEDs setzen
        setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser - 1);
        //Neue Anzahl Häuser speichern
        spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser - 1;
        //Den Wert für ein Haus an den Spieler überweisen
        kaufStatus = geldUeberweisen(0,spielerAmZug,spielfeld[feldNummer].kostenHaus / 2);
        //Den Wert 1 zurückgeben um zu Signalisieren, dass der Abbau erfolgreich war
        return 1;
    }
    else
    {
        //Den Wert 0 zurückgeben um zu Signalisieren, dass nichts abgebaut wurde
        return 0;
    }
    
}



void animationSpielerPleite(uint8_t spielernummer)
{
        //for schleife setzt die RGB auf jedem spielfeld
        for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
        {
            if (spielfeld[i].rgbNummer)
            {
                setPropertyRgb(spielfeld[i].rgbNummer,0);
            }
            if (spielfeld[i].hausnummer)
            {
                setHaus(spielfeld[i].hausnummer,0);
            }
            setzeSpielerPositionAnimation(i,spielernummer,1,0);
            setzeSpielerPositionAnimation(i,spielernummer,1,1);
        }
        _delay_ms(100);
        spielfeldBlinken(0);
        for (uint8_t i = ANZAHL_FELDER; i > 0; i = i - 1)
        {
            if (spielfeld[i].rgbNummer)
            {
                //setPropertyRgb(spielfeld[i].rgbNummer,0);
            }
            if (spielfeld[i].hausnummer)
            {
                setHaus(spielfeld[i].hausnummer,0);
            }
            setzeSpielerPositionAnimation(i,spielernummer,0,0);
            setzeSpielerPositionAnimation(i,spielernummer,0,1);
        }
        setzeSpielerPosition(40,4);
        for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
        {
            if (spielfeld[i].besitzer)
            {
                setPropertyRgb(spielfeld[i].rgbNummer,spielfeld[i].besitzer);
                setHaus(spielfeld[i].hausnummer,spielfeld[i].anzahlHaeuser);
            }
            if (spielfeld[i].feldBelastet)
            {
                setHaus(spielfeld[i].hausnummer,6);
            }
        }
        spielerInfo[4].position = 0;
        setzeSpielerPosition(0,spielernummer);
        spielerInfo[4].position = 40;
        setzeSpielerPosition(40,spielernummer);
        for(uint8_t i = 1; i <= 4; i = i + 1)
        {
            setzeSpielerPosition(spielerInfo[i].position,i);
        }
        
}

void animationSpielerGewonnen(uint8_t spielernummer)
{
    
    for (uint8_t i = 0; i < 255; i = i + 1)
    {
        for (uint8_t j = 1; j <= 4; j = j + 1)
        {
            setGeld((rand() % 10000),j,1);
        }
        for (uint8_t j = 0; j < 2; j = j + 1)
        {
            wuerfelTransmit((rand() % 6) + 1, (rand() % 6) + 1);
        }
        _delay_ms(10);
        
    }

}