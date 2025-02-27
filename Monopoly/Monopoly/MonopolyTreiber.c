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
#pragma GCC optimize 0
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


//#define SIEBENSEGMENT_OFF 0
/*--- Datentypen (typedef) --------------------------------------------------*/
rgb_color leds[LED_COUNT];
typedef enum {INVENTAR_PRUEFEN, HAEUSER_J_N, HAEUSER, BELASTEN_J_N, BELASTEN, GENUG_GELD, PLEITE, GROSSVERSTEIGERUNG, FELDER_ABGEBEN, ENDE_VERSTEIGERUNG}pleite_t;
/*--- Globale Konstanten ----------------------------------------------------*/
/*--- Globale Variablen -----------------------------------------------------*/
/*--- Modullokale Konstanten ------------------------------------------------*/
/*--- Modullokale Variablen -------------------------------------------------*/
pleite_t pleiteZustand = GENUG_GELD; 
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
    setPlayerPosition(SPIELER_START_FELD, 1);
    setPlayerPosition(SPIELER_START_FELD, 2);
    setPlayerPosition(SPIELER_START_FELD, 3);
    setPlayerPosition(SPIELER_START_FELD, 4);

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
    uint8_t anzahlLeds, startRegister, startLed, hausWert = 0;
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
    (SEG_G)                                                                     //12 - (---- = Spieler ist aus versteigerung zurückgetreten)
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
    uint8_t tausender, hunderter, zehner, einer, transmitdata = 0;

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
void wuerfel(void)
{
    char buffer[16];
    uint8_t zufallszahl1, zufallszahl2 = 0;
    
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
void wuerfelAB(uint8_t wuerfelNummer, uint8_t flagWuerfel1, uint8_t flagWuerfel2)
{
    //Variabeln für zufallszahl 1 und zufallszahl 2 initialisieren
    uint8_t zufallszahl1, zufallszahl2 = 10;
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
    setPlayerPosition(10,spielerNr);
}

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
* Setzt die register A bis L
* 
*
*
*
* Parameter:
* Keine Parameter (void)
*
*
* Rückgabewert: kein Rückgabewert (void)
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
* Teilt das Startgeld an die ausgewählte anzahl spieler aus
*
*
*
*
* Parameter:
* keine Parameter (void)
*
*
* Rückgabewert: kein Rückgabewert (void)
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
            updateKontostand(anzahlSpieler,spielerInfo); 
        }
        //warte 50 ms
        _delay_ms(50); //Delay dient zu animationszwecken
    }
}



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
    spielfeld[1].besitzer = 1;
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
    spielfeld[3].besitzer = 1;
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
    spielfeld[5].besitzer = 4;
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
    spielfeld[11].besitzer = 3;
    spielfeld[11].farbGruppe = ROSA;
    spielfeld[11].farbgruppenFelder[0] = 11;
    spielfeld[11].farbgruppenFelder[1] = 13;
    spielfeld[11].farbgruppenFelder[2] = 14;
    spielfeld[11].hausnummer = 5;
    spielfeld[11].anzahlHaeuser = 5;
    spielfeld[11].kostenHaus = 100;
    spielfeld[11].rgbNummer = 6;
    spielfeld[11].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Informatikdienst
    strcpy(spielfeld[12].name, "Informatikdienst");
    spielfeld[12].typ = WERK;
    spielfeld[12].preis = 150;
    spielfeld[12].besitzer = 2;
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
    spielfeld[13].besitzer = 3;
    spielfeld[13].farbGruppe = ROSA;
    spielfeld[13].farbgruppenFelder[0] = 11;
    spielfeld[13].farbgruppenFelder[1] = 13;
    spielfeld[13].farbgruppenFelder[2] = 14;
    spielfeld[13].hausnummer = 6;
    spielfeld[13].anzahlHaeuser = 5;
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
    spielfeld[14].besitzer = 3;
    spielfeld[14].farbGruppe = ROSA;
    spielfeld[14].farbgruppenFelder[0] = 11;
    spielfeld[14].farbgruppenFelder[1] = 13;
    spielfeld[14].farbgruppenFelder[2] = 14;
    spielfeld[14].hausnummer = 7;
    spielfeld[14].anzahlHaeuser = 5;
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
    spielfeld[15].besitzer = 4;
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
    spielfeld[16].anzahlHaeuser = 5;
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
    spielfeld[18].besitzer = 4;
    spielfeld[18].farbGruppe = ORANGE;
    spielfeld[18].farbgruppenFelder[0] = 16;
    spielfeld[18].farbgruppenFelder[1] = 18;
    spielfeld[18].farbgruppenFelder[2] = 19;
    spielfeld[18].hausnummer = 9;
    spielfeld[18].anzahlHaeuser = 5;
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
    spielfeld[19].besitzer = 4;
    spielfeld[19].farbGruppe = ORANGE;
    spielfeld[19].farbgruppenFelder[0] = 16;
    spielfeld[19].farbgruppenFelder[1] = 18;
    spielfeld[19].farbgruppenFelder[2] = 19;
    spielfeld[19].hausnummer = 10;
    spielfeld[19].anzahlHaeuser = 5;
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
    spielfeld[25].besitzer = 2;
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
    spielfeld[28].besitzer = 2;
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
    spielfeld[31].besitzer = 1;
    spielfeld[31].farbGruppe = GRUEN;
    spielfeld[31].farbgruppenFelder[0] = 31;
    spielfeld[31].farbgruppenFelder[1] = 32;
    spielfeld[31].farbgruppenFelder[2] = 34;
    spielfeld[31].hausnummer = 17;
    spielfeld[31].anzahlHaeuser = 0;
    spielfeld[31].kostenHaus = 200;
    spielfeld[31].rgbNummer = 22;
    spielfeld[31].feldBelastet = 1;  //wenn das Feld belastet ist = 1
    
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
    spielfeld[32].feldBelastet = 1;  //wenn das Feld belastet ist = 1
    
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
    spielfeld[34].feldBelastet = 1;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Gewerbeschule
    strcpy(spielfeld[35].name, "Gewerbeschule");
    spielfeld[35].typ = HALTESTELLE;
    spielfeld[35].preis = 200;
    spielfeld[35].mieten[0] = 25;   //wenn man 1 Bahn besitzt
    spielfeld[35].mieten[1] = 50;   //wenn man 2 Bahnen besitzt
    spielfeld[35].mieten[2] = 100;  //wenn man 3 Bahnen besitzt
    spielfeld[35].mieten[3] = 200;  //wenn man 4 Bahnen besitzt
    spielfeld[35].besitzer = 3;
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
    spielfeld[37].besitzer = 1;
    spielfeld[37].farbGruppe = BLAU;
    spielfeld[37].farbgruppenFelder[0] = 37;
    spielfeld[37].farbgruppenFelder[1] = 39;
    spielfeld[37].farbgruppenFelder[2] = 0;
    spielfeld[37].hausnummer = 20;
    spielfeld[37].anzahlHaeuser = 0;
    spielfeld[37].kostenHaus = 200;
    spielfeld[37].rgbNummer = 26;
    spielfeld[37].feldBelastet = 1;  //wenn das Feld belastet ist = 1
    
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
    spielfeld[39].besitzer = 1;
    spielfeld[39].farbGruppe = BLAU;
    spielfeld[39].farbgruppenFelder[0] = 37;
    spielfeld[39].farbgruppenFelder[1] = 37;
    spielfeld[39].farbgruppenFelder[2] = 37;
    spielfeld[39].farbgruppenFelder[3] = 39;
    spielfeld[39].hausnummer = 21;
    spielfeld[39].anzahlHaeuser = 0;
    spielfeld[39].kostenHaus = 200;
    spielfeld[39].rgbNummer = 27;
    spielfeld[39].feldBelastet = 1;  //wenn das Feld belastet ist = 1
}

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
    chanceKanzlei[6].bewegung = 0;
    
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
    "Oli mag dich    nicht.          Gehe ins        Gef"AE"ngnis",
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
    "Ihr macht eine 	Exkursion       zahle 50",
    "Du hast die     Abschlussreise  organisiert,    jeder Spieler   zahlt dir 10",
    "Elektroniker    des Monats      Du erh"AE"ltst 100",
    "Du hilfst deinemMitschüler beim lernen.         Du erh"AE"ltst 10",
    "Du erh"AE"lst 50",
    "Du erh"AE"lst 100",
    "Du erh"AE"lst 25",
    "Du erh"AE"lst 20",
    "Du erh"AE"lst 100",
    "Du erh"AE"lst 100", 
    "Du bist         Weltmeister.    Rücke vor bis   Start ziehe den doppelten       Betrag ein",
    "Du hast gegen   die Handyregel  verstossen.     R"UE"cke vor bis   Produktion      Elektroniker" 
};


