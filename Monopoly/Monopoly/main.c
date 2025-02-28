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
* Projekt  : IPA_Monopoly
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
* 10.01.2025  A.Schneider    V1.0      Neuerstellung
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

#define MAX_ANZAHL_HAEUSER_IM_SPIEL 32
#define MAX_ANZAHL_HOTELS_IM_SPIEL 12

#define ZAHLUNG_ERFOLGREICH 1
#define ZAHLUNG_FEHLGESCHLAGEN 2

#define ANZAHL_FELDER 40
#define STARTGELD 1500
#define RUNDEN_GELD 200

#define WUERFEL_A 1
#define WUERFEL_B 2
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

uint16_t tasteAlt, tasteNeu, positiveFlanke = 0; //Variabeln Flankenerkennung

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


int main(void)
{
    char buffer[200];  // Buffer im RAM
    
    
    /*--- Modullokale Konstanten ------------------------------------------------*/
    /*--- Modullokale Variablen -------------------------------------------------*/
    //char
    char lcdBuffer[16];
    //8-Bit Variabeln
    uint8_t spielerAmZug = 1;
    uint8_t flagNextPlayer, flagPasch = 0;
    uint8_t flagWeiter = 1;
    uint8_t aktuellePosition = 0;
    
    uint8_t flagTasteX = 0;
    uint8_t flagTasteY = 0;
    uint8_t bieter[6] = {0};//0-3 Bieter 4 anz. spieler raus 5 höchstbieter
    uint8_t ersterSpieler = 0;
    
    
    
    uint8_t spielerSetup = 0;
    
    uint8_t flagFertigGewuerfelt, letzterWuerfel, flagSchulden = 0;
    
    uint8_t flagVersteigert, verkaufSpielerEingabe = 0;
    
    uint8_t updateLCD = 0;
    uint8_t flagSpielLCD = 0;
    
    uint8_t feldBesitzer = 0;
    uint8_t bezahlStatus = 0;
    uint8_t flagZahlungAbgeschlossen = 1;//flagZahlungAbgeschlossen auf 1 setzen
    uint8_t flagKaufAbgechlossen = 1;
    
    
    
    
    uint8_t farbgruppenCounter = 0;
    
    
    uint8_t flagHausFeld1 = 0;
    uint8_t flagHausFeld2 = 0;
    uint8_t flagHausFeld3 = 0;
    
    

    
    uint8_t haltestelleFarbgruppe = 0;
    uint8_t multiplikator = 0;
    
    uint8_t ereignisSchritt = 0;
    uint8_t flagEreignisWeiter = 0;
    uint8_t flagEreignisAbgeschlossen = 1;
    uint8_t ereignisfeldRueckgabe = 0;
    uint8_t flagKanzlei = 0;
    
    uint8_t kaufStatus = 0;
    uint16_t zahlBetrag = 0;
    uint8_t flagMieteFarbgruppe = 0;
    uint8_t zahlBetragFarbgruppe = 0;
    uint8_t zahlSchritt = 0;
    
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
    uint8_t flagSpielerPleite = 0;
    
    uint8_t spielerInventar[28] = {0};
    uint8_t flagKeineHaeuser = 0;
    
    uint8_t haendlerZaehler = 0;
    uint8_t haendlerAmZug = 0;
    uint8_t auswahlAbgeschlossen = 0;
    uint8_t handelSpielerNummern[2] = {0};
    uint8_t handelFeldNummer = 0;
    uint8_t flagHandelBelastet = 0;
    uint8_t handelHypothek = 0;
    uint8_t handelBezahlBetrag = 0;
    uint8_t handelzahlungErfolgreich = 0;
    /*--- Prototypen modullokaler Funktionen ------------------------------------*/
    uint8_t feldKaufen(uint8_t feldNummer, Feld spielfeld[40], uint8_t spielerAmZug);
    uint8_t bauen(uint8_t feldNummer, uint8_t spielerAmZug);
    uint8_t abBauen(uint8_t feldNummer, uint8_t spielerAmZug);
    void warteBisGewuerfelt(void);
    uint8_t handelWareAuswaehlen(uint8_t haendlerNr);
    uint8_t auswahlBestaetigen(uint8_t haendlerNr);
    
    uint32_t seed = 0;
    uint32_t adcWert = 0;
    /*--- Funktionsdefinitionen -------------------------------------------------*/
    
    
    //Initialisierung
    PortInitialisierung();//ports initialisieren während vorbereitung erstellt!!
    lcdInitAll();//lcd Initialisieren während vorbereitung erstellt!!
    initialisiereSpielfeld(spielfeld);//spielfelder Initialisieren
    initSpieler(spielerInfo);
    initialisiereKarten(chanceKanzlei);
    initialisiereHandelInventar(handel);//initialisiert das handelinventar
    SPI_init_all(9600);//SPI initialisieren während vorbereitung erstellt!!
    resetMonopoly();
    //random Seed setzen
    //ADC initialisieren
    adm_ADC_init();//ADC Initialisieren während vorbereitung erstellt!!
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
    
    
    _delay_ms(1000);
    //for schleife setzt die RGB auf jedem spielfeld
    for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
    {
        if (spielfeld[i].besitzer)
        {
            setPropertyRgb(spielfeld[i].rgbNummer,spielfeld[i].besitzer);
        }
    }
    
    
    uint8_t textCounter = 0;
    uint8_t stringCounter = 0;
    uint8_t flagEreignisfeld = 0;
    //_delay_ms(1000);
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
                updateKontostand(anzahlSpieler,spielerInfo);//Spielernummer auf siebensegmenten anzeigen
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
                updateKontostand(anzahlSpieler,spielerInfo);
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
                updateKontostand(anzahlSpieler,spielerInfo);
            }
            //Wenn anzahl Spieler bestätigt wurde Taste S
            if (positiveFlanke & TASTE13)
            {
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    spielerInfo[i].geld = 0; //setze das geld von Spieler i auf 0
                    spielerInfo[i].position = 0; //Setze die Position von spieler i auf 0
                    setPlayerPosition(spielerInfo[i].position,i);//gib die position von spieler i an den leds aus
                }
                //aktualisiere den Kontostand für alle spieler
                updateKontostand(anzahlSpieler,spielerInfo);
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
            case WUERFELSTART://erster spieler anhand von würfeln bbestimmen
            //Geldanzeige aller Spieler auschalten
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //schalte die Konto Siebensegmentanzeigen des spielers i aus
                setGeld(STARTGELD,i,SIEBENSEGMENT_OFF);
            }
            //lässt jeden spieler würfeln
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //spieler am zug und Navigation am lcd Ausgeben
                //LCD Ausgabe
                writeText(0,0,"   Spieler X    ");
                sprintf(lcdBuffer,"%u",i);//lädt den Spieler am zug in denlcdBuffer
                writeText(0,11,lcdBuffer);//gibt lcdBuffer an LCD aus
                writeText(1,0,"    w"UE"rfelt    ");
                writeText(2,0," wuerfeln A / B ");
                
                //solange noch nicht mit beiden würfel gewürfelt wurde  
                while (!(flagWuerfel1 && flagWuerfel2)) 
                {
                    //flankenerkennung
                    tasteAlt = tasteNeu;
                    tasteNeu = 0;
                    tasteNeu = (PINL << 8) | PINK;
                    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                    //würfelt würfel 1 nur wenn taste9 betätigt wurde
                    //und würfel 1 noch nicht gewürfelt wurde
                    if ((positiveFlanke & TASTE9) && !flagWuerfel1)
                    {
                        wuerfelAB(WUERFEL_A,flagWuerfel1,flagWuerfel2);//Würfel A wüefeln
                        flagWuerfel1 = 1;//Flag setzen um erneutes würfeln mit Würfel A zu blockieren
                    }
                    //würfelt würfel 2 nur wenn taste9 betätigt wurde
                    //und würfel 2 noch nicht gewürfelt wurde
                    else if ((positiveFlanke & TASTE10) && !flagWuerfel2)
                    {
                        wuerfelAB(WUERFEL_B,flagWuerfel1,flagWuerfel2);//Würfel B wüefeln
                        flagWuerfel2 = 1;//Flag setzen um erneutes würfeln mit Würfel B zu blockieren
                    }
                    //prüft welche würfel als 2. verwendet wurde
                    //dies ist wichtig um sicherzustellen, dass nur ein spieler
                    //die höchste zahl hat.
                    if (flagWuerfel1 && !letzterWuerfel)//Wenn das Flag von Würfel A gesetzt ist und das von Würfel B nicht gesetzt ist
                    {
                        //Speichert Würfel B als den Würfel der als 2. gewürfelt wurde
                        letzterWuerfel = WUERFEL_B; //würfel 2 wird als letztes verwendet
                    }
                    if (flagWuerfel2 && !letzterWuerfel)//Wenn das Flag von Würfel A nicht gesetzt ist und das von Würfel B gesetzt ist
                    {
                        //Speichert Würfel A als den Würfel der als 2. gewürfelt wurde
                        letzterWuerfel = WUERFEL_A; //würfel 1 wird als letztes verwendet
                    }
                }
                
                //Setzt das geld des Spielers auf die Summe der Würfel
                //Speichert die Summe der beiden gewürfelten Zahlen
                spielerInfo[i].geld = wuerfelArray[0] + wuerfelArray[1];
                
                //falls der aktuelle Spieler die gleiche Summe wie Platz 1 hat
                
                //Wenn der Aktuelle Spieler die Gleiche Würfelsumme hat,
                //wie der Spieler der momentan die höchste Summe hat
                if (spielerInfo[i].geld == spielerInfo[ersterSpieler].geld) 
                {
                    //Wenn die würfelzahl des 2. Würfels > 1 ist,
                    //dann wird die Würfelzahl um 1 verkleinert
                    if (wuerfelArray[letzterWuerfel-1] > 1) 
                    {
                        //Die 2. gewürfelte zahl wird um 1 verkleinert
                        wuerfelArray[letzterWuerfel-1] = wuerfelArray[letzterWuerfel-1] - 1;
                    }
                    else
                    {
                        //Die 2. gewürfelte zahl wird um 1 vergrössert
                        wuerfelArray[letzterWuerfel-1] = wuerfelArray[letzterWuerfel-1] + 1;
                    }
                    //Speichert die angepasste Zahl
                    spielerInfo[i].geld = wuerfelArray[0] + wuerfelArray[1];
                    //neue zahlen werden an die Würfel siebensegmente ausgegeben
                    //Ausgabe der angepassten Würfelzahl
                    wuerfelTransmit(wuerfelArray[0],wuerfelArray[1]); 
                }
                
                //wenn der aktuelle Spieler eine grössere Würfelsumme hat,
                //als der Spieler der Spieler der momentan die Höchste Summe hat
                if (spielerInfo[i].geld > spielerInfo[ersterSpieler].geld)
                {
                    //Speichert den momentanen Spieler als den Spieler mit der höchsten Summe
                    ersterSpieler = i;
                }
                
                //Flags und Variabeln zurücksetzen
                flagWuerfel1 = 0;
                flagWuerfel2 = 0;
                letzterWuerfel = 0;
                //Würfelsumme auf Konto Siebensegmente Ausgeben
                updateKontostand(i,spielerInfo);
            }
            //Setzt den Spieler mit der höchsten Summe als den Spieler der am Zug ist
            spielerAmZug = ersterSpieler;
            
            //wartet 1s um es den Spielern zu ermöglichen festzustellen, wer die höchste zahl hat
            _delay_ms(1000);//eine Sekunde Warten
            //Startgeld verteilen
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //Gibt den Spielern das Startgeld
                spielerInfo[i].geld = STARTGELD;
            }
            //Ausgabe an Siebensegment
            updateKontostand(anzahlSpieler,spielerInfo);
            //LCD Ausgabe
            // Spieler X
            //würfeln A / B
            //weiter C
            //Spieler am zug und Navigation am lcd Ausgeben
            writeText(0,0,"   Spieler      ");
            //lädt die Variabel "spielerAmZug" in den lcdBuffer
            sprintf(lcdBuffer,"%u",spielerAmZug);
            writeText(0,11,lcdBuffer);//Ausgabe auf LCD
            writeText(1,0," w"UE"rfeln A / B ");
            writeText(2,0,"weiter C  mehr S");
            
            //Wechselt zum Zustand in dem das hauptspiel statt findet
            //Zum Zustand SPIEL wechseln
            zustand = SPIEL;
            break;
            //in diesem Zustand findet das hauptspiel statt
            case SPIEL://Zustand steuert das Spiel
            //nach einer Versteigerung info an lcd anzeigen
            /*if (flagVersteigert || flagZahlungAbgeschlossen)
            {
                clear();//lcd leeren
                //spieler am zug anzeigen
                writeText(0,0,"   Spieler      "); 
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"    weiter C    ");
                flagVersteigert = 0;
                flagZahlungAbgeschlossen = 0;
            }*/
            //flagSpielLCD kann auf 1 gesetzt werden
            //um die Standart LCD maske anzuzeigen
            if (flagSpielLCD)
            {
                //lcd Inhalt löschen
                clear();//lcd leeren
                //Spieler am zug und Navigation an LCD ausgeben
                writeText(0,0,"   Spieler      ");
                //lädt die Variabel "spielerAmZug" in den lcdBuffer
                sprintf(lcdBuffer,"%u",spielerAmZug);
                //ausgabe von lcdBuffer auf display
                writeText(0,11,lcdBuffer);
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"weiter C  mehr S");
                //Setzt das flag wieder zurück um sicherzustellen
                //dass die Ausgabe nicht erneut erfolgt
                //flagSpielLcd auf 0 setzen um erneutes Ausführen zu verhindern
                flagSpielLCD = 0;
            }
            //Spielzug abschliessen alles zurücksetzen
            //wenn Taste C betätigt wurde oder flagGefaengnisWeiter = 0 und
            //flagFertigGewuerfelt, flagZahlungAbgeschlossen, flagKaufAbgeschlossen,
            //flagEreignisAbgeschlossen gesetzt sind
            //flagSpieler Pleite --> wenn spieler pleite ist, ist automatisch der nächste Spieler an der Reihe
            if(((positiveFlanke & TASTE_C) || flagGefaengnisWeiter || flagSpielerPleite) && flagFertigGewuerfelt && flagZahlungAbgeschlossen && flagKaufAbgechlossen && flagEreignisAbgeschlossen)
            {
                //beide Würfel Siebensegment Anzeigen ausschalten
                wuerfelTransmit(SIEBENSEGMENT_OFF,SIEBENSEGMENT_OFF);
                //nächster spieler
                spielerAmZug = (spielerAmZug % anzahlSpieler) + 1;
                //spieler am zug anzeigen
                writeText(0,0,"   Spieler      ");
                //lädt die Variabel "spielerAmZug" in den lcdBuffer
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                //ausgabe von lcdBuffer auf display
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"weiter C  mehr S");
                //~~~~~~~~~~~~flags zurücksetzten~~~~~~~~~~~~~~~~~~
                flagFertigGewuerfelt = 0;//flag zum würfeln blockieren
                PORTC &= ~0xC0;//Schaltet das Blaulicht aus
                //Variablen und flags auf 0 setzen
                updateLCD = 0;//flag wird verwendet um lcdausgabe zu blockieren
                bezahlStatus = 0;//rückgabewert geldUeberweisen
                ereignisfeldRueckgabe = 0;//~?????~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                //speichert feldTyp in aktuellesFeld
                //wird unten im switch case verwendet
                //aktueller Feld Typ anhand der Position des Spielers am Zug bestimmen
                aktuellesFeld = spielfeld[spielerInfo[spielerAmZug].position].typ;
                flagGefaengnis = 0; //flag wird gesetzt wenn spieler im gefängnis ist
                flagGefaengnisLCD = 0;//flag wird im gefängnis verwendet bez. LCD
                flagGefaengnisWeiter = 0;//flag um spiel weiterlaufen zulassen nach entlassung aus gefängnis
                flagSpielerPleite = 0;//flag wird gesetzt um einen Spieler der pleite ist zu überspringen
                //überprüft ob der Spieler im gefängnis ist
                if ((aktuellesFeld == GEFAENGNIS) && spielerInfo[spielerAmZug].gefaengnis)//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Ãktuelles feld kann weggelassen werden
                {
                    //flagGefaengnis auf 1 setzen
                    flagGefaengnis = 1;//flag setzen wenn spieler im gefängnis ist
                    //flag blockiert würfeln
                    //flagFertigGewuerfelt auf 1 setzen um würfeln zu blockieren
                    flagFertigGewuerfelt = 1; //wenn dieses Flag gesetzt ist kann nicht gewürfelt werden
                }
                //prüft ob der spielerAmZug pleite ist
                if (spielerInfo[spielerAmZug].pleite)
                {
                    flagSpielerPleite = 1;//Der spieler, der am zug ist ist bereits aus dem Spiel ausgeschieden
                    flagFertigGewuerfelt = 1;
                    aktuellesFeld = FREIPARKEN;
                }
            }
            //ermöglicht es dem spieler bei Pasch zu kaufen
            if ((positiveFlanke & TASTE_C) && !flagWeiter && flagZahlungAbgeschlossen && flagEreignisAbgeschlossen)
            {
                //Würfel Siebensegmente ausschalten
                wuerfelTransmit(SIEBENSEGMENT_OFF,SIEBENSEGMENT_OFF);
                //flag setzen um erneut würfeln zu können
                flagWeiter = 1; //Flag setzen
                bezahlStatus = 0;//~~~~~~~~~~~~~~~~~kann ev. weggelassen werden
                ereignisfeldRueckgabe = 0;
            }//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Würfel~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //lässt den Spieler einmal würfel
            //Wenn flagWeiter nicht gesetzt ist, kann man nicht würfeln das
            //flag braucht es, da man ansonsten bei einem pasch nichts kaufen kann
            //wenn flagSpielerPleite = 1 ist spielt der spieler nicht mehr mit
            if (!flagFertigGewuerfelt && flagWeiter && !flagSpielerPleite) 
            {
                //wartet bis mit beiden Würfel gewürfelt wurde
                warteBisGewuerfelt();//lässt den spieler würfeln
                //lässt den spieler bei einem Pasch nochmal würfeln,
                //ausser der Pasch wurde im Workshop gemacht
                if ((wuerfelArray[0] == wuerfelArray[1]) && !flagGefaengnis) //Pasch
                {   
                    flagWeiter = 0; //bei pasch flag = 0 => frühzeitiges würfeln blockieren
                    flagPasch = flagPasch + 1; //paschzähler erhöhen ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~variabel umbenennen
                    //beide würfel flags zurücksetzen
                    //flags dienen um festzustellen
                    //mit welchem würfel bereits gewürfelt wurde
                    flagWuerfel1 = 0;
                    flagWuerfel2 = 0;
                    //verarbeitung aufgrund von anzahl pasch infolge
                    switch (flagPasch) //schaltet LEDs ein bei Pasch
                    {
                        case 1://1. Pasch
                        PORTC |= 0x40; //Erste Blaulicht LED einschalten
                    	break;
                        case 2:
                        PORTC |= 0x80; //Zweite Blaulicht LED einschalten
                        break;
                        case 3: //beim 3. Pasch = Gefängnis
                        PORTC &= ~0xC0; //Blaulicht ausschalten
                        
                        wuerfelArray[0] = 0;//würfelzahlen zurücksetzen
                        wuerfelArray[1] = 0;//würfelzahlen zurücksetzen
                        //flags zurücksetzen
                        flagWuerfel1 = 0;
                        flagWuerfel2 = 0;
                        flagPasch = 0;
                        flagWeiter = 1;
                        flagFertigGewuerfelt = 1;//erneutes würfeln blockieren
                        //spieler am zug ins gefängnis schicken
                        abInsGefaengnis(spielerAmZug); 
                        aktuellesFeld = GEFAENGNIS;//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~glaub unnötig
                        break;
                    }
                }
                else//wenn kein pasch gewürfelt wurde
                {
                    //flag setzen erlaubt es den spielzug abzuschliessen
                    flagFertigGewuerfelt = 1; //erneutes Würfeln blockieren
                    flagPasch = 0; //flagPasch zurücksetzen
                }
                 //wenn die aktuelle spition des Spielers + wüfelsumme grösser gleich 40 ist
                 // erhält der spieler 200 CHF
                 //Prüft ob der Spieler über los kommt
                 
                 //animiert die fortbewegung des spielers
                 for (uint8_t i = spielerInfo[spielerAmZug].position + 1; i <= (spielerInfo[spielerAmZug].position + (wuerfelArray[0] + wuerfelArray[1])); i = i + 1)
                 {
                     setPlayerPosition(i % ANZAHL_FELDER,spielerAmZug);
                     if ((i % ANZAHL_FELDER) == 0)
                     {
                         geldUeberweisen(0,spielerAmZug,RUNDEN_GELD);
                     }
                     //wenn der Spieler auf Feld 
                     _delay_ms(100); //delay dient zu animationszwecken
                 }
                 //addiert die würfelsumme zur aktuellen position dazu
                 spielerInfo[spielerAmZug].position = (spielerInfo[spielerAmZug].position + (wuerfelArray[0] + wuerfelArray[1])) % 40;
                 //setzt den spieler auf das richtige Feld
                 setPlayerPosition(spielerInfo[spielerAmZug].position,spielerAmZug);
                 //speichert den aktuellen Feld typ
                 aktuellesFeld = spielfeld[spielerInfo[spielerAmZug].position].typ;
                 //speichert die aktuelle position
                 aktuellePosition = spielerInfo[spielerAmZug].position;
                 //würfelflags zurücksetzen
                 flagWuerfel1 = 0;
                 flagWuerfel2 = 0;
            }
            
            //kontostand aktualisieren und anzeigen
            updateKontostand(anzahlSpieler,spielerInfo);
            //Spieler hat fertig gewürfelt ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            
            //Wenn Taste S => mehr betätigt wurde
            if (positiveFlanke & TASTE_S)
            {
                //Wenn die Taste S betätigt wurde gelangt man zur verwaltung
                /*writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"weiter C  mehr S");*/
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~VERWALTUNG~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                updateLCD = 0;//LCD ausgabe blockieren
                zustand = VERWALTEN;//Zustandswechsel
            }
            
            switch (aktuellesFeld) //verarbeitung aufgrund aktuellem feldtyp
            {
                //wenn das aktuelle Feld ein Ereignissfeld ist
                case EREIGNISFELD:
                //prüft ob das aktuelle feld Chance ode Kanzlei ist
                //dies ist wichtig, da die chance karten sich 
                //von den Kanzlei karten unterscheiden
                if ((aktuellePosition == CHANCE1) || (aktuellePosition == CHANCE2) || (aktuellePosition == CHANCE3))
                {
                    flagKanzlei = 0;//das feld ist chance
                }
                else
                {
                    flagKanzlei = 1;//das feld ist kanzlei
                }
                //wenn das ereignis noch nicht durchgeführt wurde
                if (!ereignisfeldRueckgabe)
                {
                    flagEreignisAbgeschlossen = 0;//wird ev. nicht behr benötigt
                    if ((positiveFlanke & TASTE_R) || !ereignisSchritt)//Taste zum scrollen
                    {
                        //funktion zur verarbeitung von Ereignis
                        ereignisfeldRueckgabe = ereignisFeld(flagKanzlei,spielerAmZug,ereignisSchritt,flagEreignisWeiter,chanceKanzlei);
                        //wenn ereignis erfolgreich war
                        if (!ereignisfeldRueckgabe)
                        {
                            //erhöht den schritt zähler um 2
                            //das bedeutet, das jeweils die nächsten 2 
                            //zeilen auf dem LCD angezeigt werden
                            ereignisSchritt += 2;
                        }
                    }
                    //wartet bis spieler mit seiner X Taste bestätigt hat
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        flagEreignisWeiter = 1;//flag erlaubt durchfürung von Erreigniss
                        ereignisfeldRueckgabe = ereignisFeld(flagKanzlei,spielerAmZug,ereignisSchritt,flagEreignisWeiter,chanceKanzlei);
                        //speichert den aktuellen Feld typ
                        aktuellesFeld = spielfeld[spielerInfo[spielerAmZug].position].typ;
                        //speichert die aktuelle position
                        aktuellePosition = spielerInfo[spielerAmZug].position;
                        
                        //erreignis wurde durchgeführt
                        //alle flags auf uhrsprungszustand
                        flagEreignisWeiter = 0;
                        ereignisSchritt = 0;
                        flagEreignisAbgeschlossen = 1;
                        flagSpielLCD = 1;
                    }
                }
                break;
                //wenn das aktuelle Feld eine strasse ist
                case STRASSE:
                //Wenn das Feld noch kein besitzer hat, kann man es kaufen
                if (spielfeld[aktuellePosition].besitzer == 0)
                {
                    flagKaufAbgechlossen = 0;
                    //lässt den spieler das feld kaufen
                    verkaufSpielerEingabe = feldKaufen(aktuellePosition,spielfeld,spielerAmZug);
                    //verarbeitung aufgrund von Rückgabewert 
                    switch (verkaufSpielerEingabe)
                    {
                        case 0://es wurde noch keine Eingabe getätigt
                    	break;
                        case 1://Der Spieler hat das Feld gekauft
                        flagKaufAbgechlossen = 1;
                        break;
                        case 2://Das Feld wurde nicht gekauft
                        flagKaufAbgechlossen = 1;
                        //bezahlstatus auf 1 setzen, ansonsten müsste spieler auf feld nach auktion miete bezahlen
                        bezahlStatus = 1;
                        //feld wird versteigert, weil es nicht gekauft wurde
                        zustand = VERSTEIGERUNG;
                        break;
                    }
                }
                //prüft ob das feld belastet ist
                feldbelastet = spielfeld[aktuellePosition].feldBelastet;
                //wenn das Aktuelle feld einem spieler gehört muss man bezahlen
                //ausser es gehört einem selbst oder es ist belastet
                if((spielfeld[aktuellePosition].besitzer && !(spielfeld[aktuellePosition].besitzer == spielerAmZug)) && !(bezahlStatus == 1) && !feldbelastet)
                {
                    flagZahlungAbgeschlossen = 0;
                    if (!updateLCD)
                    {
                        //besitzer des feldes aus array auslesen
                        feldBesitzer = spielfeld[aktuellePosition].besitzer;
                        //miete anhand von anzahl häuser aus array auslesen
                        zahlBetrag = spielfeld[aktuellePosition].mieten[spielfeld[aktuellePosition].anzahlHaeuser];
                        //betrag bei kompletter Farbgrupppe auslesen
                        zahlBetragFarbgruppe = spielfeld[aktuellePosition].mieten[6];
                        //flag setzen
                        flagMieteFarbgruppe = 1;
                        
                        //prüft ob die ganze farbgruppe dem selben SPieler gehhört
                        for (uint8_t i = 0; i < 3; i = i + 1)
                        {
                            if (!(spielfeld[spielfeld[aktuellePosition].farbgruppenFelder[i]].besitzer == feldBesitzer))
                            {
                                //wenn ein feld nicht dem spieler gehört
                                //wird das flag wieder auf 0 gesetzt
                                flagMieteFarbgruppe = 0;
                            }
                        }
                        //bestimmt ob farbgruppenmiete oder miete mit Häuser bezahlt werden muss
                        if (flagMieteFarbgruppe && (zahlBetragFarbgruppe > zahlBetrag))
                        {
                            //wenn farbgruppenpreis bezahlt werden muss
                            //wird der zahlBetrag dementsprechend gesetzt
                            zahlBetrag = zahlBetragFarbgruppe;
                        }
                        
                        //LCD ausgabe~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);
                        writeText(0,11,lcdBuffer);
                        writeText(1,0,"  bezahle       ");
                        sprintf(lcdBuffer,"%u",zahlBetrag);
                        writeText(1,10,lcdBuffer);
                        writeText(2,0,"an Spieler   =>X");
                        sprintf(lcdBuffer,"%u",feldBesitzer);
                        writeText(2,11,lcdBuffer);
                    }
                    
                    //spieler am zug muss Taste X drücken um zu bezahlen
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        //geld überweisen
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag);
                        //prüfen ob zahlung erfolgreich
                        if (bezahlStatus == 1)
                        {
                            updateLCD = 0;
                            flagZahlungAbgeschlossen = 1;
                            flagSpielLCD = 1;
                        }
                    }
                }
                break;
                //wenn das aktuelle Feld ein Steuerfeld ist
                case STEUERFELD:
                if(!(bezahlStatus == 1))
                {
                    flagZahlungAbgeschlossen = 0;
                    if (!updateLCD)
                    {
                        //miete aus array auslesen
                        zahlBetrag = spielfeld[aktuellePosition].preis;
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);
                        writeText(0,11,lcdBuffer);
                        writeText(1,0,"  bezahle       ");
                        sprintf(lcdBuffer,"%u",zahlBetrag);
                        writeText(1,10,lcdBuffer);
                        writeText(2,0,"an die Bank  =>X");
                        updateLCD = 1;
                    }
                        
                    //spieler am zug muss Taste X drücken um zu bezahlen
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        //geld an die Bank überweisen
                        bezahlStatus = geldUeberweisen(spielerAmZug,0,zahlBetrag);
                        if (bezahlStatus == 1)
                        {
                            updateLCD = 0;
                            flagZahlungAbgeschlossen = 1;
                            flagSpielLCD = 1;
                        }
                    }
                }
                break;
                case HALTESTELLE://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                //wenn die Haltestelle noch niemandem gehört
                if (spielfeld[aktuellePosition].besitzer == 0)
                {
                    flagKaufAbgechlossen = 0;
                    verkaufSpielerEingabe = feldKaufen(aktuellePosition,spielfeld,spielerAmZug);
                    switch (verkaufSpielerEingabe)
                    {
                        case 0://es wurde noch keine Eingabe getätigt
                        break;
                        case 1://Der Spieler hat das Feld gekauft
                        flagKaufAbgechlossen = 1;
                        break;
                        case 2://Das Feld wurde nicht gekauft
                        flagKaufAbgechlossen = 1;
                        //bezahlstatus auf 1 setzen, ansonsten müsste spieler auf feld nach auktion miete bezahlen
                        bezahlStatus = 1;
                        zustand = VERSTEIGERUNG;
                        break;
                    }
                }
                feldbelastet = spielfeld[aktuellePosition].feldBelastet;
                if((spielfeld[aktuellePosition].besitzer && !(spielfeld[aktuellePosition].besitzer == spielerAmZug)) && !(bezahlStatus == 1) && !feldbelastet)
                {
                    flagZahlungAbgeschlossen = 0;
                    if (!updateLCD)
                    {
                        //besitzer des feldes aus array auslesen
                        feldBesitzer = spielfeld[aktuellePosition].besitzer;
                        //miete aus array auslesen
                        
                        haltestelleFarbgruppe = 0;
                        for (uint8_t i = 5; i <= 35; i = i + 10)
                        {
                            if (spielfeld[i].besitzer == spielfeld[aktuellePosition].besitzer)
                            {
                                //wenn dem selben spieler noch eine Haltestelle gehört, wird der zähler erhöt
                                haltestelleFarbgruppe += 1; 
                            }
                        }
                        //vom zähler muss 1 abgezogen werden da eine haltestelle dem spieler gehören muss
                        haltestelleFarbgruppe -= 1;
                        zahlBetrag = spielfeld[aktuellePosition].mieten[haltestelleFarbgruppe];
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);
                        writeText(0,11,lcdBuffer);
                        writeText(1,0,"  bezahle       ");
                        sprintf(lcdBuffer,"%u",zahlBetrag);
                        writeText(1,10,lcdBuffer);
                        writeText(2,0,"an Spieler   =>X");
                        sprintf(lcdBuffer,"%u",feldBesitzer);
                        writeText(2,11,lcdBuffer);
                        updateLCD = 1;
                    }
                    
                    //spieler am zug muss Taste X drücken um zu bezahlen
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag);
                        if (bezahlStatus == 1)
                        {
                            updateLCD = 0;
                            flagZahlungAbgeschlossen = 1;
                            flagSpielLCD = 1;
                        }
                    }
                }
                break;
                case GEFAENGNIS://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                if (flagGefaengnis)//nur wenn spieler im workshop
                {
                    if (spielerInfo[spielerAmZug].gefaengnis && !flagGefaengnisLCD)
                    {
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);
                        writeText(0,11,lcdBuffer);
                        writeText(1,0,"   Du bist im   ");
                        writeText(2,0,"    Workshop    ");
                        flagGefaengnisLCD = 1;
                        _delay_ms(5000); //5s Auf LCD anzeigen
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
                    }
                    switch (workshopZustand)
                    {
                        case FREIKARTE_J_N:
                        if (flagTasteY)//spieler will Freikarte nicht verwenden
                        {
                            writeText(1,0," Pasch    X=JA  ");
                            writeText(2,0,"W"UE"rfeln   Y=NEIN");
                            //wechselt zur nächsten option
                            workshopZustand = PASCH_J_N;
                        }
                        else if (flagTasteX) //spieler will Freikarte verwenden
                        {
                            spielerInfo[spielerAmZug].gefaengnis = 0;
                            spielerInfo[spielerAmZug].freikarte = 0;
                            spielerInfo[spielerAmZug].rundenImGefaengnis = 0;
                            flagFertigGewuerfelt = 0;//lässt spieler würfeln
                            flagGefaengnis = 0;
                            writeText(1,0," w"UE"rfeln A / B ");
                            writeText(2,0,"                ");
                        }
                        break;
                        case PASCH_J_N:
                        if (flagTasteY)//Spieler will keinen Pasch würfeln
                        {
                            //wechselt zum nächsten zustand
                            workshopZustand = BEZAHLEN_J_N;
                            writeText(1,0,"   50     X=JA  ");
                            writeText(2,0,"Bezahlen  Y=NEIN");
                        }
                        else if (flagTasteX)//Spieler will Pasch würfeln
                        {
                            writeText(1,0," w"UE"rfeln A / B ");
                            writeText(2,0,"                ");
                            //wechselt zu zustand Pasch
                            workshopZustand = PASCH;
                        }
                        break;
                        case BEZAHLEN_J_N:
                        if (flagTasteY)//Spieler will nicht zahlen
                        {
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
                                writeText(1,0," Pasch    X=JA  ");
                                writeText(2,0,"W"UE"rfeln   Y=NEIN");
                            }
                        }
                        else if (flagTasteX)//Spieler will bezahlen
                        {
                            //zieht den betrag vom spieler ab
                            bezahlStatus = geldUeberweisen(spielerAmZug,0,50);
                            //prüft ob zahlung erfolgreich
                            if (bezahlStatus == ZAHLUNG_ERFOLGREICH)
                            {
                                writeText(1,0,"     Zahlung    ");
                                writeText(2,0,"   Erfolgreich  ");
                                //entlässt den spieler aus dem gefängnis
                                spielerInfo[spielerAmZug].gefaengnis = 0;
                                spielerInfo[spielerAmZug].rundenImGefaengnis = 0;
                                flagFertigGewuerfelt = 0;//lässt spieler würfeln
                                flagGefaengnis = 0;
                            }
                            else if (bezahlStatus == ZAHLUNG_FEHLGESCHLAGEN)
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
                                /*sprintf(lcdBuffer,"%u",spielerInfo[spielerAmZug].rundenImGefaengnis);
                                writeText(1,0,lcdBuffer);
                                writeText(1,0,"   Runden X=JA  ");
                                writeText(2,0,"  warten  Y=NEIN");*/
                            }
                        }
                        break;
                        case PASCH:
                        //wartet bis mit beiden Würfel gewürfelt wurde
                        flagWuerfel1 = 0;
                        flagWuerfel2 = 0;
                        warteBisGewuerfelt();
                        //prüft ob ein Pasch gewürfelt wurde
                        if (wuerfelArray[0] == wuerfelArray[1])
                        {
                            writeText(1,0,"     PASCH      ");
                            writeText(2,0,"                ");
                            //läst den spieler mit dem letzten wurf fahren
                            flagFertigGewuerfelt = 0;
                            spielerInfo[spielerAmZug].rundenImGefaengnis = 0;
                            spielerInfo[spielerAmZug].gefaengnis = 0;
                            _delay_ms(5000); //5s Warten
                        }
                        else
                        {
                            writeText(1,0,"   KEIN PASCH   ");
                            writeText(2,0,"                ");
                            flagWuerfel1 = 0;
                            flagWuerfel2 = 0;
                            //erhöt die gewarteten runden
                            spielerInfo[spielerAmZug].rundenImGefaengnis += 1;
                            //wenn beim 3. versuch kein Pasch gewürfelt wurde
                            if (spielerInfo[spielerAmZug].rundenImGefaengnis == 3)
                            {
                                //nach 3 Runden muss bezahlt werden
                                bezahlStatus = geldUeberweisen(spielerAmZug,0,50);
                                spielerInfo[spielerAmZug].rundenImGefaengnis = 0;
                                spielerInfo[spielerAmZug].gefaengnis = 0;
                                //läst den spieler mit dem letzten wurf fahren
                                flagFertigGewuerfelt = 0; 
                            }
                            else
                            {
                                //Der nächste Spieler ist am zug
                                flagGefaengnisWeiter = 1;
                                _delay_ms(5000); //5s Warten
                            }
                        }
                        break;
                    }
                    if (flagGefaengnisLCD)
                    {
                    }
                    //tasten abfragen
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        flagTasteX = 1;
                        flagTasteY = 0;
                    }
                    else if (positiveFlanke & yTasten[spielerAmZug - 1])
                    {
                        flagTasteX = 0;
                        flagTasteY = 1;
                    }
                    else
                    {
                        flagTasteX = 0;
                        flagTasteY = 0;
                    }
                }
                break;
                case GEH_INS_GEFAENGNIS://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                abInsGefaengnis(spielerAmZug);
                aktuellesFeld = GEFAENGNIS;
                spielerInfo[spielerAmZug].gefaengnis = 1;
                flagWeiter = 1;
                flagFertigGewuerfelt = 1;
                break;
                case FREIPARKEN://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                break;
                case WERK://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                if (spielfeld[aktuellePosition].besitzer == 0)
                {
                    flagKaufAbgechlossen = 0;
                    verkaufSpielerEingabe = feldKaufen(aktuellePosition,spielfeld,spielerAmZug);
                    switch (verkaufSpielerEingabe)
                    {
                        case 0://es wurde noch keine Eingabe getätigt
                        break;
                        case 1://Der Spieler hat das Feld gekauft
                        flagKaufAbgechlossen = 1;
                        break;
                        case 2://Das Feld wurde nicht gekauft
                        flagKaufAbgechlossen = 1;
                        //bezahlstatus auf 1 setzen, ansonsten müsste spieler auf feld nach auktion miete bezahlen
                        bezahlStatus = 1;
                        zustand = VERSTEIGERUNG;
                        break;
                    }
                }
                feldbelastet = spielfeld[aktuellePosition].feldBelastet;
                if((spielfeld[aktuellePosition].besitzer && !(spielfeld[aktuellePosition].besitzer == spielerAmZug)) && !(bezahlStatus == 1) && !feldbelastet)
                {
                    flagZahlungAbgeschlossen = 0;
                    if (!updateLCD)
                    {
                        if (spielfeld[12].besitzer == spielfeld[28].besitzer)
                        {
                            multiplikator = 10;
                        }
                        else
                        {
                            multiplikator = 4;
                        }
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);
                        writeText(0,11,lcdBuffer);
                        writeText(1,0,"  W"UE"rfeln A / B  ");
                        
                        writeText(2,0,"W"UE"rfelsumme x   ");
                        sprintf(lcdBuffer,"%u",multiplikator);
                        writeText(2,14,lcdBuffer);

                        //wartet bis mit beiden Würfel gewürfelt wurde
                        while (!(flagWuerfel1 && flagWuerfel2))
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
                        }
                        //besitzer des feldes aus array auslesen
                        feldBesitzer = spielfeld[aktuellePosition].besitzer;
                        //miete aus array auslesen
                        zahlBetrag = (wuerfelArray[0] + wuerfelArray[1]) * multiplikator;
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);
                        writeText(0,11,lcdBuffer);
                        writeText(1,0,"  bezahle       ");
                        sprintf(lcdBuffer,"%u",zahlBetrag);
                        writeText(1,10,lcdBuffer);
                        writeText(2,0,"an Spieler   =>X");
                        sprintf(lcdBuffer,"%u",feldBesitzer);
                        writeText(2,11,lcdBuffer);
                        updateLCD = 1;
                    }
                    
                    //spieler am zug muss Taste X drücken um zu bezahlen
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag);
                        if (bezahlStatus == 1)
                        {
                            updateLCD = 0;
                            flagZahlungAbgeschlossen = 1;
                            flagSpielLCD = 1;
                            flagWuerfel1 = 0;
                            flagWuerfel2 = 0;
                        }
                    }
                }
                
                break;
            }
            break;
            case VERSTEIGERUNG://Zustand indem Versteigerungen stattfinden
            //LCD aktualisieren
            if (!updateLCD)
            {
                clear();
                writeText(0,0," VERSTEIGERUNG  ");
                writeText(1,0,spielfeld[aktuellePosition].name);
                writeText(2,0,"bieten X sonst Y");
                aktuellesGebot = 9; //startgebot
                updateLCD = 1; //LCD nicht mehr aktualisiern
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    //Speichert den aktuellen kontostand der Spieler
                    geldZwischenspeicher[i] = spielerInfo[i].geld;
                    //Geld Siebensegmente ausschalten
                    setGeld(0,i,0);
                }
            }
            //verarbeitet tasten eingaben der spieler
            for (uint8_t i = 0; i < anzahlSpieler; i = i + 1)
            {
                //Gebot wird abgegeben, wenn der spieler noch dabei ist und er genug geld hat
                if (((positiveFlanke & xTasten[i]) && !bieter[i]) && (spielerInfo[i+1].geld > (aktuellesGebot))) 
                {
                    aktuellesGebot = aktuellesGebot + 1; //aktuelles Gebot erhöhen
                    for (uint8_t j = 1; j <= anzahlSpieler; j = j + 1)
                    {
                        setGeld(0,j,0); //Geld Siebensegmente ausschalten
                    }
                    //Geld Siebensegmente vom höchstbieter einschalten
                    setGeld(aktuellesGebot,i + 1,1); 
                    bieter[5] = i + 1; //Speichert Spieler Nummer vom Höchstbieter      bieter[5] speicher höchstbieter
                }
                //wenn ein spieler die Taste Y betätigt bietet er nicht mehr mit
                if ((positiveFlanke & yTasten[i]) && !bieter[i])
                {
                    bieter[i] = 1; //schliesst spieler aus auktion aus                  bieter 0 - 3 spieler zurückgetreten
                    bieter[4] = bieter[4] + 1; //erhöht anzahl zurückgezogene spieler   bieter 4 anzahl zurückgetretene Spieler
                }
            }
            if (bieter[4] == anzahlSpieler) //wenn alle Spieler aus der Auktion zurückgetretn sind
            {
                if (!(bieter[5] == 0)) //wenn jemand die Auktion gewonnen hat
                {
                    //LCD Leeren und neu beschreiben
                    clear();
                    _delay_ms(10);
                    writeText(0,0," VERSTEIGERT an ");
                    writeText(1,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",bieter[5]);
                    writeText(1,11,lcdBuffer);
                    _delay_ms(4000); //delay um Spieler zeit zu lassen LCD zu lesen
                    //Geld aus Zwischenspeicher zurück auf Spielerkonto
                    for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                    {
                        spielerInfo[i].geld = geldZwischenspeicher[i];
                    }
                    //kontostand aktualisieren
                    updateKontostand(anzahlSpieler,spielerInfo);
                    //kontostand von Höchstbieter um gebot verkleinern
                    spielerInfo[bieter[5]].geld = (spielerInfo[bieter[5]].geld - aktuellesGebot);
                    //besitz überschreiben
                    spielfeld[aktuellePosition].besitzer = bieter[5];
                    //beitz RGB einschalten
                    setPropertyRgb(spielfeld[aktuellePosition].rgbNummer,bieter[5]);
                    //bieter array zurücksetzen
                    for (uint8_t i = 0; i < 6; i = i + 1)
                    {
                        bieter[i] = 0;
                    }
                    //flags setzen
                    updateLCD = 0;
                    //flagVersteigert = 1;
                    flagSpielLCD = 1;
                    //zum spiel zurückkehren
                    zustand = SPIEL;
                }
                else
                {
                    //LCD Leeren und neu beschreiben
                    clear();
                    writeText(0,0,"     nicht     ");
                    writeText(1,0,"   VERSTEIGERT  ");
                    _delay_ms(4000);
                    //Geld aus Zwischenspeicher zurück auf Spielerkonto
                    for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                    {
                        spielerInfo[i].geld = geldZwischenspeicher[i];
                    }
                    //kontostand aktualisieren
                    updateKontostand(anzahlSpieler,spielerInfo);
                    //bieter array zurücksetzen
                    for (uint8_t i = 0; i < 6; i = i + 1)
                    {
                        bieter[i] = 0;
                    }
                    //flags setzen
                    updateLCD = 0;
                    //flagVersteigert = 1;
                    flagSpielLCD = 1;
                    //zum spiel zurückkehren
                    zustand = SPIEL;
                }
            }
            break;
            case BAUEN://Zustand indem gebaut wird
            hausBauen(spielerAmZug);
            flagSpielLCD = 1;//Standart LCD Maske anzeigen
            zustand = SPIEL;
            break;
            //Zustand durchden man versteigern, Bauen und Handeln kann
            case VERWALTEN://in diesem zustand, kann man in andere zustände rein navigieren
            switch (verwaltung)
            {
                case VERWALTUNG_BAUEN:
                if (!updateLCD)
                {
                    writeText(0,0,"   Verwaltung   ");
                    writeText(1,0,"     Bauen?     ");
                    writeText(2,0,"C zur"UE"ck|weiter"PFEIL_R);
                    updateLCD = 1;
                }
                if (positiveFlanke & TASTE_R)
                {
                    //zustand wechseln
                    verwaltung = HYPOTHEK;
                    updateLCD = 0;
                }
                else if (positiveFlanke & TASTE_S)
                {
                    //zurück zum Spiel
                    verwaltung = VERWALTUNG_BAUEN;
                    zustand = BAUEN;
                    updateLCD = 0;
                }
                else if (positiveFlanke & TASTE_C)
                {
                    zustand = SPIEL;
                    flagSpielLCD = 1;
                }
                break;
                case HYPOTHEK:
                if (!updateLCD)
                {
                    writeText(0,0,"   Verwaltung   ");
                    writeText(1,0,"    Hypothek    ");
                    writeText(2,0,"C zur"UE"ck|weiter"PFEIL_R);
                    updateLCD = 1;
                }
                if (positiveFlanke & TASTE_R)
                {
                    //zustand wechseln
                    verwaltung = VERWALTUNG_HANDELN;
                    updateLCD = 0;
                }
                else if (positiveFlanke & TASTE_S)
                {
                    //zurück zum Spiel
                    verwaltung = VERWALTUNG_BAUEN;
                    zustand = VERPFAENDEN;
                    updateLCD = 0;
                }
                else if (positiveFlanke & TASTE_C)
                {
                    zustand = SPIEL;
                    flagSpielLCD = 1;
                }
                break;
                case VERWALTUNG_HANDELN:
                if (!updateLCD)
                {
                    writeText(0,0,"   Verwaltung   ");
                    writeText(1,0,"    Handeln     ");
                    writeText(2,0,"C zur"UE"ck|weiter"PFEIL_R);
                    updateLCD = 1;
                }
                if (positiveFlanke & TASTE_R)
                {
                    //zustand wechseln
                    verwaltung = VERWALTUNG_BAUEN;
                    updateLCD = 0;
                }
                else if (positiveFlanke & TASTE_S)
                {
                    //zurück zum Spiel
                    verwaltung = VERWALTUNG_BAUEN;
                    zustand = HANDELN;
                    updateLCD = 0;
                }
                else if (positiveFlanke & TASTE_C)
                {
                    zustand = SPIEL;
                    flagSpielLCD = 1;
                }
                break;
                default:
                break;
            }
            break;
            case VERPFAENDEN://Zustand indem man eine Hypothek aufnehmen oder auflösen kann
            if (!updateLCD)
            {
                writeText(0,0,"                ");
                writeText(1,0,PFEIL_O"Hyp.|Hyp.aufl."PFEIL_U);
                writeText(2,0,"C zur"UE"ck|weiter"PFEIL_R);
                updateLCD = 1;
                //Spielerinventar zurücksetzen
                for (uint8_t i = 0; i < 28; i = i + 1)
                {
                    spielerInventar[i] = 0;
                }
                anzahlEigentum = 0;
                //sucht alle felder ab nach Felder die dem Spieler gehören
                for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
                {
                    //sucht die spielfelder nach denen ab, die dem Spieler gehören
                    if (spielfeld[i].besitzer == spielerAmZug)
                    {
                        //speichert die Feldnummer im spielerinventar
                        spielerInventar[anzahlEigentum] = i;
                        anzahlEigentum += 1;
                    }
                }
                //Schreibt den namen des ersten feldes auf das LCD
                writeText(0,0,"                ");
                writeText(0,0,spielfeld[spielerInventar[feldZaehler]].name);
            }
            //nächstes Feld mit taste R
            if (positiveFlanke & TASTE_R)
            {
                //zählt die anzahl felder
                feldZaehler = (feldZaehler + 1) % anzahlEigentum;
                //Schreibt den Namen des Feldes auf das LCD
                //Löscht die oberste Zeile auf dem LCD
                writeText(0,0,"                ");
                writeText(0,0,spielfeld[spielerInventar[feldZaehler]].name);
            }
            //wenn spieler Feld verpfänden will
            if (positiveFlanke & TASTE_O)
            {
                flagKeineHaeuser = 1;
                for (uint8_t i = 0; i < 3; i = i + 1)
                {
                    if (spielfeld[spielfeld[spielerInventar[feldZaehler]].farbgruppenFelder[i]].anzahlHaeuser)
                    {
                        flagKeineHaeuser = 0;
                    }
                }
                //prüft ob es auf dem Feld noch Häuser hatt und ob das Feld bereits belastet ist
                //if (!(spielfeld[spielerInventar[feldZaehler]].anzahlHaeuser) && !(spielfeld[spielerInventar[feldZaehler]].feldBelastet))
                if (!(spielfeld[spielerInventar[feldZaehler]].feldBelastet) && flagKeineHaeuser)
                {
                    //berechnet den Wert des Feldes
                    zahlBetrag = spielfeld[spielerInventar[feldZaehler]].preis / 2;
                    bezahlStatus = geldUeberweisen(0,spielerAmZug,zahlBetrag);
                    if (bezahlStatus == 1) //wenn die Bezahlung erfolgreich war
                    {
                        //vermerkt das feld als verpfändet
                        spielfeld[spielerInventar[feldZaehler]].feldBelastet = 1;
                        //Markiert das Feld als verpfändet
                        hausNummer = 0;
                        hausNummer = spielfeld[spielerInventar[feldZaehler]].hausnummer;
                        rgbFeldNummer = spielfeld[spielerInventar[feldZaehler]].rgbNummer;
                        if (spielfeld[spielerInventar[feldZaehler]].typ == STRASSE)//wenn es eine Strase ist
                        {
                            //Markiert das Feld als verpfändet
                            //alle 5 haus LEDs werden eingeschaltet
                            setHaus(hausNummer,6);
                        }
                        else //wenn es keine strasse ist
                        {
                            //wenn es keine häuser hat, die man als Markierung nutzen kann
                            // wird die RGB auf weiss gestellt.
                            setPropertyRgb(rgbFeldNummer,5);
                        }
                    }
                }
            }
            else if (positiveFlanke & TASTE_U) //wenn der Spieler die hypothek auflösen will
            {
                //prüft ob das Feld belastet ist
                if (spielfeld[spielerInventar[feldZaehler]].feldBelastet)
                {
                    //berechnet den Preis um eine Hypothek aufzulösen
                    zahlBetrag = (spielfeld[spielerInventar[feldZaehler]].preis / 2) * 1.1;
                    //Geld wird an Bank überwiesen
                    bezahlStatus = geldUeberweisen(spielerAmZug,0,zahlBetrag);
                    if (bezahlStatus == 1) //wenn die Zahlung erfolgreich war
                    {
                        //vermerkt das feld als nicht mehr verpfändet
                        spielfeld[spielerInventar[feldZaehler]].feldBelastet = 0;
                        hausNummer = 0;
                        hausNummer = spielfeld[spielerInventar[feldZaehler]].hausnummer;
                        rgbFeldNummer = spielfeld[spielerInventar[feldZaehler]].rgbNummer;
                        if (spielfeld[spielerInventar[feldZaehler]].typ == STRASSE)//wenn es eine Strase ist
                        {
                            //Markiert das Feld als nicht mehr verpfändet
                            setHaus(hausNummer,0);
                        }
                        else //wenn es keine strasse ist
                        {
                            //markiert das Feld wieder mit der Spielerfarbe
                            setPropertyRgb(rgbFeldNummer,spielerAmZug);
                        }
                    }
                }
                
            }
            if (positiveFlanke & TASTE_C)
            {
                zustand = SPIEL;
                flagSpielLCD = 1;
            }
            
            break;
            case HANDELN://zustand in dem gehandelt wird
            switch (handelZustand)
            {
                case HAENDLER_AUSWAHL://auswählen, wer mit wem handelt
                if (!updateLCD)//LCD einmal schreiben
                {
                    writeText(0,0,"beide h"AE"ndler   ");
                    writeText(1,0,"taste x dr"UE"cken ");
                    writeText(2,0,"                ");
                    updateLCD = 1;
                    haendlerZaehler = 0;
                }
                //for schleife fragt alle x tasten ab
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    //überprüft die x tasten und stellt sicher dass der 1. händler nicht nochmal gedrückt hat
                    if ((positiveFlanke & xTasten[i - 1]) && !(i == handel[0].spielerNr))
                    {
                        //Speichert die Spielernummer des haendlers
                        handel[haendlerZaehler].spielerNr = i;//speichert die Spielernummer
                        haendlerZaehler += 1;//zähler wird erhöt
                    }
                }
                //prüft ob im 2.speicher ein händler eingetragen ist
                if (handel[1].spielerNr)
                {
                    globalUpdateLCD = 0;
                    handelZustand = WARE_AUSWAEHLEN;//zustandswechsel
                    haendlerZaehler = 0;
                }
            	break;
                case WARE_AUSWAEHLEN://ware die gehandelt werden soll auswählen
                //solaange nicht beide Spieler mit ihrer Auswahl fertig sind
                while (haendlerZaehler < 2)
                {
                    auswahlAbgeschlossen = handelWareAuswaehlen(haendlerZaehler);
                    if (auswahlAbgeschlossen)
                    {
                        haendlerZaehler += 1;//zähler erhöhen
                        auswahlAbgeschlossen = 0;
                    }
                }
                if (haendlerZaehler == 2)
                {
                    handelZustand = HANDEL_BESTAETIGEN;//zustandswechsel
                    globalUpdateLCD = 0;
                    haendlerZaehler = 0;
                    auswahlAbgeschlossen = 0;
                }
                break;
                case HANDEL_BESTAETIGEN://handel bestätigen
                //blaulicht(100,10);
                writeText(1,0,"   Best"AE"tigen   ");
                auswahlAbgeschlossen = auswahlBestaetigen(haendlerZaehler);
                if (auswahlAbgeschlossen)
                {
                    haendlerZaehler += 1;//zähler erhöhen
                    auswahlAbgeschlossen = 0;
                    if (haendlerZaehler == 2)
                    {
                        handelZustand = BESITZ_UEBERTRAGEN;//zustandswechsel
                        globalUpdateLCD = 0;
                    }
                }
                else
                handelZustand = HANDEL_ABSCHLIESSEN;
                break;
                case BESITZ_UEBERTRAGEN://ausgewählte ware übertragen
                clear();
                _delay_ms(1000);
                writeText(0,0,"   "UE"bertragen   ");
                _delay_ms(1000);
                handelSpielerNummern[0] = handel[0].spielerNr;
                handelSpielerNummern[1] = handel[1].spielerNr;
                //Grundstücke übertragen
                for (uint8_t i = 0; i < 2; i = i + 1)
                {
                    for (uint8_t j = 0; handel[i].feldNummern[j] > 0; j = j + 1)
                    {
                        //speichert die zu übertragende Feldnummer 
                        handelFeldNummer = handel[i].feldNummern[j];
                        spielfeld[handelFeldNummer].besitzer = handelSpielerNummern[1 - i];//Besitz übertragen
                        //wenn das verkaufte Feld belastet ist
                        if (spielfeld[handelFeldNummer].feldBelastet)
                        {
                            flagHandelBelastet = 1;
                            globalUpdateLCD = 0;
                            while(flagHandelBelastet)
                            {
                                //Flankenerkennung
                                tasteAlt = tasteNeu;
                                tasteNeu = 0;
                                tasteNeu = (PINL << 8) | PINK;
                                positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                                if (!globalUpdateLCD)
                                {
                                    writeText(0,0,"   Spieler      ");
                                    sprintf(lcdBuffer,"%u",handelSpielerNummern[1 - i]);
                                    writeText(0,11,lcdBuffer);
                                    writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                                    writeText(2,0,"andere option  "PFEIL_U);
                                    globalUpdateLCD = 1;
                                }
                                if (positiveFlanke & TASTE_O)
                                {
                                    if (!handelHypothek)//Hypothek soll aufgelöst werden
                                    {
                                        handelBezahlBetrag = spielfeld[handelFeldNummer].preis;
                                        handelBezahlBetrag = (handelBezahlBetrag / 2) + (handelBezahlBetrag / 20);
                                        writeText(1,0,"Zahle          S");
                                        sprintf(lcdBuffer,"%u",handelBezahlBetrag);
                                        writeText(1,6,lcdBuffer);
                                    }
                                    else
                                    {
                                        handelBezahlBetrag = spielfeld[handelFeldNummer].preis;
                                        handelBezahlBetrag = (handelBezahlBetrag / 20);
                                        writeText(1,0,"Zahle          S");
                                        sprintf(lcdBuffer,"%u",handelBezahlBetrag);
                                        writeText(1,6,lcdBuffer);
                                    }
                                    //warte bis der Spieler mit Taste X bestätigt hat
                                    while (!(positiveFlanke & TASTE_S) && !(positiveFlanke & TASTE_U))
                                    {
                                        //Flankenerkennung
                                        tasteAlt = tasteNeu;
                                        tasteNeu = 0;
                                        tasteNeu = (PINL << 8) | PINK;
                                        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                                    }
                                    if (positiveFlanke & TASTE_S)
                                    {
                                        handelzahlungErfolgreich = geldUeberweisen(handelSpielerNummern[1 - i],0,handelBezahlBetrag);
                                    }
                                    else
                                    {
                                        handelHypothek = (handelHypothek + 1) % 2;
                                        if (handelHypothek)
                                        {
                                            writeText(1,0,"Hyp. Behalten  "PFEIL_O);
                                        }
                                        else
                                        {
                                            writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                                        }
                                    }
                                    //Wenn die Zahlung erfolgreich war
                                    if (handelzahlungErfolgreich == 1)
                                    {
                                        //wenn die Hypothek aufgelöst werden soll
                                        if (!handelHypothek)
                                        {
                                            //feld als nicht mehr belastet speichern
                                            spielfeld[handelFeldNummer].feldBelastet = 0;
                                            setHaus(spielfeld[handelFeldNummer].hausnummer,0);
                                        }
                                        //flagHandelBelastet auf 0 setzen um aus der schleife raus zu kommen
                                        flagHandelBelastet = 0;
                                        handelzahlungErfolgreich = 0;
                                    }
                                    
                                }
                                else if (positiveFlanke & TASTE_U)
                                {
                                    handelHypothek = (handelHypothek + 1) % 2;
                                    if (handelHypothek)
                                    {
                                        writeText(1,0,"Hyp. Behalten  "PFEIL_O);
                                    }
                                    else
                                    {
                                        writeText(1,0,"Hyp. Aufl"OE"sen  "PFEIL_O);
                                    }
                                }
                            }
                        }
                        setPropertyRgb(spielfeld[handelFeldNummer].rgbNummer, spielfeld[handelFeldNummer].besitzer);//Neuer besitzer an RGB ausgeben
                    }
                }
                for (uint8_t i = 0; i < 2; i = i + 1)
                {
                    //wenn Bargeld gehandelt wird
                    if (handel[i].barGeld)
                    {
                        //überweist das Geld
                        geldUeberweisen(handelSpielerNummern[i],handelSpielerNummern[1 - i],handel[i].barGeld);
                    }
                }
                for (uint8_t i = 0; i < 2; i = i + 1)
                {
                    if(handel[i].freikarte)//prüft ob Freikarten gehandelt werden
                    {
                        spielerInfo[handelSpielerNummern[i]].freikarte -= 1;//Anzahl Freikarten vom aktuellen Spieler um 1 verkleinern
                        spielerInfo[handelSpielerNummern[1 - i]].freikarte += 1;//Anzahl Freikarten des anderen Spielers um 1 erhöhen
                    }
                }
                handelZustand = HANDEL_ABSCHLIESSEN;
                break;
                case HANDEL_ABSCHLIESSEN://handel abschliessen
                writeText(0,0,"     Handel     ");
                writeText(0,0," abgeschlossen  ");
                _delay_ms(1000);
                flagSpielLCD = 1;
                initialisiereHandelInventar(handel);//initialisiert das handelinventar
                handelZustand = HAENDLER_AUSWAHL;
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

uint8_t feldKaufen(uint8_t feldNummer, Feld spielfeld[40], uint8_t spielerAmZug)
{
    char lcdBuffer[16];
    uint8_t spielerEingabe = 0;
        if (!globalUpdateLCD) //LCD 1 mal aktualisieren
        {
            clear();
            _delay_ms(100);
            writeText(0,0,"   Spieler      ");
            sprintf(lcdBuffer,"%u",spielerAmZug);
            writeText(0,11,lcdBuffer);
            writeText(1,0,spielfeld[feldNummer].name);
            writeText(2,0,"kaufen? <=N >=J");
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
                updateKontostand(anzahlSpieler,spielerInfo);
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

uint8_t bauen(uint8_t feldNummer, uint8_t spielerAmZug)
{
    uint8_t kaufStatus = 0;
    //wenn ein Hotel gebaut wird
    if (spielfeld[feldNummer].anzahlHaeuser < 5)
    {
        if ((spielfeld[feldNummer].anzahlHaeuser == 4) && hotelsImSpiel < MAX_ANZAHL_HOTELS_IM_SPIEL)
        {
            haeuserImSpiel -= 4;
            hotelsImSpiel += 1;
            
            kaufStatus = geldUeberweisen(spielerAmZug,0,spielfeld[feldNummer].kostenHaus);
            setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser + 1); //Anzahl Häuser um 1 erhöhen
            spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser + 1;//Neue anzahl Häuser speichern
            return 1;//Erfolgreich
        }
        //haus bauen
        else if (haeuserImSpiel < MAX_ANZAHL_HAEUSER_IM_SPIEL)
        {
            haeuserImSpiel += 1;
            
            kaufStatus = geldUeberweisen(spielerAmZug,0,spielfeld[feldNummer].kostenHaus);
            setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser + 1); //Anzahl Häuser um 1 erhöhen
            spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser + 1;//Neue anzahl Häuser speichern
            return 1;//Erfolgreich
        }
        else
        {
            return 0;//Fehlgeschlagen
        }
    }
}

uint8_t abBauen(uint8_t feldNummer, uint8_t spielerAmZug)
{
    uint8_t kaufStatus = 0;
    if ((spielfeld[feldNummer].anzahlHaeuser == 5) && (spielfeld[feldNummer].anzahlHaeuser > 0))
    {
        haeuserImSpiel += 4;
        hotelsImSpiel -= 1;
        setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser - 1); //Anzahl Häuser um 1 erhöhen
        spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser - 1;//Neue anzahl Häuser speichern
        kaufStatus = geldUeberweisen(0,spielerAmZug,spielfeld[feldNummer].kostenHaus / 2);
        return 1;
    }
    else if (spielfeld[feldNummer].anzahlHaeuser > 0)
    {
        haeuserImSpiel -= 1;
        setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser - 1); //Anzahl Häuser um 1 erhöhen
        spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser - 1;//Neue anzahl Häuser speichern
        kaufStatus = geldUeberweisen(0,spielerAmZug,spielfeld[feldNummer].kostenHaus / 2);
        return 1;
    }
    else
    {
        return 0;
    }
    
}

