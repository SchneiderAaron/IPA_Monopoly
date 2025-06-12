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
* Dateiname: main.c
*
* Projekt  : IPA_Monopoly_v2
* Hardware : Monopoly-Board, ATmega2560v von Atmel
*
*
* Copyright: MSW, AE3
*
* Beschreibung:
* =============
* Monopoly software IPA 2025 Aaron Schneider
*
* Verlauf:
* ========
* Datum:      Autor:         Version   Grund der Änderung:
* 22.04.2025  A.Schneider    V2.0      Nacharbeit
*
\*********************************************************************************/



//Standardisierte Datentypen
#include <stdint.h>
//ATmega2560v I/O-Definitionen
#include <avr/io.h>

#define F_CPU 16000000UL //CPU Clock Definition
#define __DELAY_BACKWARD_COMPATIBLE__ //Erlaupt es Variablen in delays zu verwenden
#include <util/delay.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include <avr/interrupt.h>

#include "SPI.h"
#include "MonopolyTreiber.h"
#include "LCD.h"

#pragma GCC optimize 0
/*--- #define-Konstanten und Makros -----------------------------------------*/
#define MIN_ANZAHL_SPIELER  2
#define MAX_ANZAHL_SPIELER  4

#define SIEBENSEGMENT_ON    0
#define SIEBENSEGMENT_OFF   10

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




//#define UMLAUT_U "\u00DC"
//#define UMLAUT_U "š"

#define MIN_ANZAHL_HAEUSER 0
#define MAX_ANZAHL_HAEUSER 5



#define ZAHLUNG_ERFOLGREICH 1
#define ZAHLUNG_FEHLGESCHLAGEN 2

#define ANZAHL_FELDER 40
#define STARTGELD 1500
#define RUNDEN_GELD 200

#define WUERFEL_A 1
#define WUERFEL_B 2

#define ANZAHL_EREIGNISS_LCD_TEXT_ZEILEN 2

#define FELD_KAUFEN_KEIN_SPIELER_INPUT 0
#define FELD_KAUFEN_FELD_GEKAUFT 1
#define FELD_KAUFEN_FELD_NICHT_GEKAUFT 2

#define POS_HALTESTELLE4 35
#define POS_HALTESTELLE1 5
#define HALTESTELLEN_ABSTAND 10

#define FELDNUMMER_WERK1 12
#define FELDNUMMER_WERK2 28
#define MULTIPLIKATOR_WERK_EINZELN 4
#define MULTIPLIKATOR_WERK_GRUPPE 10

#define BLAULICHT_LED_1 0x40
#define BLAULICHT_LED_2 0x80
#define BLAULICHT_LED_AUS ~0xC0
#define HOECHSTBIETER 5
#define ANZAHL_ZURUECKGETRETENE_SPIELER 4
#define SPIELER_INVENTAR_GROESSE 28
#define ANZAHL_FELDER_IN_FARBGRUPPE 3
#define MARKIERUNG_STRASSE_BELASTET 6
#define MARKIERUNG_RGB_FELD_BELASTET 5
#define ANZAHL_HAENDLER 2
/*--- Datentypen (typedef) --------------------------------------------------*/


typedef enum {SPIELERAUSWAHL, WUERFELSTART, SPIEL, VERSTEIGERUNG, BAUEN, VERWALTEN, VERPFAENDEN, HANDELN} zustand_t;
typedef enum {FREIKARTE_J_N, PASCH_J_N, BEZAHLEN_J_N, PASCH} workshopZustand_t;
typedef enum {HYPOTHEK, VERWALTUNG_BAUEN, VERWALTUNG_HANDELN} verwaltung_t;
typedef enum {HAENDLER_AUSWAHL, WARE_AUSWAEHLEN, HANDEL_BESTAETIGEN, BESITZ_UEBERTRAGEN, HANDEL_ABSCHLIESSEN} handel_t;
typedef enum {GRUNDSTUECK, BARGELD, FREIKARTEN, AUSWAHL_BEENDEN} handelWare_t;


/*--- Globale Konstanten ----------------------------------------------------*/

/*--- Globale Variablen -----------------------------------------------------*/

//uint8_t houses[14][8] = {0};
uint8_t hausRegister[14] = {0};
uint8_t spieler[20][8] = {0};
uint8_t spielerPos[4] = {0};

uint8_t siebensegment[16] = {0};
uint8_t wuerfelArray[2] = {0};
    
uint8_t spielerImGefaengnis[5] = {0};

uint16_t tasteAlt       = 0; //Variabeln Flankenerkennung
uint16_t tasteNeu       = 0;
uint16_t positiveFlanke = 0;
uint16_t negativeFlanke = 0;

zustand_t zustand = SPIELERAUSWAHL;//Spielzustand auf SPIELERAUSWAHL setzen
workshopZustand_t workshopZustand = PASCH_J_N;//Workshopzustand auf PASH_J_N setzen
verwaltung_t verwaltung = VERWALTUNG_BAUEN;//Verwaltung auf VERWALTUNG_BAUEN Setzen

uint8_t anzahlSpieler = 2;//Anzahl Spieler auf 2 setzen NORMALERWEISE 2

uint8_t globalUpdateLCD = 0;

Feld spielfeld[40];
Karte chanceKanzlei[34];
handelInventar handel[2];
handelWare_t handelware = GRUNDSTUECK;//Handelware auf GRUNDSTUEK setzen
uint8_t haeuserImSpiel = 0;
uint8_t hotelsImSpiel = 0;

uint8_t flagWuerfel1 = 0;
uint8_t flagWuerfel2 = 0;
uint8_t flagGeldBeschaffen = 0;

uint32_t getSystemzeit(void);

volatile uint32_t millis;

uint32_t startZeit = 0;
uint32_t systemZeit = 0;

extern const char kartenArray[][200];
void initSpieler(Spieler spielerInfo[])
{
    //Eigenschaften Spieler 1
    strcpy(spielerInfo[1].name, "Spieler 1");
    spielerInfo[1].geld = 1111;
    spielerInfo[1].position = 40;
    spielerInfo[1].gefaengnis = 0;
    spielerInfo[1].rundenImGefaengnis = 0;
    spielerInfo[1].freikarte = 0;
    spielerInfo[1].haeuser = 0;
    spielerInfo[1].hotels = 0;
    spielerInfo[1].pleite = 0;
    
    //Eigenschaften Spieler 2
    strcpy(spielerInfo[2].name, "Spieler 2");
    spielerInfo[2].geld = 2222;
    spielerInfo[2].position = 40;
    spielerInfo[2].gefaengnis = 0;
    spielerInfo[2].rundenImGefaengnis = 0;
    spielerInfo[2].freikarte = 0;
    spielerInfo[2].haeuser = 0;
    spielerInfo[2].hotels = 0;
    spielerInfo[2].pleite = 0;
    
    //Eigenschaften Spieler 3
    strcpy(spielerInfo[3].name, "Spieler 3");
    spielerInfo[3].geld = 3333;
    spielerInfo[3].position = 40;
    spielerInfo[3].gefaengnis = 0;
    spielerInfo[3].rundenImGefaengnis = 0;
    spielerInfo[3].freikarte = 0;
    spielerInfo[3].haeuser = 0;
    spielerInfo[3].hotels = 0;
    spielerInfo[3].pleite = 0;
    
    //Eigenschaften Spieler 4
    strcpy(spielerInfo[4].name, "Spieler 4");
    spielerInfo[4].geld = 4444;
    spielerInfo[4].position = 40;
    spielerInfo[4].gefaengnis = 0;
    spielerInfo[4].rundenImGefaengnis = 0;
    spielerInfo[4].freikarte = 0;
    spielerInfo[4].haeuser = 0;
    spielerInfo[4].hotels = 0;
    spielerInfo[4].pleite = 0;
}
Spieler spielerInfo[5];



uint8_t handelbareFelder[28] = {0};
uint8_t flagHandelbar = 0;
uint8_t anzahlHandelbareFelder = 0;
uint8_t anzahlAusgewaehlteFelder = 0;
uint8_t handelFeld = 0;
uint8_t handelUpdateLCD = 0;


uint8_t xTasten[4] = {TASTE_X1, TASTE_X2, TASTE_X3, TASTE_X4};
uint8_t yTasten[4] = {TASTE_Y1, TASTE_Y2, TASTE_Y3, TASTE_Y4};


#define TIMER1_PRESCALER 1024



uint8_t flagSpielerPleite = 0;
uint8_t flagFertigGewuerfelt = 0;

uint8_t letzterWuerfel = 0;