void read_string(char *buf, size_t i) 
{
    // Kopiere direkt aus dem Flash ins RAM
    strcpy_P(buf, kartenArray[i]);
}

#define ANZAHL_KARTEN 17
uint8_t zufallsNummer = 0;
uint8_t ereignisFeld(uint8_t kanzlei, uint8_t spielerAmZug, uint8_t schritt, uint8_t flagWeiter, Karte chanceKanzlei[])
{
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
    if (!schritt)
    {
        rueckgabewert = 0;
        zufallsNummer = (rand() % 16);
        if (kanzlei)
        {
            zufallsNummer += ANZAHL_KARTEN;
        }
        read_string(text, zufallsNummer);  //Liest den Text aus dem Flash speicher
    }
    
    if (flagWeiter) //wenn der Spieler bestätigt hat.
    {
        switch(chanceKanzlei[zufallsNummer].typ)
        {
            case WORKSHOP:
            spielerInfo[spielerAmZug].position = 10; //setzt die Position des spielers auf gefängnis
            spielerInfo[spielerAmZug].gefaengnis = 1; //vermerkt den Spieler als Häftling
            spielerInfo[spielerAmZug].rundenImGefaengnis = 0; //setzt die anzahl im gefängnis verbrachten runden auf 0
            setPlayerPosition(10, spielerAmZug);
            break;
            case FREIKARTE:
            spielerInfo[spielerAmZug].freikarte = 1; //gibt dem Spieler eine Freikarte
            break;
            case RENOVIEREN:
            //berechet den Preis für die Häuser und die hotels
            hausBetrag = spielerInfo[spielerAmZug].haeuser * chanceKanzlei[zufallsNummer].geld;
            hausBetrag = spielerInfo[spielerAmZug].hotels * chanceKanzlei[zufallsNummer].geld2;
            //zieht den berechneten betrag vom spieler ab
            geldUeberweisen(spielerAmZug,0,(hausBetrag + hotelBetrag),1);
            break;
            case GELD_AN_BANK:
            geldUeberweisen(spielerAmZug,0,chanceKanzlei[zufallsNummer].geld,1);
            break;
            case GELD_VON_BANK:
            geldUeberweisen(0,spielerAmZug,chanceKanzlei[zufallsNummer].geld,1);
            break;
            case GELD_AN_MITSPIELER:
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                if (!(i == spielerAmZug))
                {
                    geldUeberweisen(spielerAmZug,i,chanceKanzlei[zufallsNummer].geld,1);
                }
                
            }
            break;
            case GELD_VON_MITSPIELER:
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                if (i == spielerAmZug)
                {
                    geldUeberweisen(i,spielerAmZug,chanceKanzlei[zufallsNummer].geld,1);
                }
            }
            break;
            case BEWEGEN:
            if (chanceKanzlei[zufallsNummer].bewegung)
            {
                ausgangsPosition = spielerInfo[spielerAmZug].position;
                neuePosition = ausgangsPosition + chanceKanzlei[zufallsNummer].bewegung;
                setPlayerPosition(neuePosition, spielerAmZug);
            }
            else
            {
                ausgangsPosition = spielerInfo[spielerAmZug].position;
                for (uint8_t i = ausgangsPosition; !(chanceKanzlei[zufallsNummer].zielFeldTyp == spielfeld[i].typ); i = i + 1)
                {
                    anzahlFelder = i;
                    neuePosition = (i % 40) + 1;
                    setPlayerPosition(neuePosition, spielerAmZug);
                    if (neuePosition == 0)
                    {
                        geldUeberweisen(0,spielerAmZug,200,10);
                    }
                    _delay_ms(100);
                }
            }
            //setzt die neue Position
            spielerInfo[spielerAmZug].position = neuePosition;
            //setzt den spieler auf das richtige Feld
            setPlayerPosition(spielerInfo[spielerAmZug].position,spielerAmZug);
            break;
            case TELEPORTIEREN:
            //wenn die aktuelle spition des Spielers + wüfelsumme grösser gleich 40 is
            // erhält der spieler 200 CHF
            
            if (spielerInfo[spielerAmZug].position > chanceKanzlei[zufallsNummer].zielFeld)
            {
                //berechnet den betrag, den man auf start erhält
                startGeld = 200 + chanceKanzlei[zufallsNummer].geld;
                //animiert die fortbewegung des spielers bis feld Los
                for (uint8_t i = spielerInfo[spielerAmZug].position; i <= 40; i = i + 1)
                {
                    setPlayerPosition(i % 40,spielerAmZug);
                    _delay_ms(100); //delay dient zu animationszwecken
                }
                geldUeberweisen(0,spielerAmZug,startGeld,10);
            }
            for (uint8_t i = spielerInfo[spielerAmZug].position; i < chanceKanzlei[zufallsNummer].zielFeld; i = i + 1)
            {
                setPlayerPosition(i % 40,spielerAmZug);
                _delay_ms(100); //delay dient zu animationszwecken
            }
            //addiert die würfelsumme zur aktuellen position dazu
            spielerInfo[spielerAmZug].position = chanceKanzlei[zufallsNummer].zielFeld;
            //setzt den spieler auf das richtige Feld
            setPlayerPosition(spielerInfo[spielerAmZug].position,spielerAmZug);
            break;
        }
        
        
        return 1;
    }
    
    if (!rueckgabewert)
    {
        rueckgabewert = lcdLauftext(text,schritt); //schreibt den Text auf das LCD
        writeText(0,0,"X OK     Weiter"PFEIL_R);
    }
    else
    {
        writeText(0,0,"X OK            ");
    }
    return 0;
    
}