void warteBisGewuerfelt(void)
{
    //wartet bis mit beiden Würfel gewürfelt wurde
    while (!(flagWuerfel1 && flagWuerfel2))
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
    }
}

uint8_t handelWareAuswaehlen(uint8_t haendlerNr)
{
    char lcdBuffer[16] = {0};
    //flankenerkennung
    tasteAlt = tasteNeu;
    tasteNeu = 0;
    tasteNeu = (PINL << 8) | PINK;
    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
    
    switch (handelware)
    {
        case GRUNDSTUECK:
        //Nachfolgender Code wird einmal durchgeführt~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if (!globalUpdateLCD)
        {
            /*sprintf(lcdBuffer,"%u",handel[haendlerNr].spielerNr);
            writeText(0,11,lcdBuffer);*/
            writeText(0,0,"   Grundst"UE"ck   ");
            writeText(1,0,"S Handel|weiter"PFEIL_R);
            writeText(2,0,"                ");
            globalUpdateLCD = 1;
            
            //handelbareFelder zurücksetzen
            for (uint8_t i = 0; i < 28; i = i + 1)
            {
                handelbareFelder[i] = 0;
            }
            anzahlHandelbareFelder = 0;
            //sucht alle felder ab nach Felder die dem Spieler gehören
            for (uint8_t i = 0; i < ANZAHL_FELDER; i = i + 1)
            {
                //sucht die spielfelder nach denen ab, die dem Spieler gehören und keine Häuser haben
                if ((spielfeld[i].besitzer == handel[haendlerNr].spielerNr) && (spielfeld[i].anzahlHaeuser == 0))
                {
                    flagHandelbar = 1;
                    for (uint8_t j = 0; j < 3; j = j + 1)
                    {
                        if (spielfeld[spielfeld[i].farbgruppenFelder[j]].anzahlHaeuser)//wenn es auf der Farbgruppe keine Häuser hatt
                        {
                            flagHandelbar = 0;
                        }
                        
                    }
                    if (flagHandelbar)
                    {
                        //speichert die Feldnummer im spielerinventar
                        handelbareFelder[anzahlHandelbareFelder] = i;
                        //anzahlHandelbareFelder erhöhen
                        anzahlHandelbareFelder += 1;
                    }
                }
            }
            //zeigt erstes handelbares Feld auf LCD an
            handelFeld = 0;
            writeText(2,0,"                ");
            writeText(2,0,spielfeld[handelbareFelder[handelFeld]].name);
            
        }
        //Nachfolgender Code wird mehrmals durchgeführt~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if (positiveFlanke & TASTE_R)//nächstes Feld
        {
            //wechselt zum nächsten feld
            handelFeld = (handelFeld + 1) % anzahlHandelbareFelder; 
            //Schreibt das Aktuelle Feld auf das LCD
            writeText(2,0,"                ");
            writeText(2,0,spielfeld[handelbareFelder[handelFeld]].name);
        }
        else if (positiveFlanke & TASTE_S)//Feld bestätigt
        {
            //speichert das aktuelle Feld 
            handel[haendlerNr].feldNummern[anzahlAusgewaehlteFelder] = handelbareFelder[handelFeld];
            anzahlAusgewaehlteFelder += 1;
            handelware = AUSWAHL_BEENDEN;
            globalUpdateLCD = 0;
        }
        else if (positiveFlanke & TASTE_L)//etwas anderes handeln
        {
            writeText(0,0,"    Bargeld     ");
            writeText(1,0,"S Handel|weiter"PFEIL_R);
            writeText(2,0,"                ");
            handelware = BARGELD;
            globalUpdateLCD = 0;
        }
    	break;
        case BARGELD:
        if (!globalUpdateLCD)
        {
            writeText(0,0,"Bargeld 0       ");
            writeText(1,0,"S Handel|weiter"PFEIL_R);
            writeText(2,0,PFEIL_O" +10      +100"PFEIL_U);
            globalUpdateLCD = 1;
        }
        if (positiveFlanke & TASTE_O)
        {
            handel[haendlerNr].barGeld += 10;
            sprintf(lcdBuffer,"%4u",handel[haendlerNr].barGeld);
            writeText(0,8,lcdBuffer);
        }
        else if (positiveFlanke & TASTE_U)
        {
            handel[haendlerNr].barGeld += 100;
            sprintf(lcdBuffer,"%4u",handel[haendlerNr].barGeld);
            writeText(0,8,lcdBuffer);
        }
        //prüft ob der Spieler eine Freikarte hat
        if ((positiveFlanke & TASTE_L) && spielerInfo[handel[haendlerNr].spielerNr].freikarte)
        {
            handelware = FREIKARTEN;//zustandswechsel
            globalUpdateLCD = 0;
        }
        else if(positiveFlanke & TASTE_L)
        {
            handelware = GRUNDSTUECK;//zustandswechsel
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
            handelware = AUSWAHL_BEENDEN;
            globalUpdateLCD = 0;
        }
        break;
        case AUSWAHL_BEENDEN:
        if (!globalUpdateLCD)
        {
            writeText(0,0,"Auswahl Beenden?");
            writeText(1,0,PFEIL_O"Beenden   mehr"PFEIL_U);
            writeText(2,0,"                ");
            globalUpdateLCD = 1;
        }
        if (positiveFlanke & TASTE_O)//Auswahl Beenden~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        {
            anzahlAusgewaehlteFelder = 0;
            handelware = GRUNDSTUECK;
            globalUpdateLCD = 0;
            return 1;
        }
        else if (positiveFlanke & TASTE_U)//mehr auswählen
        {
            globalUpdateLCD = 0;
            handelware = GRUNDSTUECK;
            //handelWareAuswaehlen(haendlerNr); //Funktion erneut aufrufen
        }
        break;
        default:
        break;
    }
    return 0;
}