int main(void)
{
    
    //zufallsgeneratorAuswertung();
    //char buffer[200];  // Buffer im RAM
    
    
    /*--- Modullokale Konstanten ------------------------------------------------*/
    /*--- Modullokale Variablen -------------------------------------------------*/
    //char
    char lcdBuffer[16];
    //8-Bit Variabeln
    uint8_t spielerAmZug = 1;
    //uint8_t flagNextPlayer = 0;
    uint8_t paschZaehler = 0;
    uint8_t flagWeiter = 1;
    uint8_t aktuellePosition = 0;
    
    uint8_t flagTasteX = 0;
    uint8_t flagTasteY = 0;
    uint8_t bieter[6] = {0};//0-3 Bieter 4 anz. spieler raus 5 höchstbieter
    uint8_t ersterSpieler = 0;
    
    
    
    uint8_t spielerSetup = 0;
    
    
    
    //uint8_t flagSchulden = 0;
    
    //uint8_t flagVersteigert = 0;
    uint8_t verkaufSpielerEingabe = 0;
    
    uint8_t updateLCD = 0;
    uint8_t flagSpielLCD = 0;
    
    uint8_t feldBesitzer = 0;
    uint8_t bezahlStatus = 0;
    uint8_t flagZahlungAbgeschlossen = 1;//flagZahlungAbgeschlossen auf 1 setzen
    uint8_t flagKaufAbgechlossen = 1;
    
    
    
    
    //uint8_t farbgruppenCounter = 0;
    
    
    //uint8_t flagHausFeld1 = 0;
    //uint8_t flagHausFeld2 = 0;
    //uint8_t flagHausFeld3 = 0;
    
    

    
    uint8_t haltestelleFarbgruppe = 0;
    uint8_t multiplikator = 0;
    
    uint8_t ereignisSchritt = 0;
    uint8_t flagEreignisWeiter = 0;
    uint8_t flagEreignisAbgeschlossen = 1;
    uint8_t ereignisfeldRueckgabe = 0;
    uint8_t flagKanzlei = 0;
    
    //uint8_t kaufStatus = 0;
    uint16_t zahlBetrag = 0;
    uint8_t flagMieteFarbgruppe = 0;
    uint8_t zahlBetragFarbgruppe = 0;
    //uint8_t zahlSchritt = 0;
    
    uint8_t anzahlEigentum = 0;
    uint8_t feldZaehler = 0;
    uint8_t hausNummer = 0;
    uint8_t rgbFeldNummer = 0;
    
    uint8_t feldbelastet = 0;
    
    //16-Bit Variabeln
    uint16_t geldZwischenspeicher[5] = {0};
    uint16_t aktuellesGebot = 0;
    //Eigene Datentypen
    
    FeldTyp aktuellesFeld = FREIPARKEN;
    handel_t handelZustand = HAENDLER_AUSWAHL;
    uint8_t flagGefaengnis = 0;
    uint8_t flagGefaengnisLCD = 0;
    uint8_t flagGefaengnisWeiter = 0;
    
    
    uint8_t spielerInventar[28] = {0};
    uint8_t flagKeineHaeuser = 0;
    
    uint8_t haendlerZaehler = 0;
    //uint8_t haendlerAmZug = 0;
    uint8_t auswahlAbgeschlossen = 0;
    uint8_t handelSpielerNummern[2] = {0};
    uint8_t handelFeldNummer = 0;
    uint8_t flagHandelBelastet = 0;
    uint8_t handelHypothek = 0;
    uint8_t handelBezahlBetrag = 0;
    uint8_t handelzahlungErfolgreich = 0;
    /*--- Prototypen modullokaler Funktionen ------------------------------------*/
    uint8_t feldKaufen(uint8_t feldNummer, Feld spielfeld[40], uint8_t spielerAmZug);
    
    void warteBisGewuerfelt(void);
    uint8_t handelWareAuswaehlen(uint8_t haendlerNr);
    uint8_t auswahlBestaetigen(uint8_t haendlerNr);
    
    uint32_t seed = 0;
    uint32_t adcWert = 0;
    /*--- Funktionsdefinitionen -------------------------------------------------*/
    
    
    //Initialisierung
    PortInitialisierung();//ports initialisieren während vorbereitung erstellt!!
    lcdInitAll();//lcd Initialisieren während vorbereitung erstellt!!
    //initialisiereSpielfeld(spielfeld);//spielfelder Initialisieren
    initialisiereSpielfeld(spielfeld);//spielfelder Initialisieren
    initSpieler(spielerInfo);
    initialisiereKarten(chanceKanzlei);
    initialisiereHandelInventar(handel);//initialisiert das handelinventar
    SPI_init_all(9600);//SPI initialisieren während vorbereitung erstellt!!
    resetMonopoly();
    //random Seed setzen
    //ADC initialisieren
    adm_ADC_init();//ADC Initialisieren während vorbereitung erstellt!!
    //Configuration of Timer1
    OCR1A = 999;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11);
    TIMSK1 |= (1 << OCIE1A);
    sei();
    
    //Random Seed generieren
    //Es werden 3 adcwerte eingelesen und miteinander verknüpft um einen Seed zu erstellen
    for (uint8_t i = 0; i < 4; i = i + 1)
    {
        _delay_us(500);
        adcWert = adm_ADC_read(i);
        seed ^= adcWert << 22;
        seed ^= ~adcWert << 12;
        seed ^= adcWert << 2;
    }
    //Seed setzen
    srand(seed);
    //Seed auf LCD ausgeben, 1s anzeigen
    writeText(0,0,"      seed      ");
    sprintf(lcdBuffer,"%lu",seed);
    writeText(1,3,lcdBuffer);
    _delay_ms(1000);
    //for schleife setzt die RGB auf jedem spielfeld
    for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
    {
        if (spielfeld[i].besitzer)
        {
            setPropertyRgb(spielfeld[i].rgbNummer,spielfeld[i].besitzer);
            setHaus(spielfeld[i].hausnummer,spielfeld[i].anzahlHaeuser);
        }
    }
    //uint8_t textCounter = 0;
    //uint8_t stringCounter = 0;
    //uint8_t flagEreignisfeld = 0;
    //_delay_ms(1000);
    uint8_t startFlag = 0;
    uint8_t zufallsSpieler = 0;
    uint8_t zufallsFeld = 0;
    writeText(0,0,"      MSW       ");
    writeText(1,0,"    MONOPOLY    ");
    writeText(2,0,"Taste A = Start ");
    while(1)
    {
        /*for (uint8_t i = 1; i <= 4; i = i + 1)
        {
            for(uint8_t j = 0; j < 40; j = j + 1)
            {
                //setPropertyRgb(spielfeld[j].rgbNummer,i);
                wuerfelTransmit((rand() % 6) + 1, (rand() % 6) + 1);
                setzeSpielerPositionAnimation(j,i,1,0);
                setzeSpielerPositionAnimation(j,i,1,1);
                for (uint8_t k = 1; k <= 4; k = k + 1)
                {
                    setGeld((rand() % 10000),k,1);
                    startFlag = animationAbbrechen(startFlag);
                    if (startFlag)
                    {
                        break;
                    }
                }
                startFlag = animationAbbrechen(startFlag);
                if (startFlag)
                {
                    break;
                }
            }
            startFlag = animationAbbrechen(startFlag);
            if (startFlag)
            {
                break;
            }
        }*/
        
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~START ANIMATION~~~~~~~~~~~~~~~~~~~~~~
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~START ANIMATION~~~~~~~~~~~~~~~~~~~~~~
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~START ANIMATION~~~~~~~~~~~~~~~~~~~~~~
        //Schleife um alle Spieler Position LEDs ein zu schalten
        //4 Farben x 40 Felder = 160
        for (uint8_t i = 0; i < 160; i = i + 1)
        {
            //Zufälliges Feld wählen
            zufallsFeld = rand() % 40;
            //Zufällige Spielernummer Wählen
            zufallsSpieler = (rand() % 4) + 1;
            //wenn die LED auf dem feld noch nicht leuchtet
            if (!pruefeSpielerPosition(zufallsFeld,zufallsSpieler))
            {
                //LED auf zufälligem Feld auf 1 setzen
                setzeSpielerPositionAnimation(zufallsFeld,zufallsSpieler,1,0);
                //Ausgabe tätigen
                setzeSpielerPositionAnimation(0,0,0,1);
                //_delay_ms(5);
                //Zufällige Spielernummer auf dem Würfel Siebensegmenten anzeigen
                wuerfelTransmit(zufallsSpieler,zufallsSpieler);
                //Alle Konto Siebensegmente durchlaufen
                for (uint8_t j = 1; j <= 4; j = j + 1)
                {
                    //Anzahl gesetzte LEDs auf Konto SIebensegmenten anzeigen
                    setGeld(i,j,1);
                    //startFlag setzen, wenn eine Taste betätigt wurde
                    startFlag = animationAbbrechen(startFlag);
                    //Wenn Startflag gesetzt ist, dann Schleife verlassen
                    if (startFlag)
                    {
                        break;
                    }
                }
            }
            //Wenn die Zufällig gewählte LED bereits gesetzt ist
            //wird i um 1 verkleinert und somit der Schritt wiederholt
            else
            {
                //i um 1 verkleinern
                i -= 1;
            }
            //startFlag setzen, wenn eine Taste betätigt wurde
            startFlag = animationAbbrechen(startFlag);
            //Wenn Startflag gesetzt ist, dann Schleife verlassen
            if (startFlag)
            {
                break;
            }
        }
        //Schleife um alle Spieler Position LEDs aus zu schalten
        //4 Farben x 40 Felder = 160
        for (uint8_t i = 0; i < 160; i = i + 1)
        {
            //Zufälliges Feld wählen
            zufallsFeld = rand() % 40;
            //Zufällige Spielernummer Wählen
            zufallsSpieler = (rand() % 4) + 1;
            //Wenn Zufällige LED gesetzt ist
            if (pruefeSpielerPosition(zufallsFeld,zufallsSpieler))
            {
                //Zufällige LED auf 0 Setzen
                setzeSpielerPositionAnimation(zufallsFeld,zufallsSpieler,0,0);
                //Ausgabe tätigen
                setzeSpielerPositionAnimation(0,0,0,1);
                //_delay_ms(5);
                wuerfelTransmit(zufallsSpieler,zufallsSpieler);
                //Alle Konto Siebensegmente durchlaufen
                for (uint8_t j = 1; j <= 4; j = j + 1)
                {
                    //Anzahl leuchtende LEDs an Konto Siebensegmenten anzeigen
                    setGeld(160 - i,j,1);
                    //startFlag setzen, wenn eine Taste betätigt wurde
                    startFlag = animationAbbrechen(startFlag);
                    //Wenn startFlag gesetzt ist, dann Schleife verlassen
                    if (startFlag)
                    {
                        break;
                    }
                }
            }
            //Wenn die Zufällige LED bereits ausgeschaltet wurde
            //i um 1 verkleinern. Somit wird der Schritt wiederholt
            else
            {
                i -= 1;
            }
            //startFlag setzen, wenn eine Taste betätigt wurde
            startFlag = animationAbbrechen(startFlag);
            //Wenn startFlag gesetzt ist, dann Schleife verlassen
            if (startFlag)
            {
                break;
            }
        }
        //startFlag setzen, wenn eine Taste betätigt wurde
        startFlag = animationAbbrechen(startFlag);
        //Wenn startFlag gesetzt ist, dann Schleife verlassen
        if (startFlag)
        {
            break;
        }
        //Würfel Siebensegmente ausschalten
        wuerfelTransmit(10,10);
        
        //Alle Konto Siebensegmente ausschalten
        setGeld(0,1,0);
        setGeld(0,2,0);
        setGeld(0,3,0);
        setGeld(0,4,0);
        /*for (uint8_t i = 1; i <= 4; i = i + 1)
        {
            for (uint8_t j = 0; j <= 40; j = j + 1)
            {
                setzeSpielerPositionAnimation(40 - j,i,0,0);
                setzeSpielerPositionAnimation(0,0,0,1);
                startFlag = animationAbbrechen(startFlag);
                if (startFlag)
                {
                    break;
                }
            }
            startFlag = animationAbbrechen(startFlag);
            if (startFlag)
            {
                break;
            }
        }*/
        //vier mal durchlaufen
        for(uint8_t h = 1; h <= 4; h = h + 1)
        {
            //Wenn "h"ungerade ist soll eingeschalten werden
            if (h % 2)
            {
                //alle Häuser durchlaufen
                for (uint8_t i = 1; i <= 5; i = i + 1)
                {
                    //Alle Felder durchlaufen
                    for (uint8_t j = 0; j < 40; j = j + 1)
                    {
                        //Häuser auf dem Aktuellen Feld auf den wert i setzen
                        setHausAnimation(spielfeld[j].hausnummer,i,0);
                        //startFlag setzen, wenn eine Taste betätigt wurde
                        startFlag = animationAbbrechen(startFlag);
                        //Wenn startFlag gesetzt ist, dann Schleife verlassen
                        if (startFlag)
                        {
                            break;
                        }
                    }
                    //Häuser ausgeben
                    setHausAnimation(0,0,1);
                    //30ms Delay
                    _delay_ms(30);
                    //startFlag setzen, wenn eine Taste betätigt wurde
                    startFlag = animationAbbrechen(startFlag);
                    //Wenn startFlag gesetzt ist, dann Schleife verlassen
                    if (startFlag)
                    {
                        break;
                    }
                }
                //50ms Delay
                _delay_ms(50);
                //startFlag setzen, wenn eine Taste betätigt wurde
                startFlag = animationAbbrechen(startFlag);
                //Wenn startFlag gesetzt ist, dann Schleife verlassen
                if (startFlag)
                {
                    break;
                }
            }
            //bei geradem "h" soll ausgeschalten werden
            else
            {
                //Alle Häuser durchlaufen
                for (uint8_t i = 1; i <= 5; i = i + 1)
                {
                    //alle Felder durchlaufen
                    for (uint8_t j = 0; j < 40; j = j + 1)
                    {
                        //Anzahl Häuser auf feld i um 1 verkleinern
                        setHausAnimation(spielfeld[j].hausnummer,5 - i,0);
                        //startFlag setzen, wenn eine Taste betätigt wurde
                        startFlag = animationAbbrechen(startFlag);
                        //Wenn startFlag gesetzt ist, dann Schleife verlassen
                        if (startFlag)
                        {
                            break;
                        }
                    }
                    //Häuser ausgeben
                    setHausAnimation(0,0,1);
                    //30ms delay
                    _delay_ms(30);
                    //startFlag setzen, wenn eine Taste betätigt wurde
                    startFlag = animationAbbrechen(startFlag);
                    //Wenn startFlag gesetzt ist, dann Schleife verlassen
                    if (startFlag)
                    {
                        break;
                    }
                }
                //50ms delay
                _delay_ms(50);
            }
            
        }
        //Hälfte der Spielfelder durchlaufen
        for (uint8_t i = 0; i <= 20; i = i + 1)
        {
            //Alle Spieler durchlaufen
            for(uint8_t j = 1; j <= 4; j = j + 1)
            {
                //die gesetzten Felder auf Kontosiebensegmenten anzeigen
                setGeld(i,j,1);
                //startFlag setzen, wenn eine Taste betätigt wurde
                startFlag = animationAbbrechen(startFlag);
                //Wenn startFlag gesetzt ist, dann Schleife verlassen
                if (startFlag)
                {
                    break;
                }
            }
            for (uint8_t j = 0; j <= 20 - i; j = j + 1)
            {
                //letzte Position löschen
                if (j > 0)
                {
                    //letzte Position wieder zurücksetzen
                    //Rot, Blau
                    setzeSpielerPositionAnimation(j - 1,1,0,0);
                    setzeSpielerPositionAnimation((40 - j) + 1,4,0,0);
                    //Grün, Gelb
                    setzeSpielerPositionAnimation((20 - j) + 1,2,0,0);
                    setzeSpielerPositionAnimation((20 + j) - 1,3,0,0);
                }
                //Neue Position setzen
                //Rot, Blau
                setzeSpielerPositionAnimation(j,1,1,0);
                if (!j)
                {
                    setzeSpielerPositionAnimation(0,4,1,0);
                }
                else
                {
                    setzeSpielerPositionAnimation(40 - j,4,1,0);
                }
                if(j == 1)
                {
                    setzeSpielerPositionAnimation(0,4,0,0);
                }
                
                
                //Grün, Gelb
                setzeSpielerPositionAnimation(20 - j,2,1,0);
                setzeSpielerPositionAnimation(20 + j,3,1,0);
                
                if(!j)
                {
                    //Korrektur Gelb auf Feld 0
                    setzeSpielerPositionAnimation(0,3,1,0);
                }
                if (i == 20)
                {
                    //Korrektur Blau auf Feld 0
                    setzeSpielerPositionAnimation(0,4,1,0);
                }
                
                //Spielerposition ausgeben
                setzeSpielerPositionAnimation(0,0,1,1);
                //10ms Delay
                _delay_ms(10);
                //startFlag setzen, wenn eine Taste betätigt wurde
                startFlag = animationAbbrechen(startFlag);
                //Wenn startFlag gesetzt ist, dann Schleife verlassen
                if (startFlag)
                {
                    break;
                }
            
            }
            //25ms delay
            _delay_ms(25);
            //startFlag setzen, wenn eine Taste betätigt wurde
            startFlag = animationAbbrechen(startFlag);
            //Wenn startFlag gesetzt ist, dann Schleife verlassen
            if (startFlag)
            {
                break;
            }
        }
        //Hälfte der Spielfelder durchlaufen
        for (uint8_t j = 0; j <= 20; j = j + 1)
        {
            //Alle Spieler durchlaufen
            for(uint8_t k = 1; k <= 4; k = k + 1)
            {
                //Die gesetzten Felder auf Kontosiebensegmenten anzeigen
                setGeld(20 - j,k,1);
                //startFlag setzen, wenn eine Taste betätigt wurde
                startFlag = animationAbbrechen(startFlag);
                //Wenn startFlag gesetzt ist, dann Schleife verlassen
                if (startFlag)
                {
                    break;
                }
            }
            //neue Position setzen
            //Grün, Gelb
            setzeSpielerPositionAnimation(j,2,0,0);
            setzeSpielerPositionAnimation(40 - j,3,0,0);
            
            //Rot, Blau
            setzeSpielerPositionAnimation(20 - j,1,0,0);
            setzeSpielerPositionAnimation(20 + j,4,0,0);
            
            if (!j)
            {
                //Gelb ausschalten bei Feld 0
                setzeSpielerPositionAnimation(0,3,0,0);
            }
            if (j == 20)
            {
                //Blau ausschalten bei Feld 0
                setzeSpielerPositionAnimation(0,4,0,0);
            }
            //Spielerposition ausgeben
            setzeSpielerPositionAnimation(0,0,1,1);
            //35ms delay
            _delay_ms(35);
            //startFlag setzen, wenn eine Taste betätigt wurde
            startFlag = animationAbbrechen(startFlag);
            //Wenn startFlag gesetzt ist, dann Schleife verlassen
            if (startFlag)
            {
                break;
            }
            
        }
        //100ms delay
        _delay_ms(100);
        //startFlag setzen, wenn eine Taste betätigt wurde
        startFlag = animationAbbrechen(startFlag);
        //Wenn startFlag gesetzt ist, dann Schleife verlassen
        if (startFlag)
        {
            break;
        }
        
    }
    //alle Felder durchlaufen
    for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
    {
        //Alle Spieler durchlaufen
        for (uint8_t j = 1; j <= 4; j = j + 1)
        {
            //alle Häuser ausschalten
            setHausAnimation(spielfeld[i].hausnummer,0,0);
            //Alle Spieler positionen ausschalten
            setzeSpielerPositionAnimation(i,j,0,0);
        }
        //Häuser ausgeben
        setHausAnimation(0,0,1);
        //Spieler position ausgeben
        setzeSpielerPositionAnimation(0,0,0,1); 
    }
    //Würfel Siebensegmente ausschalten
    wuerfelTransmit(10,10);
    //LCD Leeren
    writeText(0,0,"                ");
    writeText(1,0,"                ");
    writeText(2,0,"                ");
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~START ANIMATION~~~~~~~~~~~~~~~~~~~~~~
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~START ANIMATION~~~~~~~~~~~~~~~~~~~~~~
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~START ANIMATION~~~~~~~~~~~~~~~~~~~~~~
    
    
    
    //Spiel Starten
    while (1) 
    {
        //Flankenerkennung
        tasteAlt = tasteNeu;
        tasteNeu = 0;
        tasteNeu = (PINL << 8) | PINK;
        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        
        
        /*if (positiveFlanke & TASTE_O)//Wenn Taste runter gedrückt wird => Bauen
        {
            zustand = BAUEN;
        }*/
        
        //falls der Kontrast am LCD nicht mehr stimmt
        //kann man während dem Spiel das LCD neu initialisieren
        //dafür müssen alle 4 Y Tasten gleichzeitig gedrückt werden
        if (((PINL << 8) | PINK) == (TASTE_Y1 | TASTE_Y2 | TASTE_Y3 | TASTE_Y4))
        {
            DDRC = 0xFF;		// Port C auf Ausgang initialisieren (alle Pins)
            PORTC = 0x3F;
            DDRD = 0xFF;		// Port D auf Ausgang initialisieren (alle Pins)
            PORTD = 0x00;
            lcdReInit();
            //seed neu setzen
            for (uint8_t i = 0; i < 4; i = i + 1)
            {
                _delay_us(500);
                adcWert = adm_ADC_read(i);
                seed ^= adcWert << 22;
                seed ^= ~adcWert << 12;
                seed ^= adcWert << 2;
            }
            srand(seed);
            writeText(0,0,"      seed      ");
            sprintf(lcdBuffer,"%lu",seed);
            writeText(1,3,lcdBuffer);
            startZeit = getSystemzeit();

        }
        //Prüft ob der Spieler am zug pleite ist
        if (spielerInfo[spielerAmZug].pleite)
        {
            flagSpielerPleite = 1;//Der spieler, der am zug ist ist bereits aus dem Spiel ausgeschieden
        }
        //verarbeitung verschiedener zustände
        switch (zustand)//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        {
            case SPIELERAUSWAHL://Anzahl Spieler auswählen                      
            if (!spielerSetup)
            {
                //LCD ausgabe
                //Aktuelle anzahl Spieler und Navigation auf LCD schreiben
                writeText(0,0,"Spielerauswahl");
                writeText(1,0,PFEIL_L"- 2 Spieler  +"PFEIL_R);
                writeText(2,0,"weiter Taste S");
                //Mit Konto Siebensegmente alle registrierten Spieler anzeigen
                updateKontostand(anzahlSpieler,spielerInfo,0);//Spielernummer auf siebensegmenten anzeigen
                spielerSetup = 1; //flag setzen um erneute ausgabe zu blockieren
            }
            
            //Anzahl Spieler verkleinern
            //Wenn Taste L betätigt wurde und die momentane anzahlSpieler grösser als das erlaubte minimum ist
            if ((positiveFlanke & TASTE14) && anzahlSpieler > MIN_ANZAHL_SPIELER)
            {
                //anzahlSpieler um 1 verkleinern
                anzahlSpieler = anzahlSpieler - 1;
                //anzahlSpieler auf LCD ausgeben
                sprintf(lcdBuffer,"%u",anzahlSpieler);
                writeText(1,3,lcdBuffer);
                //Mit Konto Siebensegmente alle registrierten Spieler anzeigen
                updateKontostand(anzahlSpieler,spielerInfo,0);
            }
            //anzahlspieler vergrössern
            //Wenn Taste R betätigt wurde und die momentane anzahlSpieler kleiner als das erlaubte maximum ist
            if ((positiveFlanke & TASTE15) && anzahlSpieler < MAX_ANZAHL_SPIELER)
            {
                //anzahlSpieler um 1 vergrössern
                anzahlSpieler = anzahlSpieler + 1;
                //anzahlSpieler auf LCD ausgeben
                sprintf(lcdBuffer,"%u",anzahlSpieler);
                writeText(1,3,lcdBuffer);
                //Mit Konto Siebensegmente alle registrierten Spieler anzeigen
                updateKontostand(anzahlSpieler,spielerInfo,0);
            }
            //Wenn anzahl Spieler bestätigt wurde Taste S
            if (positiveFlanke & TASTE13)
            {
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    spielerInfo[i].geld = 0; //setze das geld von Spieler i auf 0
                    spielerInfo[i].position = 0; //Setze die Position von spieler i auf 0
                    setzeSpielerPosition(spielerInfo[i].position,i);//gib die position von spieler i an den leds aus
                }
                //aktualisiere den Kontostand für alle spieler
                updateKontostand(anzahlSpieler,spielerInfo,0);
                //Anzahl Spieler auf LCD anzeigen
                writeText(0,0,"      Spieler   ");
                sprintf(lcdBuffer,"%u",anzahlSpieler);
                writeText(0,4,lcdBuffer);
                //"Startgeld wird verteilt" auf LCD anzeigen
                writeText(1,0,"   Startgeld    ");
                writeText(2,0," wird verteilt  ");
                startGeldAnimation(anzahlSpieler); //animation geld austeilen
                zustand = WUERFELSTART; //zustand auf WUERFELSTART setzen
            }
        	break;
            //in diesem Zustand wird bestimmt wer zuerst würfelt
            case WUERFELSTART://erster Spieler anhand von würfeln bestimmen
            //Geldanzeige aller Spieler auschalten
            //erhöhe i um 1 solange i <= anzahlSpieler ist. Starte mit i = 1
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //schalte die Konto Siebensegmentanzeigen des Spielers i aus
                setGeld(STARTGELD,i,SIEBENSEGMENT_OFF);
            }
            //lässt jeden Spieler würfeln
            //erhöhe i um 1 solange i <= anzahlSpieler ist. Starte mit i = 1
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //Spieler am Zug und Navigation am LCD ausgeben
                writeText(0,0,"   Spieler X    ");
                sprintf(lcdBuffer,"%u",i);//lädt den Spieler am Zug in den lcdBuffer
                writeText(0,11,lcdBuffer);//gibt lcdBuffer an LCD aus
                writeText(1,0,"    w"UE"rfelt    ");
                writeText(2,0," w"UE"rfeln A / B ");
                
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                
                //solange noch nicht mit beiden Würfel gewürfelt wurde 
                /*while (!(flagWuerfel1 && flagWuerfel2)) 
                {
                    //Tasten einlesen und positive Flanken bestimmen
                    tasteAlt = tasteNeu;
                    tasteNeu = 0;
                    tasteNeu = (PINL << 8) | PINK;
                    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                    
                    //Wenn Taste A betätigt wurde und noch nicht mit Würfel A gewürfelt wurde
                    if ((positiveFlanke & TASTE_A) && !flagWuerfel1)
                    {
                        wuerfel(WUERFEL_A,flagWuerfel1,flagWuerfel2);//Würfel A würfeln
                        flagWuerfel1 = 1;//Flag setzen um erneutes würfeln mit Würfel A zu blockieren
                    }
                    //Wenn Taste B betätigt wurde und noch nicht mit Würfel B gewürfelt wurde
                    else if ((positiveFlanke & TASTE_B) && !flagWuerfel2)
                    {
                        wuerfel(WUERFEL_B,flagWuerfel1,flagWuerfel2);//Würfel B würfeln
                        flagWuerfel2 = 1;//Flag setzen um erneutes würfeln mit Würfel B zu blockieren
                    }
                    
                    //prüft welcher Würfel als 2. verwendet wurde dies ist wichtig um sicherzustellen,
                    //dass nur ein spieler die höchste zahl hat.
                    //Wenn das Flag von Würfel A gesetzt ist und noch kein Würfel als zweiter Würfel gespeichert wurde
                    if (flagWuerfel1 && !letzterWuerfel)
                    {
                        //Speichert Würfel B als den Würfel der als zweites gewürfelt wird
                        letzterWuerfel = WUERFEL_B;
                    }
                    //Wenn das Flag von Würfel B gesetzt ist und noch kein Würfel als zweiter Würfel gespeichert wurde
                    if (flagWuerfel2 && !letzterWuerfel)
                    {
                        //Speichert Würfel A als den Würfel der als zweites gewürfelt wird
                        letzterWuerfel = WUERFEL_A;
                    }
                }*/
                warteBisGewuerfelt();
                
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                //Würfelsumme  wird als Geld gespeichert,
                //da es so mit der Funktion updateKontostand ausgegeben werden kann
                
                //Speichert die Summe der beiden gewürfelten Zahlen
                spielerInfo[i].geld = wuerfelArray[0] + wuerfelArray[1];
                
                //falls der aktuelle Spieler die gleiche Summe wie Platz 1 hat
                
                //Wenn der Aktuelle Spieler die gleiche Würfelsumme hat,
                //wie der Spieler der momentan die höchste Summe hat
                if (spielerInfo[i].geld == spielerInfo[ersterSpieler].geld) 
                {
                    //Wenn die Würfelzahl des zweiten Würfels grösser als 1 ist
                    if (wuerfelArray[letzterWuerfel-1] > 1) 
                    {
                        //Die zweite gewürfelte Zahl um 1 verkleinern
                        wuerfelArray[letzterWuerfel-1] = wuerfelArray[letzterWuerfel-1] - 1;
                    }
                    else
                    {
                        //Die zweite gewürfelte Zahl um 1 vergrössern
                        wuerfelArray[letzterWuerfel-1] = wuerfelArray[letzterWuerfel-1] + 1;
                    }
                    //Speichert die angepasste Zahl
                    spielerInfo[i].geld = wuerfelArray[0] + wuerfelArray[1];
                    //Angepasste Zahlen auf Würfel Siebensegment ausgeben.
                    wuerfelTransmit(wuerfelArray[0],wuerfelArray[1]); 
                }
                
                //Wenn der aktuelle Spieler eine grössere Würfelsumme hat,
                //als der Spieler, welcher momentan die höchste Summe hat
                if (spielerInfo[i].geld > spielerInfo[ersterSpieler].geld)
                {
                    //Speichert den momentanen Spieler als den Spieler mit der höchsten Summe
                    ersterSpieler = i;
                }
                
                //Flags und Variabeln zurücksetzen
                flagWuerfel1 = 0;
                flagWuerfel2 = 0;
                letzterWuerfel = 0;
                //Würfelsumme auf den Konto Siebensegmenten des aktuellen Spielers ausgeben
                updateKontostand(i,spielerInfo,0);
            }
            //Setzt den Spieler mit der höchsten Summe als den Spieler der am Zug ist
            spielerAmZug = ersterSpieler;
            
            //Warte eine Sekunde, um es den Spielern zu ermöglichen festzustellen, wer die höchste Würfelsumme hat
            _delay_ms(1000);//eine Sekunde Warten
            wuerfelTransmit(10,10); 
            //erhöhe i um 1 solange i <= anzahlSpieler ist. Starte mit i = 1
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //Setzt den Kontostand des Spielers i auf den Betrag des Startgeldes
                spielerInfo[i].geld = STARTGELD;
            }
            //Kontostand an Konto Siebensegmenten anzeigen
            updateKontostand(anzahlSpieler,spielerInfo,spielerAmZug);

            //Spieler am zug und Navigation am lcd Ausgeben
            writeText(0,0,"   Spieler      ");
            sprintf(lcdBuffer,"%u",spielerAmZug);//lädt die Variabel "spielerAmZug" in den lcdBuffer
            writeText(0,11,lcdBuffer);//lcdBuffer Ausgeben
            writeText(1,0," w"UE"rfeln A / B ");
            writeText(2,0,"weiter C  mehr S");
            
            
            

            
            
            //Wechselt zum Zustand in dem das hauptspiel statt findet
            //Zum Zustand SPIEL wechseln
            zustand = SPIEL;
            break;
            //in diesem Zustand findet das hauptspiel statt
            case SPIEL://Zustand steuert das Spiel
            
            //flagSpielLCD kann auf 1 gesetzt werden um die Standart LCD maske anzuzeigen
            //Wenn das LCD neu beschrieben werden soll
            if (flagSpielLCD)
            {
                //lcd Inhalt löschen
                clear();//lcd leeren
                //Spieler am Zug an LCD ausgeben
                writeText(0,0,"   Spieler      ");
                //lädt die Variabel "spielerAmZug" in den lcdBuffer
                sprintf(lcdBuffer,"%u",spielerAmZug);
                //ausgabe von lcdBuffer auf display
                writeText(0,11,lcdBuffer);
                //Tastenbelegung am LCD ausgeben
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"weiter C  mehr S");
                //flagSpielLcd auf 0 setzen um erneutes Ausführen zu verhindern
                flagSpielLCD = 0;
            }
            
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //                                  Nächster Spieler Logik
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            
            
            
            //Spielzug abschliessen alles zurücksetzen
            //flagSpieler Pleite --> wenn spieler pleite ist, ist automatisch der nächste Spieler an der Reihe
            //wenn Taste C betätigt wurde oder flagGefaengnisWeiter oder flagSpielerPleite gesetzt ist
            //und flagFertigGewuerfelt, flagZahlungAbgeschlossen, flagKaufAbgeschlossen und
            //flagEreignisAbgeschlossen gesetzt sind
            if(((positiveFlanke & TASTE_C) || flagGefaengnisWeiter || flagSpielerPleite) && flagFertigGewuerfelt 
            && flagZahlungAbgeschlossen && flagKaufAbgechlossen && flagEreignisAbgeschlossen)
            {
                //beide Würfel Siebensegment Anzeigen ausschalten
                wuerfelTransmit(SIEBENSEGMENT_OFF,SIEBENSEGMENT_OFF);
                //spielerAmZug auf den nächsten Spieler setzen
                spielerAmZug = (spielerAmZug % anzahlSpieler) + 1;
                if (!spielerInfo[spielerAmZug].pleite)
                {
                    //Kontostand und Spieler am zug ausgeben
                    updateKontostand(anzahlSpieler,spielerInfo,spielerAmZug);
                }
                //Spieler am Zug am LCD ausgeben
                writeText(0,0,"   Spieler      ");
                //lädt die Variabel "spielerAmZug" in den lcdBuffer
                sprintf(lcdBuffer,"%u",spielerAmZug);
                //ausgabe von lcdBuffer auf display
                writeText(0,11,lcdBuffer);
                //Tastenbelegung am LCD ausgeben
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"weiter C  mehr S");
                //~~~~~~~~~~~~flags zurücksetzten~~~~~~~~~~~~~~~~~~
                flagFertigGewuerfelt = 0;//flag zum würfeln blockieren
                PORTC &= BLAULICHT_LED_AUS;//Blaulicht LEDs ausschalten
                //updateLCD auf 0 zurücksetzen
                updateLCD = 0;//flag wird verwendet um lcdausgabe zu blockieren
                //bezahlStatus auf 0 zurücksetzen
                bezahlStatus = 0;//rückgabewert geldUeberweisen
                //ereignisfeldRueckgabe auf 0 zurücksetzen
                ereignisfeldRueckgabe = 0;//gibt an ob ereignis erfolgreich abgeschlossen wurde
                //flagGefaengnis auf 0 zurücksetzen
                flagGefaengnis = 0; //flag wird gesetzt wenn spieler im gefängnis ist
                //flagGefaengnisLCD auf 0 zurücksetzen
                flagGefaengnisLCD = 0;//flag wird im gefängnis verwendet bez. LCD
                //flagGefaengnisWeiter auf 0 zurücksetzen
                flagGefaengnisWeiter = 0;//flag um spiel weiterlaufen zulassen nach entlassung aus gefängnis
                //flagSpielerPleite auf 0 zurücksetzen
                flagSpielerPleite = 0;//flag wird gesetzt um einen Spieler der pleite ist zu überspringen
                //speichert feldTyp in aktuellesFeld
                //wird unten im switch case verwendet
                //aktueller Feld Typ anhand der Position des Spielers am Zug bestimmen
                aktuellesFeld = spielfeld[spielerInfo[spielerAmZug].position].typ;
                
                //überprüft ob der Spieler im gefängnis ist
                //Wenn der aktuelle Feld Typ das Gefängnis ist und der Spieler am Zug verhaftet wurde
                if ((aktuellesFeld == GEFAENGNIS) && spielerInfo[spielerAmZug].gefaengnis)
                {
                    //flagGefaengnis auf 1 setzen
                    flagGefaengnis = 1;//flag setzen wenn spieler im gefängnis ist
                    //flagFertigGewuerfelt auf 1 setzen um würfeln zu blockieren
                    flagFertigGewuerfelt = 1;
                }
                //Wenn der Spieler am Zug pleite ist
                if (spielerInfo[spielerAmZug].pleite)
                {
                    //flagSpielerPleite auf 1 setzen um würfeln zu blockieren
                    flagSpielerPleite = 1;//Der spieler, der am zug ist ist bereits aus dem Spiel ausgeschieden
                    //flagFertigGewuerfelt auf 1 setzen um würfeln zu blockieren
                    flagFertigGewuerfelt = 1;
                    //aktuellesFeld auf FREIPARKEN setzen um sicherzustellen, dass keine Aktion ausgelöst wird
                    aktuellesFeld = FREIPARKEN;
                }
                //Wenn der aktuelle Spieler nicht pleite ist wird kontostand und spieler am zug ausgegeben
                
            }
            //ermöglicht es dem spieler bei Pasch zu kaufen
            //Wenn die Tatse C betätigt wurde und flagWeiter nicht gesetzt ist und flagZahlungAbgeschlossen und
            //flagEreignisAbgeschlossen gesetzt sind
            if ((positiveFlanke & TASTE_C) && !flagWeiter && flagZahlungAbgeschlossen && flagEreignisAbgeschlossen)
            {
                //Würfel Siebensegmente ausschalten
                wuerfelTransmit(SIEBENSEGMENT_OFF,SIEBENSEGMENT_OFF);
                //flagWeiter auf 1 setzten um erneutes Würfeln zu ermöglichen
                flagWeiter = 1;
                //bezahlStatus auf 0 setzen um zu verhindern, dass der Spielzug abgeschlossen wird
                bezahlStatus = 0;
                //ereignisfeldRueckgabe auf 0 setzen um erneutes auslösen eines Ereignissfeldes zu ermöglichen
                ereignisfeldRueckgabe = 0;
            }
            //lässt den Spieler einmal würfel
            //Wenn flagWeiter nicht gesetzt ist, kann man nicht würfeln. Das
            //flag braucht es, da man ansonsten bei einem pasch nichts kaufen kann
            //wenn flagSpielerPleite = 1 ist spielt der spieler nicht mehr mit
            //Wenn flagFertigGewuerfelt und flagSpielerPleite nicht gesetzt sind und flagWeiter gesetzt ist
            if (!flagFertigGewuerfelt && flagWeiter && !flagSpielerPleite) 
            {
                //wartet bis mit beiden Würfel gewürfelt wurde
                warteBisGewuerfelt();//lässt den spieler würfeln
                //lässt den spieler bei einem Pasch nochmal würfeln,
                //ausser der Pasch wurde im Gefängniss gemacht
                //Wenn beide gewürfelten Zahlen gleich sind und der Spieler nicht verhaftet wurde
                if ((wuerfelArray[0] == wuerfelArray[1]) && !flagGefaengnis) //Pasch
                {   
                    //flagWeiter auf 0 setzen um frühzeitiges würfeln zu blockieren
                    flagWeiter = 0; //bei pasch flag = 0 => frühzeitiges würfeln blockieren
                    paschZaehler = paschZaehler + 1; //paschZaehler um 1 erhöhen
                    //flags dienen um festzustellen
                    //mit welchem würfel bereits gewürfelt wurde
                    //flagWuerfel1 auf 0 zurücksetzen
                    flagWuerfel1 = 0;
                    //flagWuerfel2 auf 0 zurücksetzen
                    flagWuerfel2 = 0;
                    //Verarbeitung aufgrund von Anzahl aufeinanderfolgende Pasch
                    switch (paschZaehler) //schaltet LEDs ein bei Pasch
                    {
                        case 1://1. Pasch
                        PORTC |= BLAULICHT_LED_1; //Erste Blaulicht LED einschalten
                    	break;
                        case 2:
                        PORTC |= BLAULICHT_LED_2; //Zweite Blaulicht LED einschalten
                        break;
                        case 3: //beim 3. Pasch = Gefängnis
                        PORTC &= BLAULICHT_LED_AUS; //Blaulicht ausschalten

                        //flagWuerfel1 auf 0 zurücksetzen
                        flagWuerfel1 = 0;
                        //flagWuerfel2 auf 0 zurücksetzen
                        flagWuerfel2 = 0;
                        //paschZähler auf 0 zurücksetzen
                        paschZaehler = 0;
                        wuerfelArray[0] = 0;//würfelzahlen zurücksetzen
                        wuerfelArray[1] = 0;//würfelzahlen zurücksetzen
                        
                        
                        flagWeiter = 1; //flagWeiter auf 1 setzen
                        flagFertigGewuerfelt = 1;//flagFertigGewuerfelt auf 1 setzen
                        //Spieler am Zug ins Gefängnis schicken
                        abInsGefaengnis(spielerAmZug); 
                        aktuellesFeld = GEFAENGNIS;
                        break;
                    }
                }
                else//wenn kein pasch gewürfelt wurde
                {
                    //flag setzen erlaubt es den spielzug abzuschliessen
                    flagFertigGewuerfelt = 1; //flagFertigGewuerfelt auf 1 setzen um erneutes Würfeln zu blockieren
                    paschZaehler = 0; //paschZaehler zurücksetzen
                }
                 //wenn die aktuelle spition des Spielers + wüfelsumme grösser gleich 40 ist
                 // erhält der spieler 200 CHF
                 //Prüft ob der Spieler über los kommt
                 
                 //animiert die fortbewegung des spielers
                 //erhöhe i um 1 solange i <= alte position + Würfelsumme. Starte mit i = aktuielle Position
                 for (uint8_t i = spielerInfo[spielerAmZug].position + 1; i <= (spielerInfo[spielerAmZug].position + (wuerfelArray[0] + wuerfelArray[1])); i = i + 1)
                 {
                     //Aktuelle Spielerposition anhand von i berechnen und an LEDs ausgeben
                     setzeSpielerPosition(i % ANZAHL_FELDER,spielerAmZug);
                     //Wenn der Spieler auf dem Feld Los ist
                     if ((i % ANZAHL_FELDER) == 0)
                     {
                         //Rundengeld an den Spieler überweisen
                         geldUeberweisen(0,spielerAmZug,RUNDEN_GELD);
                     }
                     //Programm für 100ms blockieren
                     _delay_ms(100); //delay dient zu animationszwecken
                 }
                 //addiert die würfelsumme zur aktuellen position dazu
                 //Neue Position speichern
                 spielerInfo[spielerAmZug].position = (spielerInfo[spielerAmZug].position + (wuerfelArray[0] + wuerfelArray[1])) % 40;
                 //Neue Spielerposition an LEDs ausgeben
                 setzeSpielerPosition(spielerInfo[spielerAmZug].position,spielerAmZug);
                 //Typ des aktuellen Feldes speichern
                 aktuellesFeld = spielfeld[spielerInfo[spielerAmZug].position].typ;
                 //Aktuelle Position speichern
                 aktuellePosition = spielerInfo[spielerAmZug].position;
                 //flagWuerfel1 auf 0 zurücksetzen
                 flagWuerfel1 = 0;
                 //flagWuerfel2 auf 0 zurücksetzen
                 flagWuerfel2 = 0;
            }
            
            //kontostand aktualisieren und anzeigen
            updateKontostand(anzahlSpieler,spielerInfo,spielerAmZug);
            //Spieler hat fertig gewürfelt 
            
            //Wenn Taste S betätigt wurde
            if (positiveFlanke & TASTE_S)
            {
                //Wenn die Taste S betätigt wurde gelangt man zur verwaltung
                /*writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"weiter C  mehr S");*/
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~VERWALTUNG~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                updateLCD = 0;//updateLCD auf 0 setzen
                zustand = VERWALTEN;//zum zustand VERWALTEN wechseln
            }
            
            switch (aktuellesFeld) //verarbeitung aufgrund aktuellem feldtyp
            {
                //wenn das aktuelle Feld ein Ereignissfeld ist
                case EREIGNISFELD:
                //prüft ob das aktuelle feld Chance ode Kanzlei ist
                //dies ist wichtig, da die chance karten sich von den Kanzlei karten unterscheiden
                //Wenn die aktuelle Position ein Chance Feld ist
                if ((aktuellePosition == CHANCE1) || (aktuellePosition == CHANCE2) || (aktuellePosition == CHANCE3))
                {
                    //flagKanzlei auf 0 setzen, um zu markieren, dass der Spieler sich auf einem Chance Feld befindet
                    flagKanzlei = 0;//das feld ist chance
                }
                else
                {
                    //flagKanzlei auf 1 setzen, um zu markieren, dass der Spieler sich auf einem Kanzlei Feld befindet
                    flagKanzlei = 1;//das feld ist kanzlei
                }
                //Wenn das Ereignis noch nicht durchgeführt wurde
                if (!ereignisfeldRueckgabe)
                {
                    //flagEreignisAbgeschlossen auf 0 setzen
                    flagEreignisAbgeschlossen = 0;//wird ev. nicht behr benötigt
                    //Wenn Taste R betätigt wurde und noch nicht der ganze Text auf dem LCD angezeigt wurde
                    if ((positiveFlanke & TASTE_R) || !ereignisSchritt)//Taste zum scrollen
                    {                        
                        //Zufälliges Ereignis anhand von flagKanzlei bestimmen und 
                        //anhand von der Variabel ereignisSchritt die aktuellen Zeilen Text,
                        //welche angezeigt werden sollen bestimmen
                        ereignisfeldRueckgabe = 
                        ereignisFeld(flagKanzlei,spielerAmZug,ereignisSchritt,flagEreignisWeiter,chanceKanzlei);
                        //Wenn noch nicht der ganze Text des Ereignises angezeigt wurde
                        if (!ereignisfeldRueckgabe)
                        {
                            //erhöht den schritt zähler um 2
                            //das bedeutet, das jeweils die nächsten 2 zeilen auf dem LCD angezeigt werden
                            //ereignisSchritt um ANZAHL_EREIGNISS_LCD_TEXT_ZEILEN erhöhen
                            ereignisSchritt += ANZAHL_EREIGNISS_LCD_TEXT_ZEILEN;
                        }
                    }
                    //Wenn der aktuelle Spieler, das Ereignis, mit seiner X Taste bestätigt hat
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        //flagEreignisWeiter auf 1 setzen, damit das Ereigniss durchgeführt werden kann
                        flagEreignisWeiter = 1;
                        //das zuvor bestimmte Ereignis durchführen
                        ereignisfeldRueckgabe =
                        ereignisFeld(flagKanzlei,spielerAmZug,ereignisSchritt,flagEreignisWeiter,chanceKanzlei);
                        //Falls das Ereignis den Spieler auf ein anderes Feld schickt,
                        //muss der neue Feld Typ bestimmt werden
                        //Aktueller Feld Typ anhand der Position des Spielers bestimmen
                        aktuellesFeld = spielfeld[spielerInfo[spielerAmZug].position].typ;
                        //Aktuelle Position speichern
                        aktuellePosition = spielerInfo[spielerAmZug].position;
                        //flagEreignisWeiter auf 0 zurücksetzen
                        flagEreignisWeiter = 0;
                        //ereignisSchritt auf 0 zurücksetzen
                        ereignisSchritt = 0;
                        //flagEreignisAbgeschlossen auf 1 setzen, um erneutes durchführen zu blockieren
                        flagEreignisAbgeschlossen = 1;
                        //flagSpielLCD auf 1 setzen, um die Standart LCD Maske anzuzeigen
                        flagSpielLCD = 1;
                    }
                }
                break;
                //wenn das aktuelle Feld eine strasse ist
                case STRASSE:
                //Felder, welche niemandem gehören, können gekauft werden
                //Wenn das Aktuelle Feld keinen Besitzer hat
                if (spielfeld[aktuellePosition].besitzer == 0)
                {
                    //flagKaufAbgechlossen auf 0 setzen
                    flagKaufAbgechlossen = 0;
                    //Das aktuelle Feld dem Spieler zum kauf anbieten und den Rückgabewert speichern
                    verkaufSpielerEingabe = feldKaufen(aktuellePosition,spielfeld,spielerAmZug);
                    //verarbeitung aufgrund von Rückgabewert 
                    switch (verkaufSpielerEingabe)
                    {
                        case FELD_KAUFEN_KEIN_SPIELER_INPUT://Spieler hat noch keine Entscheidung getroffen
                    	break;
                        case FELD_KAUFEN_FELD_GEKAUFT://Spieler hat das Feld gekauft
                        //flagKaufAbgechlossen auf 1 setzen um das Weiterspielen zu ermöglichen
                        flagKaufAbgechlossen = 1;
                        break;
                        case FELD_KAUFEN_FELD_NICHT_GEKAUFT://Spieler hat das Feld nicht gekauft
                        //flagKaufAbgechlossen auf 1 setzen um das Weiterspielen zu ermöglichen
                        flagKaufAbgechlossen = 1;
                        //bezahlStatus auf 1 setzen, ansonsten müsste der Spieler nach der Auktion Miete bezahlen
                        bezahlStatus = 1;
                        //feld wird versteigert, weil es nicht gekauft wurde
                        //Zum zustand VERSTEIGERUNG wechseln
                        zustand = VERSTEIGERUNG;
                        break;
                    }
                }
                //Überprüfe ob das Feld belastet ist
                feldbelastet = spielfeld[aktuellePosition].feldBelastet;
                //Wenn das aktuelle Feld einem anderen Spieler gehört,
                //der aktuelle Spieler noch nicht bezahlt hat und das Feld nicht belastet ist
                if((spielfeld[aktuellePosition].besitzer && !(spielfeld[aktuellePosition].besitzer == spielerAmZug))
                    && !(bezahlStatus == 1) && !feldbelastet)
                {
                    //flagZahlungAbgeschlossen auf 0 setzen um das abschliessen des Spielzuges zu blockieren
                    flagZahlungAbgeschlossen = 0;
                    //Wenn das LCD noch nicht aktualisiert wurde
                    if (!updateLCD)
                    {
                        //feldBesitzer auslesen und Speichern
                        feldBesitzer = spielfeld[aktuellePosition].besitzer;
                        //Miete anhand der Anzahl Häuser auf dem Feld auslesen und in der Variabel zahlBetrag speichern
                        zahlBetrag = spielfeld[aktuellePosition].mieten[spielfeld[aktuellePosition].anzahlHaeuser];
                        //Miete für eine komplette Farbgruppe auslesen und in der Variabel zahlBetragFarbgruppe speichern
                        zahlBetragFarbgruppe = spielfeld[aktuellePosition].mieten[6];
                        //flagMieteFarbgruppe auf 1 setzen
                        flagMieteFarbgruppe = 1;
                        
                        //prüft ob die ganze farbgruppe dem selben Spieler gehhört
                        //Erhöhe i um 1 solange i < ANZAHL_FELDER_FARBGRUPPE ist. Starte mit i =  0
                        for (uint8_t i = 0; i < 3; i = i + 1)
                        {
                            //Wenn das Feld i der Farbgruppe, nicht dem selben Spieler gehört wie das aktuelle Feld
                            if (!(spielfeld[spielfeld[aktuellePosition].farbgruppenFelder[i]].besitzer == feldBesitzer))
                            {
                                //flagMieteFarbgruppe auf 0 setzen um zu markieren,
                                //dass dem Spieler nicht die komplette Farbgruppe gehört
                                flagMieteFarbgruppe = 0;
                            }
                        }
                        //bestimmt ob farbgruppenmiete oder miete mit Häuser bezahlt werden muss
                        //In der Variabel zahlBetrag ist Gespeichert, wie viel Geld überwiesen werden muss.
                        //Wenn der Spieler keine Häuser hat, aber eine Farbgruppe, dann wird die Variabel
                        //dementsprechend angepasst
                        //wenn er Häuser hat oder nicht die gesammte Farbgruppe,
                        //dann muss die Variabel nicht angepasst werden
                        
                        //Wenn dem Spieler die komplette Farbgruppe gehört und zahlBetragFarbgruppe > zahlBetrag ist
                        if (flagMieteFarbgruppe && (zahlBetragFarbgruppe > zahlBetrag))
                        {
                            //Variabel zahlBetrag auf zahlBetragFarbgruppe setzen
                            zahlBetrag = zahlBetragFarbgruppe;
                        }
                        
                        //LCD ausgabe
                        //Spieler am Zug am LCD ausgeben
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);   //spielerAmZug in den lcdBuffer laden
                        writeText(0,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //Miete die bezahlt werden muss am LCD ausgeben
                        writeText(1,0,"  bezahle       ");
                        sprintf(lcdBuffer,"%u",zahlBetrag);     //zahlBetrag in den lcdBuffer laden
                        writeText(1,10,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //Tastenbelegung und Feldbesitzer am LCD ausgeben
                        writeText(2,0,"an Spieler     X");
                        sprintf(lcdBuffer,"%u",feldBesitzer);   //feldBesitzer in den lcdBuffer laden
                        writeText(2,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //updateLCD auf 1 setzen um erneutes Ausführen zu verhindern
                        updateLCD = 1;
                    }
                    
                    //Wenn der Spieler am Zug, den Betrag, mit seiner Taste X bestätigt hat
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        //Den zu bezahlenden Betrag an den Feldbesitzer überweisen
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag);
                        //Wenn die Zahlung erfolgreich war
                        if (bezahlStatus == 1)
                        {
                            //updateLCD auf 0 zurücksetzen
                            updateLCD = 0;
                            //flagZahlungAbgeschlossen auf 1 setzen um es zu ermöglichen den Spielzug abzuschliessen
                            flagZahlungAbgeschlossen = 1;
                            //flagSpielLCD auf 1 setzen um die Standart LCD Maske einzublenden
                            flagSpielLCD = 1;
                        }
                    }
                }
                break;
                //wenn das aktuelle Feld ein Steuerfeld ist
                case STEUERFELD:
                //Wenn die Zahlung noch nicht abgeschlossen ist
                if(!(bezahlStatus == 1))
                {
                    //flagZahlungAbgeschlossen auf 0 setzen um zu verhindern, dass der Spielzug abgeschlossen wird
                    flagZahlungAbgeschlossen = 0;
                    //Wenn das LCD noch nicht aktualisiert wurde
                    if (!updateLCD)
                    {
                        //Den zu bezahlenden Betrag auslesen und speichern
                        zahlBetrag = spielfeld[aktuellePosition].preis;
                        //Spieler am Zug an LCD ausgeben
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);   //spielerAmZug in den lcdBuffer laden
                        writeText(0,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //Betrag, der bezahlt werden muss, am LCD ausgeben
                        writeText(1,0,"  bezahle       ");
                        sprintf(lcdBuffer,"%u",zahlBetrag);     //zahlBetrag in den lcdBuffer laden
                        writeText(1,10,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //Tastenbelegung am LCD ausgeben
                        writeText(2,0,"an die Bank    X");
                        //updateLCD auf 1 setzen um erneutes ausführen zu verhindern
                        updateLCD = 1;
                    }
                        
                    //Wenn der Spieler am Zug, den Betrag, mit seiner Taste X bestätigt hat
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        //Den zu bezahlenden Betrag an die Bank überweisen
                        bezahlStatus = geldUeberweisen(spielerAmZug,0,zahlBetrag);
                        //Wenn die Zahlung erfolgreich war
                        if (bezahlStatus == 1)
                        {
                            //updateLCD auf 0 zurücksetzen
                            updateLCD = 0;
                            //flagZahlungAbgeschlossen auf 1 setzen um es zu ermöglichen den Spielzug abzuschliessen
                            flagZahlungAbgeschlossen = 1;
                            //flagSpielLCD auf 1 setzen um die Standart LCD Maske einzublenden
                            flagSpielLCD = 1;
                        }
                    }
                }
                break;
                case HALTESTELLE:
                //Wenn das Aktuelle Feld keinen Besitzer hat
                if (spielfeld[aktuellePosition].besitzer == 0)
                {
                    //flagKaufAbgechlossen auf 0 setzen
                    flagKaufAbgechlossen = 0;
                    //Das aktuelle Feld dem Spieler zum kauf anbieten und den Rückgabewert speichern
                    verkaufSpielerEingabe = feldKaufen(aktuellePosition,spielfeld,spielerAmZug);
                    //Rückgabewert verarbeiten
                    switch (verkaufSpielerEingabe)
                    {
                        case FELD_KAUFEN_KEIN_SPIELER_INPUT://Spieler hat noch keine Entscheidung getroffen
                        break;
                        case FELD_KAUFEN_FELD_GEKAUFT://Spieler hat das Feld gekauft
                        //flagKaufAbgechlossen auf 1 setzen um das Weiterspielen zu ermöglichen
                        flagKaufAbgechlossen = 1;
                        break;
                        case FELD_KAUFEN_FELD_NICHT_GEKAUFT://Spieler hat das Feld nicht gekauft
                        //flagKaufAbgechlossen auf 1 setzen um das Weiterspielen zu ermöglichen
                        flagKaufAbgechlossen = 1;
                        //bezahlStatus auf 1 setzen, ansonsten müsste der Spieler nach der Auktion Miete bezahlen
                        bezahlStatus = 1;
                        //Zum zustand VERSTEIGERUNG wechseln
                        zustand = VERSTEIGERUNG;
                        break;
                    }
                }
                //Überprüfe ob das Feld belastet ist
                feldbelastet = spielfeld[aktuellePosition].feldBelastet;
                //Wenn das aktuelle Feld einem anderen Spieler gehört, der aktuelle Spieler
                //noch nicht bezahlt hat und das Feld nicht belastet ist
                if((spielfeld[aktuellePosition].besitzer && !(spielfeld[aktuellePosition].besitzer == spielerAmZug)) 
                    && !(bezahlStatus == 1) && !feldbelastet)
                {
                    //flagZahlungAbgeschlossen auf 0 setzen um das abschliessen des Spielzuges zu blockieren
                    flagZahlungAbgeschlossen = 0;
                    //Wenn das LCD noch nicht aktualisiert wurde
                    if (!updateLCD)
                    {
                        //Feld Besitzer auslesen und Speichern
                        feldBesitzer = spielfeld[aktuellePosition].besitzer;
                        //haltestelleFarbgruppe auf 0 setzen
                        haltestelleFarbgruppe = 0;
                        //pröft wie viele HAltestellen der spieler besitzt
                        for (uint8_t i = POS_HALTESTELLE1; i <= POS_HALTESTELLE4; i = i + HALTESTELLEN_ABSTAND)
                        {
                            //Wenn die Haltestelle i, dem gleichen Spieler gehört wie die aktuelle Haltestelle
                            if (spielfeld[i].besitzer == spielfeld[aktuellePosition].besitzer)
                            {
                                //haltestelleFarbgruppe um 1 erhöhen
                                haltestelleFarbgruppe += 1; 
                            }
                        }
                        //Von der Variabel haltestelleFarbgruppe eins abziehen, 
                        //da der Variabeln Wert sonst nicht mit den, im system hinterlegten Preisen übereinstimmt
                        haltestelleFarbgruppe -= 1;
                        //Den zu bezahlenden Betrag auslesen
                        zahlBetrag = spielfeld[aktuellePosition].mieten[haltestelleFarbgruppe];
                        //Spieler am Zug am LCD ausgeben
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);   //spielerAmZug in den lcdBuffer laden
                        writeText(0,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //Preis, der bezahlt werden muss am LCD ausgeben
                        writeText(1,0,"  bezahle       ");
                        sprintf(lcdBuffer,"%u",zahlBetrag);     //zahlBetrag in den lcdBuffer laden
                        writeText(1,10,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //Tastenbelegung und Feldbesitzer am LCD ausgeben
                        writeText(2,0,"an Spieler     X");
                        sprintf(lcdBuffer,"%u",feldBesitzer);   //feldBesitzer in den lcdBuffer laden
                        writeText(2,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //updateLCD auf 1 setzen um erneutes Ausführen zu verhindern
                        updateLCD = 1;
                    }
                    
                    //Wenn der Spieler am Zug, den Betrag, mit seiner Taste X bestätigt hat
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        //Den zu bezahlenden Betrag an den Feldbesitzer überweisen
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag);
                        //Wenn die Zahlung erfolgreich war
                        if (bezahlStatus == 1)
                        {
                            //updateLCD auf 0 zurücksetzen
                            updateLCD = 0;
                            //flagZahlungAbgeschlossen auf 1 setzen um es zu ermöglichen den Spielzug abzuschliessen
                            flagZahlungAbgeschlossen = 1;
                            //flagSpielLCD auf 1 setzen um die Standart LCD Maske einzublenden
                            flagSpielLCD = 1;
                        }
                    }
                }
                break;
                //Wenn das Atuelle Feld das Gefängnis ist
                case GEFAENGNIS:
                //Wenn flagGefaengnis gesetzt ist
                if (flagGefaengnis)
                {
                    //Wenn der Spieler am Zug im Gefängnis ist und noch keine LCD Ausgabe gemacht wurde
                    if (spielerInfo[spielerAmZug].gefaengnis && !flagGefaengnisLCD)
                    {
                        //Spieler am Zug am LCD ausgeben
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);   //spielerAmZug in den lcdBuffer laden
                        writeText(0,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        writeText(1,0,"   Du bist im   ");
                        writeText(2,0,"    Workshop    ");
                        //flagGefaengnisLCD setzen um erneutes Ausführen zu verhindern
                        flagGefaengnisLCD = 1;
                        //Programm für fünf Sekunden blockieren, um den Spielern Zeit zu geben das LCD zu lesen
                        _delay_ms(3000);
                        //Wenn der Spieler am Zug eine Freikarte besitzt
                        if (spielerInfo[spielerAmZug].freikarte)
                        {
                            //Zum workshopZustand FREIKARTE_J_N wechseln
                            workshopZustand = FREIKARTE_J_N;
                            //Via LCD fragen, ob der Spieler die Freikarte verwenden will
                            //und Tastenbelegung am LCD ausgeben
                            writeText(1,0,"Freikarte X=JA  ");
                            writeText(2,0,"verwenden Y=NEIN");
                        }
                        else
                        {
                            //Zum workshopZustand PASH_J_N wechseln
                            workshopZustand = PASCH_J_N;
                            //Via LCD fragen, ob der Spieler einen Pash würfeln will
                            //und Tastenbelegung am LCD ausgeben
                            writeText(1,0,"  Pasch   X=JA  ");
                            writeText(2,0,"W"UE"rfeln   Y=NEIN");
                        }
                    }
                    //Verarbeitet die eingabe des Spielers
                    switch (workshopZustand)
                    {
                        //Will der Spieler eine Freikarte verwenden?
                        case FREIKARTE_J_N:
                        //Wenn der Spieler am Zug seine Taste Y betätigt hat
                        if (flagTasteY)//spieler will Freikarte nicht verwenden
                        {
                            //Via LCD fragen, ob der Spieler einen Pash würfeln will
                            //und Tastenbelegung am LCD ausgeben
                            writeText(1,0," Pasch    X=JA  ");
                            writeText(2,0,"W"UE"rfeln   Y=NEIN");
                            //Zum workshopZustand PASH_J_N wechseln
                            workshopZustand = PASCH_J_N;
                        }
                        //Wenn der Spieler am Zug seine Taste X betätigt hat
                        else if (flagTasteX) //spieler will Freikarte verwenden
                        {
                            //Spieler am Zug, als nicht mehr verhaftet markieren
                            spielerInfo[spielerAmZug].gefaengnis = 0;
                            //Anzahl Freikarten des Spielers reduzieren
                            spielerInfo[spielerAmZug].freikarte = 0;
                            //Anzahl Runden, welche der Spieler
                            //im Gefängnis verbracht hat auf 0 setzen
                            spielerInfo[spielerAmZug].rundenImGefaengnis = 0;
                            //flagFertigGewuerfelt auf 0 setzen um dem
                            //Spieler am Zug das Würfeln zu ermöglichen
                            flagFertigGewuerfelt = 0;
                            //flagGefaengnis auf 0 zurücksetzen
                            flagGefaengnis = 0;
                            //Tastenbelegung am LCD ausgeben
                            writeText(1,0," w"UE"rfeln A / B ");
                            writeText(2,0,"                ");
                        }
                        break;
                        //Will der Spieler einen Pasch würfeln
                        case PASCH_J_N:
                        //Wenn der Spieler am Zug seine Taste Y betätigt hat
                        if (flagTasteY)//Spieler will keinen Pasch würfeln
                        {
                            //Via LCD fragen, ob der Spieler bezahlen will
                            //und Tastenbelegung am LCD ausgeben
                            writeText(1,0,"   50     X=JA  ");
                            writeText(2,0,"Bezahlen  Y=NEIN");
                            //Zum workshopZustand BEZAHLEN_J_N wechseln
                            workshopZustand = BEZAHLEN_J_N;
                        }
                        //Wenn der Spieler am Zug seine Taste X betätigt hat
                        else if (flagTasteX)//Spieler will Pasch würfeln
                        {
                            //Tastenbelegung am LCD ausgeben
                            writeText(1,0," w"UE"rfeln A / B ");
                            writeText(2,0,"                ");
                            //Zum workshopZustand PASH wechseln
                            workshopZustand = PASCH;
                        }
                        break;
                        //Will der Spieler bezahlen
                        case BEZAHLEN_J_N:
                        //Wenn der Spieler am Zug seine Taste Y betätigt hat
                        if (flagTasteY)//Spieler will nicht zahlen
                        {
                            //Wenn der Spieler am Zug eine Freikarte besitzt
                            if (spielerInfo[spielerAmZug].freikarte)
                            {
                                //Via LCD fragen, ob der Spieler die Freikarte verwenden will
                                //und Tastenbelegung am LCD ausgeben
                                writeText(1,0,"Freikarte X=JA  ");
                                writeText(2,0,"verwenden Y=NEIN");
                                //Zum workshopZustand FREIKARTE_J_N wechseln
                                workshopZustand = FREIKARTE_J_N;
                            }
                            else//Wenn der Spieler keine Freikarte hat, ist die nächste option: Pasch
                            {
                                //Via LCD fragen, ob der Spieler einen Pash würfeln will
                                //und Tastenbelegung am LCD ausgeben
                                writeText(1,0," Pasch    X=JA  ");
                                writeText(2,0,"W"UE"rfeln   Y=NEIN");
                                //Zum workshopZustand PASH_J_N wechseln
                                workshopZustand = PASCH_J_N;
                            }
                        }
                        //Wenn der Spieler am Zug seine Taste X betätigt hat
                        else if (flagTasteX)//Spieler will bezahlen
                        {
                            //Geld an Bank überweisen
                            bezahlStatus = geldUeberweisen(spielerAmZug,0,50);
                            //Wenn die Zahlung erfolgreich war
                            if (bezahlStatus == ZAHLUNG_ERFOLGREICH)
                            {
                                //Zahlung erfolgreich am LCD ausgeben
                                writeText(1,0,"     Zahlung    ");
                                writeText(2,0,"   Erfolgreich  ");
                                //Spieler am Zug, als nicht mehr verhaftet markieren
                                spielerInfo[spielerAmZug].gefaengnis = 0;
                                //Anzahl Runden, welche der Spieler
                                //im Gefängnis verbracht hat auf 0 setzen
                                spielerInfo[spielerAmZug].rundenImGefaengnis = 0;
                                //flagFertigGewuerfelt auf 0 setzen um dem
                                //Spieler am Zug das Würfeln zu ermöglichen
                                flagFertigGewuerfelt = 0;
                                //flagGefaengnis auf 0 zurücksetzen
                                flagGefaengnis = 0;
                                _delay_ms(1000);
                                writeText(1,0," w"UE"rfeln A / B ");
                                writeText(2,0,"weiter C  mehr S");
                                updateKontostand(anzahlSpieler,spielerInfo,spielerAmZug);
                            }
                            /*else if (bezahlStatus == ZAHLUNG_FEHLGESCHLAGEN)
                            {
                                writeText(1,0,"     Zahlung    ");
                                writeText(2,0," Fehlgeschlagen ");
                                _delay_ms(5000); //5s Warten 
                                //wechsel zum nächsten zustand
                                //prüfen ob spieler eine freikarte hat
                                if (spielerInfo[spielerAmZug].freikarte)
                                {
                                    workshopZustand = FREIKARTE_J_N;
                                    writeText(1,0,"Freikarte X=JA  ");
                                    writeText(2,0,"verwenden Y=NEIN");
                                }
                                else
                                {
                                    workshopZustand = PASCH_J_N;
                                    writeText(1,0,"  Pasch   X=JA  ");
                                    writeText(2,0,"W"UE"rfeln   Y=NEIN");
                                }
                                / *sprintf(lcdBuffer,"%u",spielerInfo[spielerAmZug].rundenImGefaengnis);
                                writeText(1,0,lcdBuffer);
                                writeText(1,0,"   Runden X=JA  ");
                                writeText(2,0,"  warten  Y=NEIN");* /
                            }*/
                        }
                        break;
                        case PASCH:
                        //wartet bis mit beiden Würfel gewürfelt wurde
                        //flagWuerfel1 auf 0 setzten
                        flagWuerfel1 = 0;
                        //flagWuerfel2 auf 0 setzten
                        flagWuerfel2 = 0;
                        //Warte bis mit beiden Würfeln gewürfelt wurde
                        warteBisGewuerfelt();
                        //Wenn ein Pasch gewürfelt wurde
                        if (wuerfelArray[0] == wuerfelArray[1])
                        {
                            //"PASCH" am LCD ausgeben
                            writeText(1,0,"     PASCH      ");
                            writeText(2,0,"                ");                           
                            //Spieler am Zug, als nicht mehr verhaftet markieren
                            spielerInfo[spielerAmZug].gefaengnis = 0;
                            //Anzahl Runden, welche der Spieler
                            //im Gefängnis verbracht hat auf 0 setzen
                            spielerInfo[spielerAmZug].rundenImGefaengnis = 0;
                            //flagFertigGewuerfelt auf 0 setzen um dem
                            //Spieler das vorrücken mit der soeben gewürfelten Summe zu ermöglichen
                            flagFertigGewuerfelt = 0;
                            //Programm für fünf Sekunden blockieren
                            //um es den Spielern zu ermöglichen das LCD zu lesen
                            _delay_ms(5000); //5s Warten
                        }
                        else
                        {
                            //"KEIN PASCH" am LCD ausgeben
                            writeText(1,0,"   KEIN PASCH   ");
                            writeText(2,0,"                ");
                            //flagWuerfel1 auf 0 setzten
                            flagWuerfel1 = 0;
                            //flagWuerfel2 auf 0 setzten
                            flagWuerfel2 = 0;
                            //Anzahl Runden, welche der Spieler
                            //im Gefängnis verbracht hat um 1 erhöhen
                            spielerInfo[spielerAmZug].rundenImGefaengnis += 1;
                            //Wenn beim dritten Versuch kein Pasch gewürfelt wurde
                            if (spielerInfo[spielerAmZug].rundenImGefaengnis == 3)
                            {
                                //nach 3 Runden muss bezahlt werden
                                //Geld an Bank überweisen
                                bezahlStatus = geldUeberweisen(spielerAmZug,0,50);
                                //Spieler am Zug, als nicht mehr verhaftet markieren
                                spielerInfo[spielerAmZug].gefaengnis = 0;
                                //Anzahl Runden, welche der Spieler
                                //im Gefängnis verbracht hat auf 0 setzen
                                spielerInfo[spielerAmZug].rundenImGefaengnis = 0;
                                //flagFertigGewuerfelt auf 0 setzen um dem
                                //Spieler am Zug das Würfeln zu ermöglichen
                                flagFertigGewuerfelt = 0; 
                                writeText(1,0," w"UE"rfeln A / B ");
                                writeText(2,0,"weiter C  mehr S");
                                updateKontostand(anzahlSpieler,spielerInfo,spielerAmZug);
                            }
                            else
                            {
                                //Der nächste Spieler ist am zug
                                //flagGefaengnisWeiter auf 1 setzen, um den Spielzug abzuschliessen
                                flagGefaengnisWeiter = 1;
                                //Programm für fünf Sekunden blockieren
                                //um es den Spielern zu ermöglichen das LCD zu lesen
                                _delay_ms(3000); //5s Warten
                            }
                        }
                        break;
                    }
                    /*if (flagGefaengnisLCD)
                    {
                    }*/
                    //tasten abfragen
                    //Wenn der Spieler am Zug seine Tatse X betätigt hat
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        //flagTasteX auf 1 setzen
                        flagTasteX = 1;
                        //flagTasteY auf 0 setzen
                        flagTasteY = 0;
                    }
                    //Wenn der Spieler am Zug seine Tatse Y betätigt hat
                    else if (positiveFlanke & yTasten[spielerAmZug - 1])
                    {
                        //flagTasteX auf 0 setzen
                        flagTasteX = 0;
                        //flagTasteY auf 1 setzen
                        flagTasteY = 1;
                    }
                    else
                    {
                        //flagTasteX auf 0 setzen
                        flagTasteX = 0;
                        //flagTasteY auf 0 setzen
                        flagTasteY = 0;
                    }
                }
                break;
                //Wenn das Aktuelle Feld gehe ins gefängnis ist
                case GEH_INS_GEFAENGNIS:
                //Schick den Spieler ins Gefängnis
                abInsGefaengnis(spielerAmZug);
                //aktuellesFeld auf GEFAENGNIS setzen
                aktuellesFeld = GEFAENGNIS;
                //Spieler am Zug als im Gefängnis markieren
                spielerInfo[spielerAmZug].gefaengnis = 1;
                //flagWeiter auf 1 setzen, um den Spielzug abzuschliessen
                flagWeiter = 1;
                //flagFertigGewuerfelt auf 1 setzen, um Würfeln zu blockieren
                flagFertigGewuerfelt = 1;
                break;
                //wenn das aktuelle feld Freiparken ist
                case FREIPARKEN:
                //Auf diesem Feld passiert nichts
                break;
                //Wenn das Aktuelle Feld ein Werk ist
                case WERK:
                //Wenn das Aktuelle Feld keinen Besitzer hat
                if (spielfeld[aktuellePosition].besitzer == 0)
                {
                    //flagKaufAbgechlossen auf 0 setzen
                    flagKaufAbgechlossen = 0;
                    //Das aktuelle Feld dem Spieler zum kauf anbieten und den Rückgabewert speichern
                    verkaufSpielerEingabe = feldKaufen(aktuellePosition,spielfeld,spielerAmZug);
                    //Rückgabewert verarbeiten
                    switch (verkaufSpielerEingabe)
                    {
                        case FELD_KAUFEN_KEIN_SPIELER_INPUT://Spieler hat noch keine Entscheidung getroffen
                        break;
                        case FELD_KAUFEN_FELD_GEKAUFT://Spieler hat das Feld gekauft
                        //flagKaufAbgechlossen auf 1 setzen um das Weiterspielen zu ermöglichen
                        flagKaufAbgechlossen = 1;
                        break;
                        case FELD_KAUFEN_FELD_NICHT_GEKAUFT://Spieler hat das Feld nicht gekauft
                        //flagKaufAbgechlossen auf 1 setzen um das Weiterspielen zu ermöglichen
                        flagKaufAbgechlossen = 1;
                        //bezahlStatus auf 1 setzen, ansonsten müsste der Spieler nach der Auktion Miete bezahlen
                        bezahlStatus = 1;
                        //Zum zustand VERSTEIGERUNG wechseln
                        zustand = VERSTEIGERUNG;
                        break;
                    }
                }
                //Überprüfe ob das Feld belastet ist
                feldbelastet = spielfeld[aktuellePosition].feldBelastet;
                //Wenn das aktuelle Feld einem anderen Spieler gehört, der aktuelle Spieler
                //noch nicht bezahlt hat und das Feld nicht belastet ist
                if((spielfeld[aktuellePosition].besitzer && !(spielfeld[aktuellePosition].besitzer == spielerAmZug)) 
                    && !(bezahlStatus == 1) && !feldbelastet)
                {
                    //flagZahlungAbgeschlossen auf 0 setzen um das abschliessen des Spielzuges zu blockieren
                    flagZahlungAbgeschlossen = 0;
                    //Wenn das LCD noch nicht aktualisiert wurde
                    if (!updateLCD)
                    {
                        //Wenn beide Werke dem gleichen Spieler gehören
                        if (spielfeld[FELDNUMMER_WERK1].besitzer == spielfeld[FELDNUMMER_WERK2].besitzer)
                        {
                            //Multiplikator für zwei Werke in der Variabel multiplikator speichern
                            multiplikator = MULTIPLIKATOR_WERK_GRUPPE;
                        }
                        else
                        {
                            //Multiplikator für ein Werk in der Variabel multiplikator speichern
                            multiplikator = MULTIPLIKATOR_WERK_EINZELN;
                        }
                        //Ausgabe am LCD
                        //Spieler am Zug am LCD ausgeben
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);   //spielerAmZug in den lcdBuffer laden
                        writeText(0,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //Tastenbelegung am LCD ausgeben
                        writeText(1,0,"  W"UE"rfeln A / B  ");
                        //Multiplikator am LCD ausgeben
                        writeText(2,0,"W"UE"rfelsumme x   ");
                        sprintf(lcdBuffer,"%u",multiplikator);  //multiplikator in den lcdBuffer laden
                        writeText(2,14,lcdBuffer);              //lcdBuffer am LCD ausgeben

                        //wartet bis mit beiden Würfel gewürfelt wurde
                        /*while (!(flagWuerfel1 && flagWuerfel2))
                        {
                            //flankenerkennung
                            tasteAlt = tasteNeu;
                            tasteNeu = 0;
                            tasteNeu = (PINL << 8) | PINK;
                            positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                            //würfelt würfel 1 nur wenn taste9 betätigt wurde
                            //und würfel 1 noch nicht gewürfelt wurde
                            if ((positiveFlanke & TASTE_A) && !flagWuerfel1)
                            {
                                //mit 1. würfel würfeln
                                wuerfelAB(1,flagWuerfel1,flagWuerfel2);
                                flagWuerfel1 = 1;
                            }
                            //würfelt würfel 2 nur wenn taste9 betätigt wurde
                            //und würfel 2 noch nicht gewürfelt wurde
                            else if ((positiveFlanke & TASTE_B) && !flagWuerfel2)
                            {
                                //mit 2. würfel würfeln
                                wuerfelAB(2,flagWuerfel1,flagWuerfel2);
                                flagWuerfel2 = 1;
                                
                            }
                        }*/
                        //Warte bis mit beiden Würfeln gewürfelt wurde
                        warteBisGewuerfelt();
                        
                        //Feld Besitzer auslesen und Speichern
                        feldBesitzer = spielfeld[aktuellePosition].besitzer;
                        //zahlBetrag anhand von Würfelsumme und Multiplikator berechnen
                        zahlBetrag = (wuerfelArray[0] + wuerfelArray[1]) * multiplikator;
                        //Ausgabe am LCD
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);   //spielerAmZug in den lcdBuffer laden
                        writeText(0,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //Preis der bezahlt werden muss am LCD ausgeben
                        writeText(1,0,"  bezahle       ");
                        sprintf(lcdBuffer,"%u",zahlBetrag);     //zahlBetrag in den lcdBuffer laden
                        writeText(1,10,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //Tastenbelegung und Feldbesitzer am LCD ausgeben
                        writeText(2,0,"an Spieler     X");
                        sprintf(lcdBuffer,"%u",feldBesitzer);   //feldBesitzer in den lcdBuffer laden
                        writeText(2,11,lcdBuffer);              //lcdBuffer am LCD ausgeben
                        //updateLCD auf 1 setzen um erneutes Ausführen zu verhindern
                        updateLCD = 1;
                    }
                    
                    //Wenn der Spieler am Zug, den Betrag, mit seiner Taste X bestätigt hat
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        //Den zu bezahlenden Betrag an den Feldbesitzer überweisen
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag);
                        //Wenn die Zahlung erfolgreich war
                        if (bezahlStatus == 1)
                        {
                            //updateLCD auf 0 zurücksetzen
                            updateLCD = 0;
                            //flagZahlungAbgeschlossen auf 1 setzen um es zu ermöglichen den Spielzug abzuschliessen
                            flagZahlungAbgeschlossen = 1;
                            //flagSpielLCD auf 1 setzen um die Standart LCD Maske einzublenden
                            flagSpielLCD = 1;
                            //flagWuerfel1 auf 0 zurücksetzen
                            flagWuerfel1 = 0;
                            //flagWuerfel2 auf 0 zurücksetzen
                            flagWuerfel2 = 0;
                        }
                    }
                }
                
                break;
            }
            break;
            //Zustand indem Versteigerungen stattfinden
            case VERSTEIGERUNG:
            //Wenn das LCD noch nicht aktualisiert wurde
            if (!updateLCD)
            {
                //LCD ausgabe
                //LCD leerern
                clear();
                //"VERSTEIGERUNG" am LCD ausgeben
                writeText(0,0," VERSTEIGERUNG  ");
                //Name des Feldes, dass versteigert wird am LCD ausgeben
                writeText(1,0,spielfeld[aktuellePosition].name);
                //Tastenbelegung am LCD ausgeben
                writeText(2,0,"bieten X sonst Y");
                //Die Variable aktuellesGebot auf 9 setzen
                aktuellesGebot = 9; //startgebot
                //updateLCD auf 1 setzen, um erneute Ausgabe zu verhindern
                updateLCD = 1; //LCD nicht mehr aktualisiern
                //Erhöhe i um 1, solange i kleiner als anzahlSpieler ist. Starte mit i = 1
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    //Speichert den aktuellen Kontostand des Spielers "i"
                    geldZwischenspeicher[i] = spielerInfo[i].geld;
                    //Geld Siebensegmente ausschalten
                    //Konto Siebensegment des Spielers "i" ausschalten
                    setGeld(0,i,0);
                }
            }
            //Erhöhe i um 1, solange i <= als anzahlSpieler ist. Starte mit i = 0
            for (uint8_t i = 0; i < anzahlSpieler; i = i + 1)
            {
                //Wenn der Spieler "i" seine Taste X betätigt hat, er noc am Bieten ist 
                //und genug Geld hat um das Gebot zu erhöhen
                if (((positiveFlanke & xTasten[i]) && !bieter[i]) && (spielerInfo[i+1].geld > (aktuellesGebot))) 
                {
                    //aktuelles Gebot um 1 erhöhen
                    aktuellesGebot = aktuellesGebot + 1; //aktuelles Gebot erhöhen
                    bieter[5] = i + 1;
                    //Erhöhe j um 1, solange j <= als anzahlSpieler ist. Starte mit j = 1
                    //Siebensegment aller spieler ausschalten
                    for (uint8_t j = 1; j <= anzahlSpieler; j = j + 1)
                    {
                        //Konto Siebensegment des Spielers "j" ausschalten
                        //Wenn der Spieler noch am bieten ist und nicht der Höchstbieter ist
                        if (!bieter[j - 1] && !(bieter[5] == j))
                        {
                            //wenn der Spieler kein Gebot abgegeben hat aber
                            //noch mitbietet siebensegmente abschalten
                            setGeld(0,j,0);//Das Konto Siebensegment ausschalten
                        }
                        //setGeld(0,j,0); //Geld Siebensegmente ausschalten
                    }
                    //Aktuelles Gebot am Siebensegment des Höchstbieters anzeigen
                    setGeld(aktuellesGebot,i + 1,1); 
                    //Die Spielernummer des Höchstbieters speichern
                    //bieter[HOECHSTBIETER] = i + 1; //Speichert Spieler Nummer vom Höchstbieter 
                }
                //wenn ein spieler die Taste Y betätigt bietet er nicht mehr mit
                //Wenn der Spieler "i" seine Taste Y betätigt hat und er noch nicht zurückgetreten ist
                if ((positiveFlanke & yTasten[i]) && !bieter[i] && !(positiveFlanke & yTasten[bieter[5] - 1]))
                {
                    setGeld(0,i + 1,2);//---- am Siebensegment anzeigen
                    
                    //Den Spieler "j" als zurückgetreten markieren
                    bieter[i] = 1; //schliesst spieler aus auktion aus
                    //Die anzahl zurückgetretenen Spieler um 1 erhöhen
                    bieter[ANZAHL_ZURUECKGETRETENE_SPIELER] = bieter[ANZAHL_ZURUECKGETRETENE_SPIELER] + 1;
                }
            }
            //Wenn alle Spieler zurückgetreten sind
            if ((bieter[ANZAHL_ZURUECKGETRETENE_SPIELER] == anzahlSpieler - 1))
            {
                if (!(bieter[HOECHSTBIETER] == 0)) //wenn jemand die Auktion gewonnen hat
                {
                    //LCD Leeren und neu beschreiben
                    //LCD leeren
                    clear();
                    //Am LCD ausgeben, an welchen Spieler das Feld versteigert wurde
                    writeText(0,0," VERSTEIGERT an ");
                    writeText(1,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",bieter[HOECHSTBIETER]);  //Hoechstbieter in den lcdBuffer laden
                    writeText(1,11,lcdBuffer);                      //lcdBuffer am LCD ausgeben
                    //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                    _delay_ms(3000);    
                    //Geld aus Zwischenspeicher zurück auf Spielerkonto
                    //Erhöhe i um 1, solange i kleiner als anzahlSpieler ist. Starte mit i = 1
                    for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                    {
                        //Zuvor gespeicherter Kontostand, auf das Konto der Spieler zurückschreiben
                        spielerInfo[i].geld = geldZwischenspeicher[i];
                    }
                    //Kontostand aktualisieren
                    updateKontostand(anzahlSpieler,spielerInfo,0);
                    //kontostand von Höchstbieter um gebot verkleinern
                    //spielerInfo[bieter[HOECHSTBIETER]].geld = (spielerInfo[bieter[5]].geld - aktuellesGebot);
                    //Betrag wird vom Konto des Höchstbieters abgezogen
                    geldUeberweisen(bieter[HOECHSTBIETER],0,aktuellesGebot);
                    //Das Feld an den neuen Besitzer übertragen
                    spielfeld[aktuellePosition].besitzer = bieter[HOECHSTBIETER];
                    //Die RGB LED des Feldes auf die Farbe des neuen Besitzers setzen
                    setPropertyRgb(spielfeld[aktuellePosition].rgbNummer,bieter[HOECHSTBIETER]);
                    //bieter array zurücksetzen
                    //Erhöhe i um 1, solange i kleiner als 6 ist. Starte mit i = 0
                    for (uint8_t i = 0; i < 6; i = i + 1)
                    {
                        //Versteigerungs Zwischenspeicher zurücksetzen
                        bieter[i] = 0;
                    }
                    //updateLCD auf 0 zurücksetzen
                    updateLCD = 0;
                    //flagVersteigert = 1;
                    //flagSpielLCD auf 1 setzen, um die Standart LCD maske einzublenden
                    flagSpielLCD = 1;
                    //zum spiel zurückkehren
                    zustand = SPIEL;
                }
                else
                {
                    //LCD Leeren und neu beschreiben
                    //LCD leeren
                    clear();
                    //"nicht Versteigert" am LCD ausgeben
                    writeText(0,0,"     nicht     ");
                    writeText(1,0,"   VERSTEIGERT  ");
                    //Programm für 3 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                    _delay_ms(3000);
                    //Geld aus Zwischenspeicher zurück auf Spielerkonto
                    //Erhöhe i um 1, solange i kleiner als anzahlSpieler ist. Starte mit i = 1
                    for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                    {
                        //Zuvor gespeicherter Kontostand, auf das Konto der Spieler zurückschreiben
                        spielerInfo[i].geld = geldZwischenspeicher[i];
                    }
                    //Kontostand aktualisieren
                    updateKontostand(anzahlSpieler,spielerInfo,spielerAmZug);
                    //bieter array zurücksetzen
                    //Erhöhe i um 1, solange i kleiner als 6 ist. Starte mit i = 0
                    for (uint8_t i = 0; i < 6; i = i + 1)
                    {
                        //Versteigerungs Zwischenspeicher zurücksetzen
                        bieter[i] = 0;
                    }
                    //updateLCD auf 0 zurücksetzen
                    updateLCD = 0;
                    //flagSpielLCD auf 1 setzen, um die Standart LCD maske einzublenden
                    flagSpielLCD = 1;
                    //zum Zustand SPIEL wechseln
                    zustand = SPIEL;
                }
            }
            break;
            case BAUEN://Zustand indem gebaut wird
            //Der Spieler kann Häuser Kaufen oder Verkaufen
            hausKaufenVerkaufen(spielerAmZug);
            //flagSpielLCD auf 1 setzen um Standart LCD Maske anzuzeigen
            flagSpielLCD = 1;
            //Zum zustand SPIEL wechseln
            zustand = SPIEL;
            break;
            //Zustand durchden man versteigern, Bauen und Handeln kann
            case VERWALTEN://in diesem zustand, kann man in andere zustände rein navigieren
            switch (verwaltung)
            {
                case VERWALTUNG_BAUEN:
                //Wenn das LCD noch nicht aktualisiert wurde
                if (!updateLCD)
                {
                    //lcd ausgabe
                    //"Verwaltung" am LCD ausgeben
                    writeText(0,0,"   Verwaltung   ");
                    //"Bauen?" am LCD Ausgeben
                    writeText(1,0,"     Bauen?     ");
                    //Tastenbelegung am LCD ausgeben
                    writeText(2,0,"C zur"UE"ck|weiter"PFEIL_R);
                    //updateLCD auf 1 setzen um erneute Ausgabe zu blockieren
                    updateLCD = 1;
                }
                //Wenn Taste R betätigt wurde
                if (positiveFlanke & TASTE_R)
                {
                    //zustand wechseln
                    //verwaltung auf HYPOTHEK setzen
                    verwaltung = HYPOTHEK;
                    //updateLCD auf 0 zurücksetzen
                    updateLCD = 0;
                }
                //Wenn Taste S betätigt wurde
                else if (positiveFlanke & TASTE_S)
                {
                    //verwaltung auf VERWALTUNG_BAUEN zurücksetzen
                    verwaltung = VERWALTUNG_BAUEN;
                    //Zum zustand Bauen wechseln;
                    zustand = BAUEN;
                    //updateLCD auf 0 zurücksetzen
                    updateLCD = 0;
                }
                //Wenn Taste C betätigt wurde
                else if (positiveFlanke & TASTE_C)
                {
                    //Zum zustand SPIEL wechseln
                    zustand = SPIEL;
                    //flagSpielLCD auf 1 setzen um Standart LCD Maske anzuzeigen
                    flagSpielLCD = 1;
                }
                break;
                case HYPOTHEK:
                //Wenn das LCD noch nicht aktualisiert wurde
                if (!updateLCD)
                {
                    //lcd ausgabe
                    //"Verwaltung" am LCD ausgeben
                    writeText(0,0,"   Verwaltung   ");
                    //"Hypothek" am LCD Ausgeben
                    writeText(1,0,"    Hypothek    ");
                    //Tastenbelegung am LCD ausgeben
                    writeText(2,0,"C zur"UE"ck|weiter"PFEIL_R);
                    //updateLCD auf 1 setzen um erneute Ausgabe zu blockieren
                    updateLCD = 1;
                }
                //Wenn Taste R betätigt wurde
                if (positiveFlanke & TASTE_R)
                {
                    //zustand wechseln
                    //verwaltung auf VERWALTUNG_HANDELN setzen
                    verwaltung = VERWALTUNG_HANDELN;
                    //updateLCD auf 0 zurücksetzen
                    updateLCD = 0;
                }
                //Wenn Taste S betätigt wurde
                else if (positiveFlanke & TASTE_S)
                {
                    //zurück zum Spiel
                    verwaltung = VERWALTUNG_BAUEN;
                    //Zum zustand VERPFAENDEN wechseln
                    zustand = VERPFAENDEN;
                    //updateLCD auf 0 zurücksetzen
                    updateLCD = 0;
                }
                //Wenn Taste C betätigt wurde
                else if (positiveFlanke & TASTE_C)
                {
                    //Zum zustand SPIEL wechseln
                    zustand = SPIEL;
                    //flagSpielLCD auf 1 setzen um Standart LCD Maske anzuzeigen
                    flagSpielLCD = 1;
                }
                break;
                case VERWALTUNG_HANDELN:
                //Wenn das LCD noch nicht aktualisiert wurde
                if (!updateLCD)
                {
                    //lcd ausgabe
                    //"Verwaltung" am LCD ausgeben
                    writeText(0,0,"   Verwaltung   ");
                    //"Handeln" am LCD Ausgeben
                    writeText(1,0,"    Handeln     ");
                    //Tastenbelegung am LCD ausgeben
                    writeText(2,0,"C zur"UE"ck|weiter"PFEIL_R);
                    //updateLCD auf 1 setzen um erneute Ausgabe zu blockieren
                    updateLCD = 1;
                }
                //Wenn Taste R betätigt wurde
                if (positiveFlanke & TASTE_R)
                {
                    //zustand wechseln
                    //verwaltung auf VERWALTUNG_HANDELN setzen
                    verwaltung = VERWALTUNG_BAUEN;
                    //updateLCD auf 0 zurücksetzen
                    updateLCD = 0;
                }
                //Wenn Taste S betätigt wurde
                else if (positiveFlanke & TASTE_S)
                {
                    //zurück zum Spiel
                    //verwaltung auf VERWALTUNG_BAUEN zurücksetzen
                    verwaltung = VERWALTUNG_BAUEN;
                    //Zum zustand HANDELN wechseln
                    zustand = HANDELN;
                    //updateLCD auf 0 zurücksetzen
                    updateLCD = 0;
                }
                //Wenn Taste C betätigt wurde
                else if (positiveFlanke & TASTE_C)
                {
                    //Zum zustand SPIEL wechseln
                    zustand = SPIEL;
                    //flagSpielLCD auf 1 setzen um Standart LCD Maske anzuzeigen
                    flagSpielLCD = 1;
                }
                break;
                default:
                break;
            }
            break;
            //Zustand indem man eine Hypothek aufnehmen oder auflösen kann
            case VERPFAENDEN:
            //Wenn das LCD noch nicht aktualisiert wurde
            if (!updateLCD)
            {
                //lcd Ausgabe
                //Tastenbelegung am LCD ausgeben
                writeText(0,0,"                ");
                writeText(1,0,PFEIL_O"Hyp.|Hyp.aufl."PFEIL_U);
                writeText(2,0,"C zur"UE"ck|weiter"PFEIL_R);
                //updateLcd auf 1 setzen um erneute ausgabe zu blockieren
                updateLCD = 1;
                //Spielerinventar zurücksetzen
                //Erhöhe i um 1, solange i kleiner als GROESSE_SPIELER_INVENTAR ist. Starte mit i = 0
                for (uint8_t i = 0; i < SPIELER_INVENTAR_GROESSE; i = i + 1)
                {
                    //spielerInventar an der Position "i" auf 0 zurücksetzen
                    spielerInventar[i] = 0;
                }
                anzahlEigentum = 0;
                //sucht alle felder ab nach Felder die dem Spieler gehören
                for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
                {
                    //sucht die spielfelder nach denen ab, die dem Spieler gehören
                    //Wenn das Feld "i" dem Spieler am Zug gehört
                    if (spielfeld[i].besitzer == spielerAmZug)
                    {
                        //Die Feldnummer des aktuellen Feldes im spielerInventar speichern
                        spielerInventar[anzahlEigentum] = i;
                        //anzahlEigentum aum 1 erhöhen
                        anzahlEigentum += 1;
                    }
                }
                //Schreibt den namen des ersten feldes auf das LCD
                //Den Namen des ersten Feldes auf dem LCD ausgeben
                writeText(0,0,"                ");
                writeText(0,0,spielfeld[spielerInventar[feldZaehler]].name);
            }
            //nächstes Feld mit taste R
            //Wenn die Taste R betätigt wurde
            if (positiveFlanke & TASTE_R)
            {
                //Den Feld zähler erhöhen
                feldZaehler = (feldZaehler + 1) % anzahlEigentum;
                //Schreibt den Namen des Feldes auf das LCD
                //Löscht die oberste Zeile auf dem LCD
                writeText(0,0,"                ");
                //Den Namen des nächsten Feldes am LCD ausgeben
                writeText(0,0,spielfeld[spielerInventar[feldZaehler]].name);
            }
            //wenn spieler Feld verpfänden will
            //Wenn die Taste O betätigt wurde und Somit das Feld verpfändet werden soll
            if (positiveFlanke & TASTE_O)
            {
                //flagKeineHaeuser auf 1 setzen
                flagKeineHaeuser = 1;
                //überprüft ob es auf allen 3 Feldern keine Häuser hat
                //Erhöhe i um 1, solange i kleiner als ANZAHL_FELDER_IN_FARBGRUPPE ist. Starte mit i = 0
                for (uint8_t i = 0; i < ANZAHL_FELDER_IN_FARBGRUPPE; i = i + 1)
                {
                    //Wenn es auf dem Feld "i" der Farbgruppe ein Haus hat
                    if (spielfeld[spielfeld[spielerInventar[feldZaehler]].farbgruppenFelder[i]].anzahlHaeuser)
                    {
                        //flagKeineHaeuser auf 0 setzen somit kann kein Feld der Farbgruppe verpfändet werden
                        flagKeineHaeuser = 0;
                    }
                }
                //prüft ob es auf dem Feld noch Häuser hatt und ob das Feld bereits belastet ist
                //Wenn das aktuelle Feld nicht belastet ist und flagKeineHaeuser 1 ist
                if (!(spielfeld[spielerInventar[feldZaehler]].feldBelastet) && flagKeineHaeuser)
                {
                    //Den Wert des Feldes berechnen
                    zahlBetrag = spielfeld[spielerInventar[feldZaehler]].preis / 2;
                    //Den berechneten Wert an den Spieler überweisen
                    bezahlStatus = geldUeberweisen(0,spielerAmZug,zahlBetrag);
                    //Wenn die überweisung erfolgreich war
                    if (bezahlStatus == 1) //wenn die Bezahlung erfolgreich war
                    {
                        //Das aktuelle Feld als belastet markieren
                        spielfeld[spielerInventar[feldZaehler]].feldBelastet = 1;
                        //hausnummer auf 0 zurücksetzen
                        hausNummer = 0; //~~~~~~~~~~~~Kann ziemlich sicher gelöscht werden~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        //Die Hausnummer des Feldes aus dem Speicher holen
                        hausNummer = spielfeld[spielerInventar[feldZaehler]].hausnummer;
                        //Die RGBnummer des Feldes aus dem Speicher holen
                        rgbFeldNummer = spielfeld[spielerInventar[feldZaehler]].rgbNummer;
                        //Wenn das Aktuelle Feld eine Strasse ist
                        if (spielfeld[spielerInventar[feldZaehler]].typ == STRASSE)
                        {
                            //Markiert das Feld als verpfändet
                            //alle 5 haus LEDs werden eingeschaltet
                            //Alle Haus LEDs einschalten, um das Feld als belastet zu markieren
                            setHaus(hausNummer,MARKIERUNG_STRASSE_BELASTET);
                        }
                        else //wenn es keine strasse ist
                        {
                            //wenn es keine häuser hat, die man als Markierung nutzen kann
                            // wird die RGB auf weiss gestellt.
                            //Die RGB LED des Feldes auf weiss setzen, um das Feld als belastet zu markieren
                            setPropertyRgb(rgbFeldNummer,MARKIERUNG_RGB_FELD_BELASTET);
                        }
                    }
                }
            }
            //Wenn die Taste U betätigt wurde und somit die Hypothek aufgelöst werden soll
            else if (positiveFlanke & TASTE_U) //wenn der Spieler die hypothek auflösen will
            {
                //Wenn das Feld belastet ist
                if (spielfeld[spielerInventar[feldZaehler]].feldBelastet)
                {
                    //Berechnet den Preis, der bezahlt werden muss um die Hypothek aufzulösen
                    zahlBetrag = (spielfeld[spielerInventar[feldZaehler]].preis / 2) * 1.1;
                    //Das Geld an die Bank überweisen
                    bezahlStatus = geldUeberweisen(spielerAmZug,0,zahlBetrag);
                    //Wenn die Überweisung erfolgreich war
                    if (bezahlStatus == 1)
                    {
                        //Markiert das Feld als nicht mehr belastet
                        spielfeld[spielerInventar[feldZaehler]].feldBelastet = 0;
                        //hausnummer auf 0 zurücksetzen
                        hausNummer = 0;
                        //Die Hausnummer des Feldes aus dem Speicher holen
                        hausNummer = spielfeld[spielerInventar[feldZaehler]].hausnummer;
                        //Die RGBnummer des Feldes aus dem Speicher holen
                        rgbFeldNummer = spielfeld[spielerInventar[feldZaehler]].rgbNummer;
                        if (spielfeld[spielerInventar[feldZaehler]].typ == STRASSE)//wenn es eine Strase ist
                        {
                            //Alle Haus LEDs ausschalten, um das Feld als nicht mehr belastet zu markieren
                            setHaus(hausNummer,0);
                        }
                        else //wenn es keine strasse ist
                        {
                            //Die RGB LED des Feldes auf die Farbe des Spielers am Zug setzen,
                            //um das Feld als nicht mehr belastet zu markieren
                            setPropertyRgb(rgbFeldNummer,spielerAmZug);
                        }
                    }
                }
                
            }
            //wenn taste c gedrückt wurde, gelangt man zurück zum Spiel
            if (positiveFlanke & TASTE_C)
            {
                //Zum Zustand SPIEL wechseln
                zustand = SPIEL;
                //flagSpielLCD auf 1 setzen um sie Standart LCD Maske anzuzeigen
                flagSpielLCD = 1;
            }
            
            break;
            case HANDELN://zustand in dem gehandelt wird
            switch (handelZustand)
            {
                case HAENDLER_AUSWAHL://auswählen, wer mit wem handelt
                //Wenn das LCD noch nicht aktualisiert wurde
                if (!updateLCD)//LCD einmal schreiben
                {
                    //lcd ausgabe
                    //"Beide Händler drücken die Taste X" am LCD ausgeben 
                    writeText(0,0,"beide h"AE"ndler   ");
                    writeText(1,0,"taste x dr"UE"cken ");
                    writeText(2,0,"                ");
                    //updateLCD auf 1 setzen um erneute Ausgabe zu verhindern
                    updateLCD = 1;
                    //haendlerZaehler auf 0 zurücksetzen
                    haendlerZaehler = 0;
                }
                //for schleife fragt alle x tasten ab
                //Erhöhe i um 1, solange i <= anzahlSpieler ist. Starte mit i = 1
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    //überprüft die x tasten und stellt sicher dass der 1. händler nicht nochmal gedrückt hat
                    //Wenn der Spieler "i" seine Taste X betätigt hat 
                    //und er nicht bereits als Händler registriert wurde
                    if ((positiveFlanke & xTasten[i - 1]) && !(i == handel[0].spielerNr))
                    {
                        //Die Spielernummer speichern
                        handel[haendlerZaehler].spielerNr = i;//speichert die Spielernummer
                        //haendlerZaehler um 1 vergrössern
                        haendlerZaehler += 1;
                    }
                }
                //prüft ob im 2.speicher ein händler eingetragen ist
                //Wenn zwei Händler registriert sind
                if (handel[1].spielerNr)
                {
                    //globalUpdateLCD auf 0 zurücksetzen
                    globalUpdateLCD = 0;
                    //handelZustand auf WARE_AUSWAEHLEN setzen
                    handelZustand = WARE_AUSWAEHLEN;//zustandswechsel
                    //haendlerZaehler auf 0 zurücksetzen
                    haendlerZaehler = 0;
                }
            	break;
                case WARE_AUSWAEHLEN://ware die gehandelt werden soll auswählen
                //Solange nicht beide Spieler mit ihrer Auswahl fertig sind
                while (haendlerZaehler < 2)
                {
                    //lässt die Spieler die zu handelnde Ware auswählen 
                    auswahlAbgeschlossen = handelWareAuswaehlen(haendlerZaehler);
                    //Wenn die Auswahl abgeschlossen ist
                    if (auswahlAbgeschlossen)
                    {
                        //haendlerZaehler um 1 erhöhen
                        haendlerZaehler += 1;
                        auswahlAbgeschlossen = 0;
                    }
                }
                //wenn der 2. Händler die Auswahl abgeschlossen hat
                //Wenn haendlerZaehler den Wert 2 hat
                if (haendlerZaehler == 2)
                {
                    //handel abschliessen
                    //handelZustand auf HANDEL_BESTAETIGEN setzen
                    handelZustand = HANDEL_BESTAETIGEN;//zustandswechsel
                    //globalUpdateLCD auf 0 zurücksetzen
                    globalUpdateLCD = 0;
                    //haendlerZaehler auf 0 zurücksetzen
                    haendlerZaehler = 0;
                    //auswahlAbgeschlossen auf 0 zurücksetzen
                    auswahlAbgeschlossen = 0;
                }
                break;
                case HANDEL_BESTAETIGEN://handel bestätigen
                //blaulicht(100,10);
                //lcd ausgabe
                //"Bestätigen" am LCD ausgeben
                writeText(1,0,"   Best"AE"tigen   ");
                //Lässt die Händler die Auswahl bestätigen
                auswahlAbgeschlossen = auswahlBestaetigen(haendlerZaehler);
                //Wenn die Auswahl bestätigt wurde
                if (auswahlAbgeschlossen)
                {
                    //haendlerZaehler um 1 erhöhen
                    haendlerZaehler += 1;//zähler erhöhen
                    //auswahlAbgeschlossen auf 0 zurücksetzen
                    auswahlAbgeschlossen = 0;
                    //Wenn beide Händler alles bestätigt haben
                    if (haendlerZaehler == 2)
                    {
                        //Besitz übertragen
                        //handelZustand auf BESITZ_UEBERTRAGEN setzen
                        handelZustand = BESITZ_UEBERTRAGEN;//zustandswechsel
                        //globalUpdateLCD auf 0 zurücksetzen
                        globalUpdateLCD = 0;
                    }
                }
                else
                {
                    //handelZustand auf HANDEL_ABSCHLIESSEN setzen
                    //Die Verhandlung wird abgebrochen
                    handelZustand = HANDEL_ABSCHLIESSEN;
                }
                
                break;
                case BESITZ_UEBERTRAGEN://ausgewählte ware übertragen
                //lcd ausgabe
                //LCD leeren
                clear();
                //_delay_ms(1000);
                //"Übertragen" am LCD anzeigen
                writeText(0,0,"   "UE"bertragen   ");
                //Programm für 1 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                _delay_ms(1000);
                //Die Spielernummer des ersten Händlers speichern
                handelSpielerNummern[0] = handel[0].spielerNr;
                //Die Spielernummer des zweiten Händlers speichern
                handelSpielerNummern[1] = handel[1].spielerNr;
                //Grundstücke übertragen
                //Erhöhe i um 1, solange i kleiner als ANZAHL_HAENDLER ist. Starte mit i = 0
                for (uint8_t i = 0; i < ANZAHL_HAENDLER; i = i + 1)
                {
                    //Erhöhe j um 1, solange es noch Felder zu übertragen gibt. Starte mit j = 0
                    for (uint8_t j = 0; handel[i].feldNummern[j] > 0; j = j + 1)
                    {
                        //Speichert die zu übertragende Feldnummer 
                        handelFeldNummer = handel[i].feldNummern[j];
                        //Überträgt das Feld an den neuen Besitzer
                        spielfeld[handelFeldNummer].besitzer = handelSpielerNummern[1 - i];//Besitz übertragen
                        //Wenn das Feld belastet ist
                        if (spielfeld[handelFeldNummer].feldBelastet)
                        {
                            //flagHandelBelastet auf 1 setzen
                            flagHandelBelastet = 1;
                            //globalUpdateLCD auf 0 setzen
                            globalUpdateLCD = 0;
                            //Solange der Neue besitzer noch nicht entschieden hat, was er mit der Hypothek macht
                            while(flagHandelBelastet)
                            {
                                //Flankenerkennung
                                //Tasten einlesen und Positive Flanken bestimmen
                                tasteAlt = tasteNeu;
                                tasteNeu = 0;
                                tasteNeu = (PINL << 8) | PINK;
                                positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                                //Wenn das LCD noch nicht aktualisiert wurde
                                if (!globalUpdateLCD)
                                {
                                    //lcd ausgabe
                                    //Spieler Nummer des aktuellen Händlers am LCD ausgeben
                                    writeText(0,0,"   Spieler      ");
                                    //handelSpielerNummern[1 - i] in den lcdBuffer laden
                                    sprintf(lcdBuffer,"%u",handelSpielerNummern[1 - i]);
                                    writeText(0,11,lcdBuffer);  //lcdBuffer am LCD ausgeben
                                    //"Hypothek auflösen" und dazugehörige Taste am LCD ausgeben
                                    writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                                    //"Andere Option" und dazugehörige Taste am LCD ausgeben
                                    writeText(2,0,"andere option  "PFEIL_U);
                                    //globalUpdateLCD auf 1 setzen, um erneute Ausgabe zu verhindern
                                    globalUpdateLCD = 1;
                                }
                                //Wenn die Taste O betätigt wurde
                                if (positiveFlanke & TASTE_O)
                                {
                                    //Wenn die Hypothek aufgelöst werden soll
                                    if (!handelHypothek)//Hypothek soll aufgelöst werden
                                    {
                                        //Betrag berechnen, der bezahlt werden muss um die Hypothek aufzulösen
                                        handelBezahlBetrag = spielfeld[handelFeldNummer].preis;
                                        //Betrag berechnen, der bezahlt werden muss um die Hypothek aufzulösen
                                        handelBezahlBetrag = (handelBezahlBetrag / 2) + (handelBezahlBetrag / 20);
                                        //Den zu bezahlenden Betrag am LCD ausgeben
                                        writeText(1,0,"Zahle          S");
                                        //handelBezahlBetrag in den lcdBuffer laden
                                        sprintf(lcdBuffer,"%u",handelBezahlBetrag);
                                        //lcdBuffer am LCD ausgeben
                                        writeText(1,6,lcdBuffer);
                                    }
                                    else
                                    {
                                        //Betrag berechnen, der bezahlt werden muss
                                        //um die Hypothek zu behalten
                                        handelBezahlBetrag = spielfeld[handelFeldNummer].preis;
                                        //Betrag berechnen, der bezahlt werden muss
                                        //um die Hypothek zu behalten
                                        handelBezahlBetrag = (handelBezahlBetrag / 20);
                                        //Den zu bezahlenden Betrag am LCD ausgeben
                                        writeText(1,0,"Zahle          S");
                                        //handelBezahlBetrag in den lcdBuffer laden
                                        sprintf(lcdBuffer,"%u",handelBezahlBetrag);
                                        //lcdBuffer am LCD ausgeben
                                        writeText(1,6,lcdBuffer);
                                    }
                                    //warte bis der Spieler mit Taste X bestätigt hat
                                    //Solange der Spieler noch keine entscheidung getroffen hat
                                    while (!(positiveFlanke & TASTE_S) && !(positiveFlanke & TASTE_U))
                                    {
                                        //Flankenerkennung
                                        //Tasten einlesen und Positive Flanken bestimmen
                                        tasteAlt = tasteNeu;
                                        tasteNeu = 0;
                                        tasteNeu = (PINL << 8) | PINK;
                                        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                                    }
                                    //Wenn der Spieler die Taste S betätigt hat
                                    //und somit bezahlen will
                                    if (positiveFlanke & TASTE_S)
                                    {
                                        //geld überweisen
                                        handelzahlungErfolgreich = geldUeberweisen(handelSpielerNummern[1 - i],0,handelBezahlBetrag);
                                    }
                                    else
                                    {
                                        //ändernt den Zustand der aktuellen option
                                        handelHypothek = (handelHypothek + 1) % 2;
                                        //Wenn die neue Option Hypothek behalten ist
                                        if (handelHypothek)
                                        {
                                            //lcd ausgabe
                                            //"Hypothek behalten" und dazugehörige Taste am LCD ausgeben
                                            writeText(1,0,"Hyp. Behalten  "PFEIL_O);
                                        }
                                        else
                                        {
                                            //lcd ausgabe
                                            //"Hypothek auflösen" und dazugehörige Taste am LCD ausgeben
                                            writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                                        }
                                    }
                                    //Wenn die Zahlung erfolgreich war
                                    if (handelzahlungErfolgreich == 1)
                                    {
                                        //Wenn die Hypothek aufgelöst werden soll
                                        if (!handelHypothek)
                                        {
                                            //Feld als nicht mehr belastet speichern
                                            spielfeld[handelFeldNummer].feldBelastet = 0;
                                            //Alle Haus LEDs auf dem Feld ausschalten
                                            setHaus(spielfeld[handelFeldNummer].hausnummer,0);
                                        }
                                        //flagHandelBelastet auf 0 zurücksetzen um zu SIgnalisieren, 
                                        //dass eine Entscheidung getroffen wurde
                                        flagHandelBelastet = 0;
                                        //handelzahlungErfolgreich auf 0 zurücksetzen
                                        handelzahlungErfolgreich = 0;
                                    }
                                    
                                }
                                //Wenn die Taste U betätigt wurde
                                else if (positiveFlanke & TASTE_U)
                                {
                                    //Zur nächsten Option wechseln
                                    handelHypothek = (handelHypothek + 1) % 2;
                                    //Wenn die neue Option Hypothek behalten ist
                                    if (handelHypothek)
                                    {
                                        //lcd ausgabe
                                        //"Hypothek behalten" und dazugehörige Taste am LCD ausgeben
                                        writeText(1,0,"Hyp. Behalten  "PFEIL_O);
                                    }
                                    else
                                    {
                                        //lcd ausgabe
                                        //"Hypothek auflösen" und dazugehörige Taste am LCD ausgeben
                                        writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                                    }
                                }
                            }
                        }
                        //Die RGB LED des Feldes auf die Farbe des neuen Besitzers setzen
                        setPropertyRgb(spielfeld[handelFeldNummer].rgbNummer, spielfeld[handelFeldNummer].besitzer);
                    }
                }
                for (uint8_t i = 0; i < ANZAHL_HAENDLER; i = i + 1)
                {
                    //Wenn bargeld gehandelt wurde
                    if (handel[i].barGeld)
                    {
                        //überweist das Geld
                        //Den ausgewählten Betrag an den anderen Händler überweisen
                        geldUeberweisen(handelSpielerNummern[i],handelSpielerNummern[1 - i],handel[i].barGeld);
                    }
                }
                for (uint8_t i = 0; i < ANZAHL_HAENDLER; i = i + 1)
                {
                    if(handel[i].freikarte)//prüft ob Freikarten gehandelt werden
                    {
                        //Die Anzahl Freikarten des einten Händlers um 1 verkleinern
                        spielerInfo[handelSpielerNummern[i]].freikarte -= 1;
                        //Die Anzahl Freikarten des anderen Händlers um 1 vergrössern
                        spielerInfo[handelSpielerNummern[1 - i]].freikarte += 1;
                    }
                }
                //handelZustand auf HANDEL_ABSCHLIESSEN setzen
                handelZustand = HANDEL_ABSCHLIESSEN;
                break;
                case HANDEL_ABSCHLIESSEN://handel abschliessen
                //lcd ausgabe
                //"Handel abgeschlossen" am LCD ausgeben
                writeText(0,0,"     Handel     ");
                writeText(0,0," abgeschlossen  ");
                //Programm für 1 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                _delay_ms(1000);
                //flagSpielLCD auf 1 setzen um STandart LCD Maske anzuzeigen
                flagSpielLCD = 1;
                //Handelinventar zurücksetzen
                initialisiereHandelInventar(handel);
                //handelZustand auf HAENDLER_AUSWAHL zurücksetzen
                handelZustand = HAENDLER_AUSWAHL;
                //zum zustand SPIEL wechseln
                zustand = SPIEL;
                break;
                default:
                break;
            }
            break;
            default:
            break;
        }
    }
}