uint8_t geldUeberweisen(uint8_t zahler, uint8_t empfaenger, uint16_t betrag, uint8_t schritt)
{
    uint16_t restBetrag = 0;
    if (empfaenger && zahler) //wenn der empfänger nicht spieler 0 ist
    {
        //wenn der Zahlende Spieler genug geld hat
        if (spielerInfo[zahler].geld >= betrag)
        {
            for (uint16_t i = 0; i < betrag; i = i + schritt)
            {
                spielerInfo[zahler].geld -= schritt;
                spielerInfo[empfaenger].geld += schritt;
                updateKontostand(anzahlSpieler,spielerInfo);
            }
            return 1; //zahlung erfolgreich
        }
        else //wenn es sich der Spieler nicht leisten kann
        {
            flagGeldBeschaffen = 1;//flag setzen
            restBetrag = betrag - spielerInfo[zahler].geld;//restbetrag berechnen
            geldUeberweisen(zahler,empfaenger,spielerInfo[zahler].geld,1);//gesammtes Geld überweisen
            geldBeschaffen(zahler, empfaenger, restBetrag);//restbetrag beschaffen
            return 1;
            //geldUeberweisen(zahler,empfaenger,betrag,1);//restlichesGeld überweisen
            //return 2; //zahlung fehlgeschlagen
        }
    }
    else if (!empfaenger) //wenn spieler 0 als empfänger eingegeben wurde, wird an die Bank überwiesen
    {
        //wenn der Zahlende Spieler genug geld hat
        if (spielerInfo[zahler].geld >= betrag)
        {
            for (uint16_t i = 0; i < betrag; i = i + schritt)
            {
                spielerInfo[zahler].geld -= schritt;
                updateKontostand(anzahlSpieler,spielerInfo);
            }
            return 1; //zahlung erfolgreich
        }
        else //wenn es sich der Spieler nicht leisten kann
        {
            flagGeldBeschaffen = 1;//flag setzen
            restBetrag = betrag - spielerInfo[zahler].geld;//restbetrag berechnen
            geldUeberweisen(zahler,empfaenger,spielerInfo[zahler].geld,1);//gesammtes Geld überweisen
            geldBeschaffen(zahler, empfaenger, restBetrag);//restbetrag beschaffen
            return 1;
            //return 2; //zahlung fehlgeschlagen
        }
    }
    else if (!zahler)//wenn spieler 0 als zahler eingegeben wurde, kommt das Geld von der Bank
    {
        for (uint16_t i = 0; i < betrag; i = i + schritt)
        {
            spielerInfo[empfaenger].geld += schritt;
            updateKontostand(anzahlSpieler,spielerInfo);
        }
        return 1; //zahlung erfolgreich
    }
}

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