uint8_t auswahlBestaetigen(uint8_t haendlerNr)
{
    char lcdBuffer[16] = {0};
    uint8_t flagBestaetigt = 1;
    if (!globalUpdateLCD)
    {
        //schreibt die Spielernummer auf das LCD
        writeText(0,0,"   Spieler      ");
        sprintf(lcdBuffer,"%u",handel[1 - haendlerNr].spielerNr);
        writeText(0,11,lcdBuffer);
        writeText(1,0,"   Best"AE"tigen   ");
        writeText(2,0,"                ");
        globalUpdateLCD = 1;
    }
    //prüft ob grundstücke gehandelt wurden
    if (handel[haendlerNr].feldNummern[0])
    {
        for (uint8_t i = 0; handel[haendlerNr].feldNummern[i] > 0; i = i + 1)
        {
            //schreibt den Feldnamen auf das display
            writeText(1,0,"   Best"AE"tigen   ");
            writeText(2,0,spielfeld[handel[haendlerNr].feldNummern[i]].name);
            //wartet bis der andere spieler bestätigt hatt
            while (!((positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1]) || (positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])))
            {
                //Flankenerkennung
                tasteAlt = tasteNeu;
                tasteNeu = 0;
                tasteNeu = (PINL << 8) | PINK;
                positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                //wartet auf benutzereingabe
            }
            if (positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1])
            {
                writeText(1,0,"   Best"AE"tigt    ");
                writeText(2,0,"                ");
                _delay_ms(1000);
            }
            if (positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])
            {
                flagBestaetigt = 0;
            }
            //Flankenerkennung
            tasteAlt = tasteNeu;
            tasteNeu = 0;
            tasteNeu = (PINL << 8) | PINK;
            positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        }
    }
    //prüft ob bargeld gehandelt wird
    if (handel[haendlerNr].barGeld)
    {
        writeText(0,0,"   Spieler      ");
        sprintf(lcdBuffer,"%u",handel[1 - haendlerNr].spielerNr);
        writeText(0,11,lcdBuffer);
        writeText(1,0,"   Best"AE"tigen   ");
        
        writeText(2,0,"Bargeld:        ");
        sprintf(lcdBuffer,"%4u",handel[haendlerNr].barGeld);
        writeText(2,9,lcdBuffer);
        while (!((positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1]) || (positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])))
        {
            //Flankenerkennung
            tasteAlt = tasteNeu;
            tasteNeu = 0;
            tasteNeu = (PINL << 8) | PINK;
            positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
            //wartet auf benutzereingabe
        }
        if (positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1])
        {
            writeText(1,0,"   Best"AE"tigt    ");
            writeText(2,0,"                ");
            _delay_ms(1000);
        }
        if (positiveFlanke & positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])
        {
            flagBestaetigt = 0;
        }
    }
    //prüft ob Freikarten gehandelt wird
    if (handel[haendlerNr].freikarte)
    {
        writeText(0,0,"   Spieler      ");
        sprintf(lcdBuffer,"%u",handel[1 - haendlerNr].spielerNr);
        writeText(0,11,lcdBuffer);
        writeText(1,0,"   Best"AE"tigen   ");
        
        writeText(0,2,"   Freikarte    ");
        while (!((positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1]) || (positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])))
        {
            //Flankenerkennung
            tasteAlt = tasteNeu;
            tasteNeu = 0;
            tasteNeu = (PINL << 8) | PINK;
            positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
            //wartet auf benutzereingabe
        }
        if (positiveFlanke & xTasten[(handel[1 - haendlerNr].spielerNr) - 1])
        {
            writeText(1,0,"   Best"AE"tigt    ");
            writeText(2,0,"                ");
            _delay_ms(1000);
        }
        if (positiveFlanke & positiveFlanke & yTasten[(handel[1 - haendlerNr].spielerNr) - 1])
        {
            flagBestaetigt = 0;
        }
    }
    globalUpdateLCD = 0;
    return flagBestaetigt;
}