/******************************************************************************\
* feldKaufen
*
* Diese Funktion ermöglicht es einem Spieler, ein Spielfeld zu kaufen,
* wenn er genügend Geld besitzt. Die Funktion überprüft die Eingabe des
* Spielers und aktualisiert die Besitzverhältnisse und den Kontostand.
*
* Parameter:
* feldNummer   = Nummer des Spielfelds, das gekauft werden soll
* spielfeld    = Array der Spielfelder
* spielerAmZug = Nummer des aktuellen Spielers
*
* Rückgabewert:
* 1 = Feld wurde gekauft
* 2 = Feld wurde nicht gekauft (Versteigerung)
*
\******************************************************************************/
uint8_t feldKaufen(uint8_t feldNummer, Feld spielfeld[40], uint8_t spielerAmZug)
{
    char lcdBuffer[16];
    uint8_t spielerEingabe = 0;
        if (!globalUpdateLCD) //LCD 1 mal aktualisieren
        {
            //lcd ausgabe
            clear();
            _delay_ms(100);
            writeText(0,0,"   Spieler      ");
            sprintf(lcdBuffer,"%u",spielerAmZug);
            writeText(0,11,lcdBuffer);
            writeText(1,0,spielfeld[feldNummer].name);
            writeText(2,0,"kaufen?"PFEIL_L"Nein|Ja"PFEIL_R"");
            globalUpdateLCD = 1;
        }
        if (positiveFlanke & TASTE_R)//kaufen
        {
            //wenn der Spieler genug geld hat, kann er es kaufen
            if(spielerInfo[spielerAmZug].geld >= spielfeld[feldNummer].preis)
            {
                //zieht den betrag vom Konto des spielers ab
                geldUeberweisen(spielerAmZug,0,spielfeld[feldNummer].preis);
                //spielerInfo[spielerAmZug].geld = spielerInfo[spielerAmZug].geld - spielfeld[feldNummer].preis;
                //besitz wird umgeschrieben
                spielfeld[feldNummer].besitzer = spielerAmZug;
                //Besitz RGB setzen
                setPropertyRgb(spielfeld[feldNummer].rgbNummer,spielerAmZug);
                //konto aktualisieren
                updateKontostand(anzahlSpieler,spielerInfo,spielerAmZug);
                //LCD leeren und neu beschreiben
                clear();
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"    weiter C    ");
                spielerEingabe = 1; //Rückgabewert 2 = Feld wurde gekauft
                globalUpdateLCD = 0;
            }
        }
        else if (positiveFlanke & TASTE_L)//nicht Kaufen
        {
            blaulicht(50,10); //Blaulicht um aufmeksamkeit zu erwecken
            //zustand = VERSTEIGERUNG; //bei nicht kauf wird versteigert
            //flags setzen
            spielerEingabe = 2; //Rückgabewert 1 = Feld wurde nicht gekauft -> versteigerung
            globalUpdateLCD = 0;
        }
    return spielerEingabe;
}