uint8_t geldBeschaffen(uint8_t zahler, uint8_t empfaenger, uint16_t mindestBetrag)
{
    uint8_t flagHaeuser, flagBelastebar, flagFarbgruppe, flagFeldBelastet, feldZaehler = 0;
    uint8_t felderMitHaeuser[40] = {0};
    uint8_t felderBelastbar[40] = {0};
    uint8_t anzahlFelderMitHaeuser, anzahlFelderBelastbar = 0;
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
    
    //uint8_t farbgruppenErstesFeld[8] = {1,6,11,16,21,26,31,37};
    char lcdBuffer[16];
    //Solange das Flag gesetzt ist
    while (flagGeldBeschaffen)
    {
        //Flankenerkennung
        tasteAlt = tasteNeu;
        tasteNeu = 0;
        tasteNeu = (PINL << 8) | PINK;
        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        
        switch (pleiteZustand)
        {
            case INVENTAR_PRUEFEN://überprüft ob der Spieler etwas besitzt, das man verkaufen kann
            anzahlFelderMitHaeuser = 0; //anzahlFelderMitHaeuser zurücksetzen
            anzahlFelderBelastbar = 0;  //anzahlFelderBelastbar zurücksetzen
            flagHaeuser = 0;
            flagBelastebar = 0;
            flagFarbgruppe = 0;
            //geht alle Felder durch
            for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
            {
                //Position "i" in der Liste zurücksetzen
                felderMitHaeuser[i] = 0;    //position "i" in felderMitHaeuser zurücksetzen
                felderBelastbar[i] = 0;     //position "i" in felderBelastbar zurücksetzen
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Häuser~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                //Wenn das Feld dem Spieler gehört, es Häuser hat und nicht belastet ist
                if ((spielfeld[i].besitzer == zahler) && spielfeld[i].anzahlHaeuser && (!spielfeld[i].feldBelastet))
                {
                    //wenn die bedingung eintrifft, wird das Feld in die Liste aufgenommen
                    felderMitHaeuser[anzahlFelderMitHaeuser] = i;//Speichert die Feldnummer in dem Array
                    anzahlFelderMitHaeuser += 1; //anzahlFelderMitHaeuser erhöhen
                    flagHaeuser = 1;//flagHaeuser setzen, da mindestens 1 Haus verkauft werden kann
                }
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~BELASTEN~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                //Wenn das Feld dem Spieler gehört und es nicht belastet ist
                if ((spielfeld[i].besitzer == zahler) && !spielfeld[i].feldBelastet)
                {
                    flagFarbgruppe = 1; //flagFarbgruppe Setzen
                    //überprüft ob es auf den anderen Farbgruppenfelder Häuser hat
                    for (uint8_t j = 0; j < 3; j = j + 1)
                    {
                        //Wenn es auf einem Feld der Farbgruppe ein Haus hat
                        if (spielfeld[spielfeld[i].farbgruppenFelder[j]].anzahlHaeuser)
                        {
                            //flag zurücksetzen | Feld kann erst belastet werden,
                            //wenn alle Häuser der Farbgruppe verkauft sind
                            flagFarbgruppe = 0;
                        }
                    }
                    //prüfe ob flagFarbgruppe noch gesetzt ist
                    if (flagFarbgruppe)
                    {
                        //speichert die aktuelle Feldnummer
                        felderBelastbar[anzahlFelderBelastbar] = i;
                        anzahlFelderBelastbar += 1; //anzahlFelderBelastbar erhöhen
                        flagBelastebar = 1; //flagBelastbar setzen, da mindestens ein Feld belastet werden kann
                    }
                }
            }
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~nächsten Zustand bestimmen~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //wenn flagHaeuser gesetzt ist d.h. der spieler besitzt häuser
            if (flagHaeuser)
            {
                //LCD ausgabe spieler am Zug und Navigation
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",zahler);
                writeText(0,11,lcdBuffer);
                writeText(1,0,"Haus verkaufen? ");
                writeText(2,0,"X Ja      Nein Y");
                pleiteZustand = HAEUSER;
            }//wenn flagBelastebar gesetzt ist d.h. der spieler hat belastbare Felder
            else if (flagBelastebar)
            {
                //LCD ausgabe spieler am Zug und Navigation
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",zahler);
                writeText(0,11,lcdBuffer);
                writeText(1,0," Feld Belasten? ");
                writeText(2,0,"X Ja      Nein Y");
                pleiteZustand = BELASTEN;
            }//wenn weder flagHaeuser noch flagBelastebar gesetzt ist
            else
            {
                //der Spieler kann nichts mehr verkaufen oder belasten!
                //der Spieler scheidet aus dem Spiel aus
                pleiteZustand = PLEITE;
            }
            break;
            case HAEUSER:
            //wenn Taste X betätigt wurde --> Haus verkaufen
            if (positiveFlanke & xTasten[zahler - 1])
            {
                //lässt den Spieler ein Haus verkaufen
                hausBauen(zahler);
                pleiteZustand = GENUG_GELD;//zustandswechsel
            }//wenn Taste Y betätigt wurde --> Haus nicht verkaufen
            else if (positiveFlanke & yTasten[zahler - 1])
            {
                //prüft ob der Spieler Felder Belasten kann
                if (flagBelastebar)
                {
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);
                    writeText(0,11,lcdBuffer);
                    writeText(1,0," Feld Belasten? ");
                    writeText(2,0,"X Ja      Nein Y");
                    pleiteZustand = BELASTEN;
                }//Wenn der Spieler kein Feld belasten kann
                else
                {
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);
                    writeText(0,11,lcdBuffer);
                    writeText(1,0," Du must H"AE"user ");
                    writeText(2,0,"   verkaufen!   ");
                    _delay_ms(3000);
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);
                    writeText(0,11,lcdBuffer);
                    writeText(1,0,"Haus verkaufen? ");
                    writeText(2,0,"X Ja      Nein Y");
                }
            }
            break;
            case BELASTEN:
            //wenn Taste X betätigt wurde --> Feld Belasten
            if (positiveFlanke & xTasten[zahler - 1])
            {
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",zahler);
                writeText(0,11,lcdBuffer);
                writeText(1,0,PFEIL_O"Hyp. | weiter "PFEIL_R);
                writeText(2,0,"                ");
                writeText(2,0,spielfeld[felderBelastbar[0]].name);//Name von erstem Feld ausgeben;
                flagFeldBelastet = 0;
                feldZaehler = 0;
                //Schleife, bis ein Feld Belastet wurde
                while (!flagFeldBelastet)
                {
                    //Flankenerkennung
                    tasteAlt = tasteNeu;
                    tasteNeu = 0;
                    tasteNeu = (PINL << 8) | PINK;
                    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                    
                    //wenn nächstes Feld angezeigt werden soll
                    if (positiveFlanke & TASTE_R)
                    {
                        //prüft ob es noch belastbare Felder in der liste hat
                        //und ob der zähler noch erhöt werden darf
                        if (felderBelastbar[feldZaehler + 1] && feldZaehler < 39)
                        {
                            feldZaehler += 1; //feldzähler erhöhen
                            writeText(2,0,"                ");
                            writeText(2,0,spielfeld[felderBelastbar[feldZaehler]].name);//Name von nächsten Feld ausgeben;
                        }
                        else //wenn das aktuelle Feld das letzte in der liste ist
                        {
                            feldZaehler = 0;
                            writeText(2,0,"                ");
                            writeText(2,0,spielfeld[felderBelastbar[feldZaehler]].name);//Name von nächsten Feld ausgeben;
                        }
                    }
                    else if (positiveFlanke & TASTE_O)//Wenn das Feld belastet werden soll
                    {
                        //Feld als Belastet speichern
                        spielfeld[felderBelastbar[feldZaehler]].feldBelastet = 1;
                        //Hypothek Wert berechnen
                        hypothekBetrag = spielfeld[felderBelastbar[feldZaehler]].preis / 2;
                        geldUeberweisen(0,zahler,hypothekBetrag,1);
                        
                        if (spielfeld[felderBelastbar[feldZaehler]].typ == STRASSE)//wenn es eine Strase ist
                        {
                            //Markiert das Feld als verpfändet
                            //alle 5 haus LEDs werden eingeschaltet
                            setHaus(spielfeld[felderBelastbar[feldZaehler]].hausnummer,6);
                        }
                        else //wenn es keine strasse ist
                        {
                            //wenn es keine häuser hat, die man als Markierung nutzen kann
                            // wird die RGB auf weiss gestellt.
                            setPropertyRgb(spielfeld[felderBelastbar[feldZaehler]].rgbNummer,5);
                        }
                        flagFeldBelastet = 1;//Flag setzen um loop zu verlassen
                    }
                }
                pleiteZustand = GENUG_GELD; // Zustandswechsel
            }//wenn Taste Y betätigt wurde --> Feld nicht belasten
            else if (positiveFlanke & yTasten[zahler - 1])
            {
                //prüft ob der Spieler Häuser verkaufen kann
                if (flagHaeuser)
                {
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);
                    writeText(0,11,lcdBuffer);
                    writeText(1,0,"Haus verkaufen? ");
                    writeText(2,0,"X Ja      Nein Y");
                    pleiteZustand = HAEUSER;
                }//Wenn der Spieler kein Feld belasten kann
                else
                {
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);
                    writeText(0,11,lcdBuffer);
                    writeText(1,0," Du must Felder ");
                    writeText(2,0,"   belasten!    ");
                    _delay_ms(3000);
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",zahler);
                    writeText(0,11,lcdBuffer);
                    writeText(1,0," Feld Belasten? ");
                    writeText(2,0,"X Ja      Nein Y");
                }
            }
            
            break;
            case GENUG_GELD:
            //prüft ob der Spieler das minimum auftreiben konnte
            if (spielerInfo[zahler].geld >= mindestBetrag)
            {
                //Überweist den restlichen betrag
                geldUeberweisen(zahler,empfaenger,mindestBetrag,1);
                //Spieler Nummer an LCD anzeigen
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",zahler);
                writeText(0,11,lcdBuffer);
                writeText(1,0,"                ");
                writeText(2,0,"Schuld beglichen");
                _delay_ms(5000);
                flagGeldBeschaffen = 0;
            }
            else
            {
                //berechnet die restlichen Schulden die der Spieler hat
                schuldBetrag = mindestBetrag - spielerInfo[zahler].geld;
                //Spieler Nummer an LCD anzeigen
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",zahler);
                writeText(0,11,lcdBuffer);
                
                writeText(1,0,"  Du schuldest  ");
                writeText(2,0,"                ");
                sprintf(lcdBuffer,"%4u",schuldBetrag);
                writeText(2,6,lcdBuffer);
                blaulicht(100,4);//Blaulicht
                pleiteZustand = INVENTAR_PRUEFEN;
            }
            break;
            case PLEITE:
            //Der Spieler hat alle seine Felder belastet und alle Häuser verkauft. 
            //Da der spieler nichts mehr besitzt scheidet er aus dem Spiel aus
            //Wenn der spieler schulden bei einem Mitspieler hat geht sein ganzer Besitz an
            //den Mitspieler. d.h. alle Felder und Freikarten und restliches Geld.
            //Bei Schulden bei der Bank, werden alle Felder Versteigert. Restliches Geld geht an die Bank
            writeText(0,0,"   Spieler      ");
            sprintf(lcdBuffer,"%u",zahler);
            writeText(0,11,lcdBuffer);
            writeText(1,0," CRAZY  BLINKEN ");
            writeText(2,0," du bist PLEITE ");
            spielerInfo[zahler].pleite = 1;//Markiert den Spieler als Pleite. Er spielt nicht mehr mit
            spielerInfo[zahler].position = 40;
            //setGeld(0,zahler,0);//Siebensegment ausschalten
            setPlayerPosition(40,zahler);//entfernt die Spielfigur vom Spielfeld
            _delay_ms(5000);
            
            
            //Sucht alle Felder nach Feldern ab, die dem Spieler gehörten
            for(uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
            {
                inventar[i] = 0; // inventar zurücksetzen
                //wenn das aktuelle Feld dem Spieler gehörte
                if (spielfeld[i].besitzer == zahler)
                {
                    /*spielfeld[i].feldBelastet = 0;//entfernt die Hypothek vom Haus
                    //Wenn es sich bei dem Feld um eine Strasse Handelt
                    if (spielfeld[i].typ == STRASSE)
                    {
                        setHaus(spielfeld[i].hausnummer,0);//entfernt die Markierung vom Feld
                    }
                    else
                    {
                        //Entfernt die RGB Markierung
                        setPropertyRgb(spielfeld[i].rgbNummer,0);
                    }*/
                    inventar[inventarZaehler] = i;//speichert die Feldnummer im Inventar
                    inventarZaehler += 1; //erhöt den Inventarzähler um 1
                    
                }
            }
            
            
            //bestimmen ob der Spieler Schulden bei der Bank oder bei einem Mitspieler hatte.
            if (!empfaenger) //Wenn der Empfänger die Bank ist
            {
                //die Hypotheken verfallen. Alle felder die versteigert werden sind automatisch nicht mehr belastet
                //Der Spieler hatte Schulden bei der Bank
                //Sein Ganzes restliches Geld wird überwiesen an die Bank
                geldUeberweisen(zahler,0,spielerInfo[zahler].geld,1);
                //freikarten löschen
                spielerInfo[zahler].freikarte = 0; 
                for (uint8_t i = 0; i < inventarZaehler; i = i + 1)
                {
                    spielfeld[inventar[i]].feldBelastet = 0;//Hypothek vom Feld entfernen
                    if (spielfeld[inventar[i]].typ == STRASSE)//Prüft ob das Feld eine Strasse ist
                    {
                        //wenn es eine Strasse ist ist die Hypothek mit 6 Häusern markiert
                        setHaus(spielfeld[inventar[i]].hausnummer,0);//Entfernt die Markierung auf dem Feld
                    }
                    else
                    {
                        //wenn das Feld keine Strasse ist, ist die Hypothek mit einer weissen rgb gekennzeichnet
                        setPropertyRgb(spielfeld[inventar[i]].rgbNummer,RGB_BANK);//schaltet die RGB auf die Farbe der Bank
                    }
                }
                //alle Felder des Spielers werden in der Grossversteigerung versteigert.
                pleiteZustand = GROSSVERSTEIGERUNG;
                flagNeuesFeld = 1;
            }
            else
            {
                //Der Spieler hatte Schulden bei einem Mitspieler. Sein Besitz wird dem Mitspieler übergeben
                //Sein Restliches Geld wird an den Mitspieler überwiesen
                geldUeberweisen(zahler,empfaenger,spielerInfo[zahler].geld,1);
                pleiteZustand = FELDER_ABGEBEN;
                //wird alles wieder rausgelöscht nur zum testen ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                /*for (uint8_t i = 0; i < inventarZaehler; i = i + 1)
                {
                    spielfeld[inventar[i]].feldBelastet = 0;//Hypothek vom Feld entfernen
                    if (spielfeld[inventar[i]].typ == STRASSE)//Prüft ob das Feld eine Strasse ist
                    {
                        //wenn es eine Strasse ist ist die Hypothek mit 6 Häusern markiert
                        setHaus(spielfeld[inventar[i]].hausnummer,0);//Entfernt die Markierung auf dem Feld
                    }
                    else
                    {
                        //wenn das Feld keine Strasse ist, ist die Hypothek mit einer weissen rgb gekennzeichnet
                        setPropertyRgb(spielfeld[inventar[i]].rgbNummer,RGB_BANK);//schaltet die RGB auf die Farbe der Bank
                    }
                }
                //alle Felder des Spielers werden in der Grossversteigerung versteigert.
                pleiteZustand = GROSSVERSTEIGERUNG;
                flagNeuesFeld = 1;*/
                //wird alles wieder rausgelöscht nur zum testen ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            }
            break;
            case GROSSVERSTEIGERUNG:
            //Wenn ein neues Feld versteigert wird
            if(flagNeuesFeld)
            {
                for (uint8_t i = 0; i < 6; i = i + 1)
                {
                    //bieter array --> speichert welche Spieler von der Versteiogerung zurückgetreten sind = 0 - 3
                    //anzahl zurückgetretene Spieler = 4
                    //höchstbietender Spieler = 5
                    bieter[i] = 0; //array zurücksetzen
                }
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    //wenn der spieler Pleite ist, darf er nicht mitbieten
                    setGeld(0,i,0);//Siebensegmente aller Spieler ausschalten
                    if (spielerInfo[i].pleite)
                    {
                        setGeld(0,i,2);//Spieler spielt nicht mehr mit => ---- anzeigen
                        bieter[i - 1] = 1; //Schliesst den Spieler aus der versteigerung aus
                        bieter[4] = bieter[4] + 1; //erhöht anzahl zurückgezogene spieler 
                    }
                }
                clear();
                //LCD ausgabe des aktuellen Feldes
                writeText(0,0," VERSTEIGERUNG  ");
                writeText(1,0,spielfeld[inventar[anzahlVersteigerteFelder]].name);
                writeText(2,0,"bieten X sonst Y");
                hoechstGebot = 0;//STARTGEBOT auf 0 setzen --> danach 10er Schritte
                flagNeuesFeld = 0; 
            }
            //prüft alle eingaben der Spieler
            for (uint8_t i = 0; i < anzahlSpieler; i = i + 1)
            {
                //wenn der Spieler noch in der versteigerung dabei ist und er mehr Geld hat als das Höchstgebot + 10
                if ((positiveFlanke & xTasten[i]) && !bieter[i] && (spielerInfo[i + 1].geld >= hoechstGebot + 10))
                {
                    hoechstGebot += 10;//höchstgebot um 10 erhöhen
                    bieter[5] = i + 1; //Speichert die Spielernummer des spielers mit dem Höchsten gebot
                    for (uint8_t j = 1; j <= anzahlSpieler; j = j + 1)
                    {
                        //wenn der Spieler noch am bieten ist und nicht der höchstbieter ist
                        if (!bieter[j - 1] && !(bieter[5] == j))
                        {
                            setGeld(0,j,0);//wenn der Spieler kein Gebot abgegeben hat aber noch mitbietet siebensegmente abschalten
                        }
                    }
                    setGeld(hoechstGebot,i + 1,1);//Ausgabe Höchstgebot

                }
                //wenn der Spieler die Taste Y zum ersten mal in diese versteigerungsrunde betätigt hat & nicht der höchstbieter geboten hat
                if ((positiveFlanke & yTasten[i]) && !bieter[i] && !(positiveFlanke & yTasten[bieter[5] - 1]))
                {
                    bieter[i] = 1; //schliesst spieler aus auktion aus                  bieter 0 - 3 spieler zurückgetreten
                    bieter[4] = bieter[4] + 1; //erhöht anzahl zurückgezogene spieler   bieter 4 anzahl zurückgetretene Spieler
                    setGeld(0,i + 1,2);//---- anzeigen
                }
            }
            
            //wenn alle Spieler ausser der höchstbieter nicht mehr bieten
            if (bieter[4] == anzahlSpieler - 1)
            {
                writeText(0,0," versteigert an ");
                writeText(1,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",bieter[5]);
                writeText(1,11,lcdBuffer);
                
                spielfeld[inventar[anzahlVersteigerteFelder]].besitzer = bieter[5];//Höchstbieter wird als neuer besitzer gespeichert
                //rgb Farbe auf die Farbe des neuen Besitzers schreiben
                setPropertyRgb(spielfeld[inventar[anzahlVersteigerteFelder]].rgbNummer,bieter[5]);
                anzahlVersteigerteFelder += 1;
                //wenn es noch Felder zu versteigern gibt
                if (inventar[anzahlVersteigerteFelder])
                {
                    flagNeuesFeld = 1;//flag ermöglicht es, dass das nächste Feld versteigert wird
                }
                else
                {
                    //wenn es nichts mehr zu versteigern gibt, ist die Versteigerung beendet
                    pleiteZustand = ENDE_VERSTEIGERUNG;
                }
                _delay_ms(1000);//1s warten
            }
            break;
            case ENDE_VERSTEIGERUNG:
            writeText(0,0," VERSTEIGERUNG  ");
            writeText(1,0,"    BEENDET     ");
            writeText(2,0,"                ");
            _delay_ms(3000);
            flagGeldBeschaffen = 0; // flag auf 0 setzen um aus der while Schleife rauszukommen
            pleiteZustand = GENUG_GELD;//startzustand festlegen
            break;
            case FELDER_ABGEBEN:
            //Der Spieler hatte Schulden bei einem Mitspieler
            writeText(0,0,"    ABGEBEN     ");
            //geht alle Felder die übertragen werden durch
            for (uint8_t i = 0; i < inventarZaehler; i = i + 1)
            {
                flagFeldBelastet = 1;
                flagMussHypBehalten = 0;
                flagZuWenigGeld = 0;
                updateLcd = 0;
                while(flagFeldBelastet)
                {
                    //Flankenerkennung
                    tasteAlt = tasteNeu;
                    tasteNeu = 0;
                    tasteNeu = (PINL << 8) | PINK;
                    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                    if (!updateLcd)
                    {
                        feldNummer = inventar[i];
                        writeText(0,0,"                ");
                        writeText(0,0,spielfeld[feldNummer].name);
                        writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                        writeText(2,0,"andere option  "PFEIL_U);
                        updateLcd = 1;
                    }
                    if (positiveFlanke & TASTE_O)
                    {
                        if (hypothekAufloesen)//Hypothek soll aufgelöst werden
                        {
                            bezahlBetrag = spielfeld[feldNummer].preis * 0.55;
                            //bezahlBetrag = (bezahlBetrag / 2) + (bezahlBetrag / 20);
                            //prüft ob der Spieler genug Geld hat um die Hypothek aufzulösen
                            if (spielerInfo[empfaenger].geld >= bezahlBetrag)
                            {
                                //Spieler hat genug Geld
                                writeText(1,0,"Zahle          S");
                                sprintf(lcdBuffer,"%u",bezahlBetrag);
                                writeText(1,6,lcdBuffer);
                            }
                            else
                            {
                                //der Spieler hat nicht genug Geld um die Hypothek aufzulösen
                                writeText(1,0," zu wenig Geld  ");
                                _delay_ms(3000);
                                //berechnet den neuen Betrag der bezahlt werden muss um Hyp. zu behalten
                                bezahlBetrag = spielfeld[feldNummer].preis * 0.05;
                                //prüft ob der Spieler sich den neuen Betrag leisten kann
                                if (spielerInfo[empfaenger].geld >= bezahlBetrag)
                                {
                                    //flagMussHypBehalten = 1;
                                    //der SPieler hat genug Geld um die Hypothek zu behalten
                                    writeText(1,0,"du beh"AE"ltst Hyp.");
                                    _delay_ms(3000);
                                    writeText(1,0,"Zahle          S");
                                    sprintf(lcdBuffer,"%u",bezahlBetrag);
                                    writeText(1,6,lcdBuffer);
                                }
                                else
                                {
                                    //der Spieler hat nicht genug Geld um die Hypothek zu behalten.
                                    writeText(1,0,"geht an die Bank");
                                    writeText(2,0,"                ");
                                    _delay_ms(3000);
                                    flagZuWenigGeld = 1;
                                }
                            }
                            
                        }
                        else//wenn die Hypothek nicht aufgelöst werden soll
                        {
                            bezahlBetrag = spielfeld[feldNummer].preis * 0.05;
                            //wenn der Spieler nicht genug Geld hat um die Hypothek zu behalten
                            if (bezahlBetrag > spielerInfo[empfaenger].geld)
                            {
                                writeText(1,0," zu wenig Geld  ");
                                writeText(2,0,"                ");
                                _delay_ms(3000);
                                writeText(1,0,"geht an die Bank");
                                writeText(2,0,"                ");
                                _delay_ms(3000);
                                flagZuWenigGeld = 1;
                            }
                            else
                            {
                                writeText(1,0,"Zahle          S");
                                sprintf(lcdBuffer,"%u",bezahlBetrag);
                                writeText(1,6,lcdBuffer);
                            }
                        }
                        //wenn flagZuWenigGeld nicht gesetzt wurde
                        if (!flagZuWenigGeld)
                        {
                            //warte bis der Spieler mit Taste S bestätigt hat oder mit Taste U zurück zur auswahl gegangen ist
                            while (!(positiveFlanke & TASTE_S) && !(positiveFlanke & TASTE_U))
                            {
                                //Flankenerkennung
                                tasteAlt = tasteNeu;
                                tasteNeu = 0;
                                tasteNeu = (PINL << 8) | PINK;
                                positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                            }
                            //prüft ob der neue besitzer Taste S betätigt hat
                            if (positiveFlanke & TASTE_S)
                            {
                                zahlungErfolgreich = geldUeberweisen(empfaenger,0,bezahlBetrag,1);
                            }
                            else
                            {
                                hypothekAufloesen = (hypothekAufloesen + 1) % 2;
                                if (hypothekAufloesen)
                                {
                                    writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                                }
                                else
                                {
                                    writeText(1,0,"Hyp. Behalten  "PFEIL_O);
                                }
                            }
                            //Wenn die Zahlung erfolgreich war
                            if (zahlungErfolgreich == 1)
                            {
                                //wenn die Hypothek aufgelöst werden soll
                                if (hypothekAufloesen)
                                {
                                    //feld als nicht mehr belastet speichern
                                    spielfeld[feldNummer].feldBelastet = 0;
                                    spielfeld[feldNummer].besitzer = empfaenger;
                                    if (spielfeld[feldNummer].typ == STRASSE)
                                    {
                                        setHaus(spielfeld[feldNummer].hausnummer,0);
                                    }
                                    setPropertyRgb(spielfeld[feldNummer].rgbNummer,empfaenger);
                                    
                                }
                                //flagHandelBelastet auf 0 setzen um aus der schleife raus zu kommen
                                flagFeldBelastet = 0;
                                zahlungErfolgreich = 0;
                            }
                        }
                        else
                        {
                            //Der Spieler hat zuwenig Geld -> Feld geht an die Bank
                            spielfeld[feldNummer].besitzer = 0;//Feld wird der Bank zugeschrieben
                            spielfeld[feldNummer].feldBelastet = 0;//feld als nicht belastet kennzeichnen
                            setPropertyRgb(spielfeld[feldNummer].rgbNummer,0);//RGB ausschalten
                            //Wenn das Feld eine Strasse ist
                            if (spielfeld[feldNummer].typ == STRASSE)
                            {
                                setHaus(spielfeld[feldNummer].hausnummer,0);
                            }
                            flagFeldBelastet = 0; //flag auf 0 setzen um mit nächtem Feld weiterzufahren
                        }
                        
                    }
                    else if (positiveFlanke & TASTE_U)
                    {
                        hypothekAufloesen = (hypothekAufloesen + 1) % 2;
                        if (hypothekAufloesen)
                        {
                            writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                        }
                        else
                        {
                            writeText(1,0,"Hyp. Behalten  "PFEIL_O);
                        }
                    }
                }
            }
            //FREIKARTE NICHT VERGESSEN
            //prüft ob freikarten übertragen werden müssen
            if (spielerInfo[zahler].freikarte)
            {
                //Überträgt alle freikarten
                spielerInfo[empfaenger].freikarte += spielerInfo[zahler].freikarte;
                //Löscht die Freikarten aus dem Inventar des Spielers
                spielerInfo[zahler].freikarte = 0;
            }
            pleiteZustand = ENDE_VERSTEIGERUNG;
            break;
            default:
            break;
        }
    }
}