/******************************************************************************\
* warteBisGewuerfelt
*
* Diese Funktion wartet darauf, dass beide Würfel geworfen wurden.
*
\******************************************************************************/
void warteBisGewuerfelt(void)
{
    uint16_t flagA   = 0;
    uint16_t flagB   = 0;
    uint8_t flagAB  = 0;
    
    //wartet bis mit beiden Würfel gewürfelt wurde
    /*while (!(flagWuerfel1 && flagWuerfel2))
    {
        //flankenerkennung
        tasteAlt = tasteNeu;
        tasteNeu = 0;
        tasteNeu = (PINL << 8) | PINK;
        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        negativeFlanke = (tasteAlt ^ tasteNeu) & tasteAlt;
        //würfelt würfel 1 nur wenn taste9 betätigt wurde
        //und würfel 1 noch nicht gewürfelt wurde
        if ((positiveFlanke & TASTE_A) && !flagWuerfel1)
        {
            //mit 1. würfel würfeln
            wuerfelAB(1,flagWuerfel1,flagWuerfel2);
            flagWuerfel1 = 1;
        }
        //würfelt würfel 2 nur wenn taste9 betätigt wurde
        //und würfel 2 noch nicht gewürfelt wurde
        else if ((positiveFlanke & TASTE_B) && !flagWuerfel2)
        {
            //mit 2. würfel würfeln
            wuerfelAB(2,flagWuerfel1,flagWuerfel2);
            flagWuerfel2 = 1;
            
        }
    }*/
    //wartet bis mit beiden Würfel gewürfelt wurde
    while (!(flagWuerfel1 && flagWuerfel2))
    {
        //flankenerkennung
        tasteAlt = tasteNeu;
        tasteNeu = 0;
        tasteNeu = (PINL << 8) | PINK;
        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        negativeFlanke = (tasteAlt ^ tasteNeu) & tasteAlt;
        
        //Beim ersten Tastendruck startzeit Speichern
        if (((positiveFlanke & TASTE_A) || (positiveFlanke & TASTE_B)) && (!flagA && !flagB))
        {
            flagA = (positiveFlanke & TASTE_A);
            flagB = (positiveFlanke & TASTE_B);
            startZeit = getSystemzeit();
        }
        
        //nach 100ms einzeln würfeln wenn flag AB nicht gesetzt ist
        if (((startZeit + 400) < getSystemzeit()) && !flagAB)
        {
            //würfelt würfel 1 nur wenn taste9 betätigt wurde
            //und würfel 1 noch nicht gewürfelt wurde
            if (((positiveFlanke & TASTE_A) || flagA) && !flagWuerfel1)
            {
                //mit 1. würfel würfeln
                wuerfel(1,flagWuerfel1,flagWuerfel2);
                flagWuerfel1 = 1;
                
                if (!flagB)
                {
                    letzterWuerfel = WUERFEL_B;
                }
            }
            //würfelt würfel 2 nur wenn taste9 betätigt wurde
            //und würfel 2 noch nicht gewürfelt wurde
            else if (((positiveFlanke & TASTE_B) || flagB) && !flagWuerfel2)
            {
                //mit 2. würfel würfeln
                wuerfel(2,flagWuerfel1,flagWuerfel2);
                flagWuerfel2 = 1;
                if (!flagA)
                {
                    letzterWuerfel = WUERFEL_A;
                }
            }
        }
        else
        {
            if (flagA && (positiveFlanke & TASTE_B))
            {
                flagAB = 1;
            }
            else if (flagB && (positiveFlanke & TASTE_A))
            {
                flagAB = 1;
            }
        }
        
        //wenn mit beiden Würfeln gleichzeitig gewürfelt wird
        if (flagAB)
        {
            wuerfelAB();
            flagWuerfel1 = 1;
            flagWuerfel2 = 1;
            letzterWuerfel = WUERFEL_B;
        }
         
    }
}