void hausBauen(uint8_t spielerAmZug)
{
    uint8_t farbgruppenCounter = 0;
    uint8_t updateLCD = 0;
    uint8_t minHaeuser = 5;
    uint8_t maxHaeuser = 0;
    uint8_t volleFarbgruppen[8] = {0};
    uint8_t haeuser = 0;
    uint8_t gruppeAnzahlHaeuser = 0;
    uint8_t flagFarbgruppeKomplett = 0;
    uint8_t flagBauErfolgreich = 0;
    uint8_t feldNummer = 0;
    uint8_t farbgruppenErstesFeld[8] = {1,6,11,16,21,26,31,37}; //Jeweil das erste Feld einer Strassen Farbgruppe
    uint8_t anzahlHauser[3] = {0};
    uint8_t flagZurueck = 0;
    //Felder nach vollen Farbgruppen absuchen~~~~~~~~~~~~~~~~~~~~~~~~~
    for (uint8_t i = 0; i < 8; i = i + 1)//Alle Farbgruppen werden als voll markiert
    {
        volleFarbgruppen[i] = 1;
    }
    for (uint8_t i = 0; i < 3; i = i + 1)//Alle Farbgruppen werden als voll markiert
    {
        anzahlHauser[i] = 0;
    }
    for (uint8_t i = 0;  i < 8; i = i + 1)
    {
        //Flankenerkennung
        tasteAlt = tasteNeu;
        tasteNeu = 0;
        tasteNeu = (PINL << 8) | PINK;
        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        flagFarbgruppeKomplett = 1;
        if (spielfeld[farbgruppenErstesFeld[i]].besitzer == spielerAmZug) //überprüft ob erstes feld einer Farbgruppe dem Spieler gehört
        {
            for (uint8_t j = 0; j < 3; j = j + 1)//Prüft ob Farbgruppen tatsächlich voll sind
            {
                //besitzer des spielfeldes mit dem spieler am zug vergleichen                                             Was dieser teil macht weiss ich grade auch nicht mehr         Dieser Teil prüft ob ein Feld der Farbgruppe belastet ist. Wenn ja, kann man nicht bauen
                if (((!(spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].besitzer == spielerAmZug)) && (spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j])) || spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].feldBelastet)//prüft alle Felder der Farbgruppe
                {
                    flagFarbgruppeKomplett = 0; //setzt flag auf 0 wenn ein Feld nicht dem Spieler gehört
                    volleFarbgruppen[i] = 0; //Markiert Farbgruppe als unvollständig
                }
            }
            if (flagFarbgruppeKomplett)//Wenn farbgruppe komplett
            {
                updateLCD = 0;
                //nächstes Feld
                feldNummer = farbgruppenErstesFeld[i];
                //flagZurück wird verwendet, wenn Geld aufgetrieben werden muss.
                //Es wird gesetzt, wenn ein Haus verkauft wurde um die while
                //schleife zu verlassen
                while (!(positiveFlanke & TASTE_C) && ! flagZurueck)
                {
                    //Flankenerkennung
                    tasteAlt = tasteNeu;
                    tasteNeu = 0;
                    tasteNeu = (PINL << 8) | PINK;
                    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                    minHaeuser = 255;
                    maxHaeuser = 0;
                    for (uint8_t j = 0; j < 3; j = j + 1)
                    {
                        if (spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j] > 0)
                        {
                            haeuser = spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].anzahlHaeuser;
                            if ((haeuser < minHaeuser))
                            {
                                minHaeuser = haeuser;
                            }
                            if (haeuser > maxHaeuser)
                            {
                                maxHaeuser = haeuser;
                            }
                        }
                    }
                    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                    for (uint8_t j = 0; j < 3; j = j + 1)
                    {
                        haeuser = spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].anzahlHaeuser;
                        if (haeuser == maxHaeuser)
                        {
                            anzahlHauser[j] = 1;
                        }
                        //else if (spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j])
                        else if (haeuser == minHaeuser)
                        {
                            anzahlHauser[j] = 0;
                        }
                        else
                        {
                            anzahlHauser[j] = 1;
                        }
                    }
                            
                            
                    if (!updateLCD)
                    {
                        clear();//lcd leeren
                        writeText(1,0,spielfeld[feldNummer].name);
                        //LCD ausgabe abhängig ob der Spieler schulden hat oder nicht
                        if (flagGeldBeschaffen)
                        {
                            writeText(0,0,PFEIL_U"Abbauen        ");
                            writeText(2,0,"         weiter"PFEIL_R);
                        }
                        else
                        {
                            writeText(0,0,PFEIL_U"Abbauen  Bauen"PFEIL_O);
                            writeText(2,0,"C zur"UE"ck|weiter"PFEIL_R); 
                        }
                        /*writeText(1,0," w"UE"rfeln A / B ");
                        writeText(2,0,"    weiter C    ");*/
                        gruppeAnzahlHaeuser = spielfeld[farbgruppenErstesFeld[i]].anzahlHaeuser; //holt die Anzahl Häuser
                        updateLCD = 1;
                    }
                    if ((positiveFlanke & TASTE_O) && !flagGeldBeschaffen)//Haus Bauen nur möglich wenn der Spieler nicht verschuldet ist
                    {
                        //Wenn auf allen felder gebaut wurde, flags zurücksetzten
                        if (anzahlHauser[0] && anzahlHauser[1] && anzahlHauser[2])
                        {
                            anzahlHauser[0] = 0;
                            anzahlHauser[1] = 0;
                            anzahlHauser[2] = 0;
                        }
                        switch (farbgruppenCounter)
                        {
                            case 0:
                            if (!anzahlHauser[0])
                            {
                                flagBauErfolgreich = bauen(feldNummer, spielerAmZug);
                                if (flagBauErfolgreich)
                                {
                                    anzahlHauser[0] = 1;
                                }
                                        
                            }
                            break;
                            case 1:
                            if (!anzahlHauser[1])
                            {
                                flagBauErfolgreich = bauen(feldNummer, spielerAmZug);
                                if (flagBauErfolgreich)
                                {
                                    anzahlHauser[1] = 1;
                                }
                            }
                            break;
                            case 2:
                            if (!anzahlHauser[2])
                            {
                                flagBauErfolgreich = bauen(feldNummer, spielerAmZug);
                                if (flagBauErfolgreich)
                                {
                                    anzahlHauser[2] = 1;
                                }
                            }
                            break;
                        }
                                
                    }
                    if ((positiveFlanke & TASTE_U))//Haus Verkaufen~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                    {
                        //Wenn auf allen felder gebaut wurde, flags zurücksetzten
                        if (!(anzahlHauser[0] || anzahlHauser[1] || anzahlHauser[2]))
                        {
                            anzahlHauser[0] = 1;
                            anzahlHauser[1] = 1;
                            anzahlHauser[2] = 1;
                        }
                        switch (farbgruppenCounter)
                        {
                            case 0:
                            if (anzahlHauser[0])
                            {
                                flagBauErfolgreich = abBauen(feldNummer, spielerAmZug);
                                if (flagBauErfolgreich)
                                {
                                    anzahlHauser[0] = 0;
                                } 
                            }
                            break;
                            case 1:
                            if (anzahlHauser[1])
                            {
                                flagBauErfolgreich = abBauen(feldNummer, spielerAmZug);
                                if (flagBauErfolgreich)
                                {
                                    anzahlHauser[1] = 0;
                                }
                            }
                            break;
                            case 2:
                            if (anzahlHauser[2])
                            {
                                flagBauErfolgreich = abBauen(feldNummer, spielerAmZug);
                                if (flagBauErfolgreich)
                                {
                                    anzahlHauser[2] = 0;
                                }
                            }
                            break;
                        }
                                
                    }
                    //Wenn Geld bschafft werden muss, darf nur 1 Haus aufs mal verkauft werden
                    //Wenn flagGeldBeschaffen gesetzt ist und ein Haus abgebaut wurde
                    if (flagGeldBeschaffen && flagBauErfolgreich)
                    {
                        //BauModus Verlassen
                        flagZurueck = 1;
                    }
                    
                    if (positiveFlanke & TASTE_R)//nächstes Feld
                    {
                        farbgruppenCounter = (farbgruppenCounter + 1) % 3;
                        feldNummer = spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[farbgruppenCounter];
                        if (!feldNummer)//Sonderfall bei Farbgruppen mit nur 2 Feldern
                        {
                            farbgruppenCounter = 0;
                            feldNummer = spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[farbgruppenCounter];
                            anzahlHauser[2] = anzahlHauser[1];
                        }
                        updateLCD = 0;
                    }
                }
                flagZurueck = 0;
                        
            }
                    
        }
    }

}