/******************************************************************************\
* handelWareAuswaehlen
*
* Diese Funktion ermöglicht es einem Spieler, Waren für den Handel auszuwählen,
* darunter Grundstücke, Bargeld und Freikarten.
* Die Auswahl erfolgt durch Tastensteuerung und wird auf dem LCD-Display angezeigt.
*
* Parameter:
* haendlerNr = Nummer des Händlers, der den Handel durchführt
*
* Rückgabewert: 1, wenn die Auswahl abgeschlossen ist, 0 bei laufender Auswahl
*
\******************************************************************************/
uint8_t handelWareAuswaehlen(uint8_t haendlerNr)
{
    char lcdBuffer[16] = {0};
    //flankenerkennung
    //Tasten einlesen und positive Flanken bestimmen
    tasteAlt = tasteNeu;
    tasteNeu = 0;
    tasteNeu = (PINL << 8) | PINK;
    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
    //handelWare verarbeiten
    switch (handelware)
    {
        case GRUNDSTUECK:
        //Nachfolgender Code wird einmal durchgeführt
        if (!globalUpdateLCD)
        {
            /*sprintf(lcdBuffer,"%u",handel[haendlerNr].spielerNr);
            writeText(0,11,lcdBuffer);*/
            //lcd ausgabe
            //"Grundstück" und Tastenbelegung am LCD anzeigen
            writeText(0,0,"   Grundst"UE"ck   ");
            writeText(1,0,"S Handel|weiter"PFEIL_R);
            writeText(2,0,"                ");
            //globalUpdateLCD auf 1 setzen um erneute Ausgabe zu verhindern
            globalUpdateLCD = 1;
            
            //handelbareFelder zurücksetzen
            //Alle Elemente der Liste handelbareFelder durchgehen
            for (uint8_t i = 0; i < 28; i = i + 1)
            {
                //Das aktuelle Element der Liste handelbareFelder auf 0 zurücksetzen
                handelbareFelder[i] = 0;
            }
            //anzahlHandelbareFelder auf 0 zurücksetzen
            anzahlHandelbareFelder = 0;
            //sucht alle felder ab nach Felder die dem Spieler gehören
            //Alle Spielfelder durchlaufen
            for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
            {
                //sucht die spielfelder nach denen ab, die dem Spieler gehören und keine Häuser haben
                //Wenn das aktuelle Feld dem Spieler gehört, und es keine Häuser hat
                if ((spielfeld[i].besitzer == handel[haendlerNr].spielerNr) && (spielfeld[i].anzahlHaeuser == 0))
                {
                    //flag setzen
                    //flagHandelbar auf 1 setzen
                    flagHandelbar = 1;
                    //prüft ob es auf den andern Felder der Farbgruppe noch häuser hat
                    //Alle Felder der Farbgruppe des Feldes durchlaufen
                    for (uint8_t j = 0; j < 3; j = j + 1)
                    {
                        //wenn es auf einem Feld der Farbgruppe noch ein haus hat
                        //Wenn es auf dem zu prüfenden Feld der Farbgruppe ein Haus hat
                        if (spielfeld[spielfeld[i].farbgruppenFelder[j]].anzahlHaeuser)
                        {
                            //flag auf 0 setzen
                            //flagHandelbar auf 0 setzen um das ursprüngliche Feld als nicht handelbar zu markieren
                            flagHandelbar = 0;
                        }
                        
                    }
                    //wenn das Flag immernoch auf 1 ist, kann das FEld gehandelt werden
                    //Wenn flagHandelbar noch gesetzt ist
                    if (flagHandelbar)
                    {
                        //Das aktuelle Feld in der Liste handelbareFelder speichern
                        handelbareFelder[anzahlHandelbareFelder] = i;
                        //anzahlHandelbareFelder aum 1 erhöhen
                        anzahlHandelbareFelder += 1;
                    }
                }
            }
            //zeigt erstes handelbares Feld auf LCD an
            //handelFeld auf 0 setzen
            handelFeld = 0;
            //lcd ausgabe
            writeText(2,0,"                ");
            //Den Namen des ersten handelbaren Feldes am LCD anzeigen
            writeText(2,0,spielfeld[handelbareFelder[handelFeld]].name);
            
        }
        //Nachfolgender Code wird mehrmals durchgeführt
        //Wenn die Taste R betätigt wurde
        if (positiveFlanke & TASTE_R)//nächstes Feld
        {
            //zum nächsten Feld wechseln
            handelFeld = (handelFeld + 1) % anzahlHandelbareFelder; 
            //Den Namen des neuen Feldes am LCD anzeigen
            writeText(2,0,"                ");
            writeText(2,0,spielfeld[handelbareFelder[handelFeld]].name);
        }
        //Wenn Taste S betätigt wurde
        else if (positiveFlanke & TASTE_S)//Feld bestätigt
        {
            //speichert das aktuelle Feld 
            //Das aktuelle Feld im Inventar speichern
            handel[haendlerNr].feldNummern[anzahlAusgewaehlteFelder] = handelbareFelder[handelFeld];
            //zähler erhöhen
            //anzahlAusgewaehlteFelder um 1 erhöhen
            anzahlAusgewaehlteFelder += 1;
            //zustandswechsel
            //handelware auf AUSWAHL_BEENDEN setzen
            handelware = AUSWAHL_BEENDEN;
            //globalUpdateLCD auf 0 setzen
            globalUpdateLCD = 0;
        }
        //Wenn Taste L betätigt wurde
        else if (positiveFlanke & TASTE_L)//etwas anderes handeln
        {
            //lcd ausgabe
            //"Bargeld" und Tastenbelegung am LCD anzeigen
            writeText(0,0,"    Bargeld     ");
            writeText(1,0,"S Handel|weiter"PFEIL_R);
            writeText(2,0,"                ");
            //zustandswechsel
            handelware = BARGELD;
            globalUpdateLCD = 0;
        }
    	break;
        case BARGELD:
        //Wenn das LCD noch nicht aktualisiert wurde
        if (!globalUpdateLCD)
        {
            //Betrag und Tastenbelegung am LCD anzeigen
            writeText(0,0,"Bargeld 0       ");
            writeText(1,0,"S Handel|weiter"PFEIL_R);
            writeText(2,0,PFEIL_O" +10      +100"PFEIL_U);
            globalUpdateLCD = 1;
        }
        //"Bargeld" und Tastenbelegung am LCD anzeigen
        if (positiveFlanke & TASTE_O)
        {
            //Betrag um 10 erhöhen
            handel[haendlerNr].barGeld += 10;
            //Betrag am LCD ausgeben
            sprintf(lcdBuffer,"%4u",handel[haendlerNr].barGeld);
            writeText(0,8,lcdBuffer);
        }
        //Wenn Taste U betätigt wurde
        else if (positiveFlanke & TASTE_U)
        {
            //Betrag um 100 erhöhen
            handel[haendlerNr].barGeld += 100;
            //Betrag am LCD ausgeben
            sprintf(lcdBuffer,"%4u",handel[haendlerNr].barGeld);
            writeText(0,8,lcdBuffer);
        }
        //prüft ob der Spieler eine Freikarte hat
        //Wenn Taste L betätigt wurde und der Spieler eine Freikarte besitzt
        if ((positiveFlanke & TASTE_L) && spielerInfo[handel[haendlerNr].spielerNr].freikarte)
        {
            //handelware auf FREIKARTEN setzen
            handelware = FREIKARTEN;//zustandswechsel
            //"Freikarte" am LCD ausgeben
            writeText(0,0,"   Freikarte    ");
            writeText(2,0,"                ");
            //globalUpdateLCD auf 0 setzen
            globalUpdateLCD = 0;
        }
        //Wenn Taste L betätigt wurde
        else if(positiveFlanke & TASTE_L)
        {
            //handelware auf GRUNDSTUECK setzen
            handelware = GRUNDSTUECK;//zustandswechsel
            //globalUpdateLCD auf 0 setzen
            globalUpdateLCD = 0;
        }
        if(positiveFlanke & TASTE_S)
        {
            handelware = AUSWAHL_BEENDEN;//zustandswechsel
            globalUpdateLCD = 0;
        }
        break;
        case FREIKARTEN:
        if (positiveFlanke & TASTE_S)
        {
            handel[haendlerNr].freikarte = 1;
            handelware = AUSWAHL_BEENDEN;
            globalUpdateLCD = 0;
        }
        else if(positiveFlanke & TASTE_L)
        {
            //handelware auf GRUNDSTUECK setzen
            handelware = GRUNDSTUECK;//zustandswechsel
            //globalUpdateLCD auf 0 setzen
            globalUpdateLCD = 0;
        }
        break;
        case AUSWAHL_BEENDEN:
        //Wenn das LCD noch nicht aktualisiert wurde
        if (!globalUpdateLCD)
        {
            //"Auswahl Beenden", optionen und Tastenbelegung am LCD anzeigen
            writeText(0,0,"Auswahl Beenden?");
            writeText(1,0,PFEIL_O"Beenden   mehr"PFEIL_U);
            writeText(2,0,"                ");
            //globalUpdateLCD auf 1 setzen um erneute Ausgabe zu verhindern
            globalUpdateLCD = 1;
        }
        //Wenn Taste O betätigt wurde
        if (positiveFlanke & TASTE_O)//Auswahl Beenden~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        {
            //anzahlAusgewaehlteFelder <uf 0 zurücksetzen
            anzahlAusgewaehlteFelder = 0;
            //handelware auf GRUNDSTUECK zurücksetzen
            handelware = GRUNDSTUECK;
            //globalUpdateLCD auf 0 zurücksetzen
            globalUpdateLCD = 0;
            return 1;
        }
        //Wenn Taste U betätigt wurde
        else if (positiveFlanke & TASTE_U)//mehr auswählen
        {
            //globalUpdateLCD auf 0 setzen
            globalUpdateLCD = 0;
            //handelware auf GRUNDSTUECK setzen
            handelware = GRUNDSTUECK;
        }
        break;
        default:
        break;
    }
    //Den Wert 0 zurückgeben
    return 0;
}

/******************************************************************************\
* auswahlBestaetigen
*
* Diese Funktion bestätigt die ausgewählten Handelswaren, darunter Grundstücke,
* Bargeld und Freikarten. Sie wartet auf die Bestätigung durch den anderen Spieler.
*
* Parameter:
* haendlerNr = Nummer des Händlers
*
* Rückgabewert: 1 wenn alles bestätigt wurde , 0 wenn etwas abgelehnt wurde
*
\******************************************************************************/
uint8_t auswahlBestaetigen(uint8_t haendlerNr)
{
    char lcdBuffer[16] = {0};
        //flagBestaetigt auf 1 initialisieren
    uint8_t flagBestaetigt = 1;
    //Wenn das LCD noch nicht aktualisiert wurde
    if (!globalUpdateLCD)
    {
        //schreibt die Spielernummer auf das LCD
        //Spielernummer am LCD anzeigen
        writeText(0,0,"   Spieler      ");
        sprintf(lcdBuffer,"%u",handel[1 - haendlerNr].spielerNr);
        writeText(0,11,lcdBuffer);
        //"Bestätigen" am LCD anzeigen
        writeText(1,0,"   Best"AE"tigen   ");
        writeText(2,0,"                ");
        //globalUpdateLCD auf 1 setzen um erneute Ausgabe zu verhindern
        globalUpdateLCD = 1;
    }
    //prüft ob grundstücke gehandelt wurden
    //Wenn grundstücke gehandelt werden
    if (handel[haendlerNr].feldNummern[0])
    {
        //Alle ausgewählten Grundstücke durchlaufen
        for (uint8_t i = 0; handel[haendlerNr].feldNummern[i] > 0; i = i + 1)
        {
            //"Bestätigen" am LCD anzeigen
            writeText(1,0,"   Best"AE"tigen   ");
            //Name des Feldes am LCD anzeigen
            writeText(2,0,spielfeld[handel[haendlerNr].feldNummern[i]].name);
            //wartet bis der andere spieler bestätigt hatt
            //Solange der Spieler weder bestätigt noch abgelehnt hat
            while (!((positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1]) || (positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])))
            {
                //Flankenerkennung
                //Tasten einlesen und Positive Flanken bestimmen
                tasteAlt = tasteNeu;
                tasteNeu = 0;
                tasteNeu = (PINL << 8) | PINK;
                positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                //wartet auf benutzereingabe
            }
            //Wenn die Taste X betätigt wurde
            if (positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1])
            {
                //"BESTÄTIGT" am LCD ausgeben
                writeText(1,0,"   Best"AE"tigt    ");
                writeText(2,0,"                ");
                //Programm für 1 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
                _delay_ms(1000);
            }
            //Wenn die Taste Y betätigt wurde 
            if (positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])
            {
                //flagBestaetigt auf 0 setzen
                flagBestaetigt = 0;
            }
            //Flankenerkennung
            //Tasten einlesen und Positive Flanken bestimmen
            tasteAlt = tasteNeu;
            tasteNeu = 0;
            tasteNeu = (PINL << 8) | PINK;
            positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        }
    }
    //prüft ob bargeld gehandelt wird
    //Wenn Bargeld gehandelt wird
    if (handel[haendlerNr].barGeld)
    {
        //Spielernummer am LCD anzeigen
        writeText(0,0,"   Spieler      ");
        sprintf(lcdBuffer,"%u",handel[1 - haendlerNr].spielerNr);
        writeText(0,11,lcdBuffer);
        //"Bestätigen" am LCD anzeigen
        writeText(1,0,"   Best"AE"tigen   ");
        //Geld Betrag am LCD ausgeben
        writeText(2,0,"Bargeld:        ");
        sprintf(lcdBuffer,"%4u",handel[haendlerNr].barGeld);
        writeText(2,9,lcdBuffer);
        //Solange der Spieler weder bestätigt noch abgelehnt hat
        while (!((positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1]) || (positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])))
        {
            //Flankenerkennung
            //Tasten einlesen und Positive Flanken bestimmen
            tasteAlt = tasteNeu;
            tasteNeu = 0;
            tasteNeu = (PINL << 8) | PINK;
            positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
            //wartet auf benutzereingabe
        }
        //Wenn die Taste X betätigt wurde
        if (positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1])
        {
            //"BESTÄTIGT" am LCD ausgeben
            writeText(1,0,"   Best"AE"tigt    ");
            writeText(2,0,"                ");
            //Programm für 1 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
            _delay_ms(1000);
        }
        //Wenn die Taste Y betätigt wurde 
        if (positiveFlanke & positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])
        {
            //flagBestaetigt auf 0 setzen
            flagBestaetigt = 0;
        }
    }
    //prüft ob Freikarten gehandelt wird
    //Wenn Freikarten gehandelt werden
    if (handel[haendlerNr].freikarte)
    {
        //Spielernummer am LCD anzeigen
        writeText(0,0,"   Spieler      ");
        sprintf(lcdBuffer,"%u",handel[1 - haendlerNr].spielerNr);
        writeText(0,11,lcdBuffer);
        //"Bestätigen" am LCD anzeigen
        writeText(1,0,"   Best"AE"tigen   ");
        //"Freikarte" am LCD ausgeben
        writeText(0,2,"   Freikarte    ");
        //Solange der Spieler weder bestätigt noch abgelehnt hat
        while (!((positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1]) || (positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])))
        {
            //Flankenerkennung
            //Tasten einlesen und Positive Flanken bestimmen
            tasteAlt = tasteNeu;
            tasteNeu = 0;
            tasteNeu = (PINL << 8) | PINK;
            positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
            //wartet auf benutzereingabe 
        }
        if (positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1])
        {
            //"BESTÄTIGT" am LCD ausgeben
            writeText(1,0,"   Best"AE"tigt    ");
            writeText(2,0,"                ");
            //Programm für 1 Sekunden blokieren, damit die Spieler Zeit haben das LCD zu lesen
            _delay_ms(1000);
        }
        //Wenn die Taste Y betätigt wurde 
        if (positiveFlanke & positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])
        {
            //flagBestaetigt auf 0 setzen
            flagBestaetigt = 0;
        }
    }
    //globalUpdateLCD auf 0 setzen
    globalUpdateLCD = 0;
    //flagBestaetigt zurückgeben
    return flagBestaetigt;
}


// ISR: Wird alle 1 ms aufgerufen
ISR(TIMER1_COMPA_vect)
{
    millis++;
}

uint32_t getSystemzeit(void) 
{
    uint32_t ms;
    cli();           // Interrupts deaktivieren
    ms = millis;
    sei();           // Interrupts wieder aktivieren
    return ms;
}