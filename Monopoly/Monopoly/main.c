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

#pragma GCC optimize 0

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
/*--- Datentypen (typedef) --------------------------------------------------*/


typedef enum {SPIELERAUSWAHL, WUERFELSTART, SPIEL, VERSTEIGERUNG, BAUEN, VERWALTEN, VERPFAENDEN} zustand_t;
typedef enum {FREIKARTE_J_N, PASCH_J_N, BEZAHLEN_J_N, PASCH} workshopZustand_t;
typedef enum {HYPOTHEK, VERWALTUNG_BAUEN} verwaltung_t;
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

zustand_t zustand = SPIELERAUSWAHL;
workshopZustand_t workshopZustand = PASCH_J_N;
verwaltung_t verwaltung = VERWALTUNG_BAUEN;
uint8_t anzahlSpieler = 2;

uint8_t globalUpdateLCD = 0;

Feld spielfeld[40];
Karte chanceKanzlei[34];
uint8_t haeuserImSpiel = 0;
uint8_t hotelsImSpiel = 0;

uint8_t flagWuerfel1 = 0;
uint8_t flagWuerfel2 = 0;
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
    
    //Eigenschaften Spieler 2
    strcpy(spielerInfo[2].name, "Spieler 2");
    spielerInfo[2].geld = 2222;
    spielerInfo[2].position = 40;
    spielerInfo[2].gefaengnis = 0;
    spielerInfo[2].rundenImGefaengnis = 0;
    spielerInfo[2].freikarte = 0;
    spielerInfo[2].haeuser = 0;
    spielerInfo[2].hotels = 0;
    
    //Eigenschaften Spieler 3
    strcpy(spielerInfo[3].name, "Spieler 3");
    spielerInfo[3].geld = 3333;
    spielerInfo[3].position = 40;
    spielerInfo[3].gefaengnis = 0;
    spielerInfo[3].rundenImGefaengnis = 0;
    spielerInfo[3].freikarte = 0;
    spielerInfo[3].haeuser = 0;
    spielerInfo[3].hotels = 0;
    
    //Eigenschaften Spieler 4
    strcpy(spielerInfo[4].name, "Spieler 4");
    spielerInfo[4].geld = 4444;
    spielerInfo[4].position = 40;
    spielerInfo[4].gefaengnis = 0;
    spielerInfo[4].rundenImGefaengnis = 0;
    spielerInfo[4].freikarte = 0;
    spielerInfo[4].haeuser = 0;
    spielerInfo[4].hotels = 0;
}
Spieler spielerInfo[5];








int main(void)
{
    char buffer[200];  // Buffer im RAM
    
    
    /*--- Modullokale Konstanten ------------------------------------------------*/
    /*--- Modullokale Variablen -------------------------------------------------*/
    //char
    char lcdBuffer[100];
    //8-Bit Variabeln
    uint8_t spielerAmZug = 1;
    uint8_t flagNextPlayer, flagPasch = 0;
    uint8_t flagWeiter = 1;
    uint8_t aktuellePosition = 0;
    uint8_t xTasten[4] = {TASTE_X1, TASTE_X2, TASTE_X3, TASTE_X4};
    uint8_t yTasten[4] = {TASTE_Y1, TASTE_Y2, TASTE_Y3, TASTE_Y4};
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
    uint8_t flagZahlungAbgeschlossen = 1;
    uint8_t flagKaufAbgechlossen = 1;
    
    uint8_t farbgruppenErstesFeld[8] = {1,6,11,16,21,26,31,37}; //Jeweil das erste Feld einer Strassen Farbgruppe
    uint8_t flagFarbgruppeKomplett = 0;
    uint8_t volleFarbgruppen[8] = {0};
    uint8_t farbgruppenCounter = 0;
    uint8_t feldNummer = 0;
    uint8_t gruppeAnzahlHaeuser = 0;
    uint8_t flagHausFeld1 = 0;
    uint8_t flagHausFeld2 = 0;
    uint8_t flagHausFeld3 = 0;
    uint8_t anzahlHauser[3] = {0};
    uint8_t haeuser = 0;
    uint8_t minHaeuser = 5;
    uint8_t maxHaeuser = 0;
    uint8_t flagBauErfolgreich = 0;
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
    
    uint8_t flagGefaengnis = 0;
    uint8_t flagGefaengnisLCD = 0;
    uint8_t flagGefaengnisWeiter = 0;
    
    uint8_t spielerInventar[28] = {0};
    
    
    /*--- Prototypen modullokaler Funktionen ------------------------------------*/
    uint8_t feldKaufen(uint8_t feldNummer, Feld spielfeld[40], uint8_t spielerAmZug);
    uint8_t bauen(uint8_t feldNummer, uint8_t spielerAmZug);
    uint8_t abBauen(uint8_t feldNummer, uint8_t spielerAmZug);
    void warteBisGewuerfelt(void);
    /*--- Funktionsdefinitionen -------------------------------------------------*/
    
    
    //Initialisierung
    PortInitialisierung();
    lcdInitAll();
    initialisiereSpielfeld(spielfeld);
    initSpieler(spielerInfo);
    initialisiereKarten(chanceKanzlei);
    SPI_init_all(9600);
    resetMonopoly();
    //random Seed setzen
    adm_ADC_init();
    srand(adm_ADC_read(0));
    
    for (uint8_t i = 0; i < 40; i = i + 1)
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
        
        
        
        //verarbeitung verschiedener zustände
        switch (zustand)//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        {
            case SPIELERAUSWAHL://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                        
            if (!spielerSetup)
            {
                //LCD ausgabe
                writeText(0,0,"Spielerauswahl");
                writeText(1,0,PFEIL_L"- 2 Spieler  +"PFEIL_R);
                writeText(2,0,"weiter Taste S");
                //schaltet nicht benötigte siebensegmente aus
                updateKontostand(anzahlSpieler,spielerInfo);
                spielerSetup = 1; //setzt Flag um erneutes ausführen zu verhindern
            }
            
            //Anzahl Spieler verkleinern
            if ((positiveFlanke & TASTE14) && anzahlSpieler > MIN_ANZAHL_SPIELER)
            {
                anzahlSpieler = anzahlSpieler - 1;
                //Ausgabe anzahl spieler auf LCD
                sprintf(lcdBuffer,"%u",anzahlSpieler);
                writeText(1,3,lcdBuffer);
                //Aktive spieler an Konto LCD anzeigen
                updateKontostand(anzahlSpieler,spielerInfo);
            }
            //anzahlspieler vergrössern
            if ((positiveFlanke & TASTE15) && anzahlSpieler < MAX_ANZAHL_SPIELER)
            {
                anzahlSpieler = anzahlSpieler + 1;
                //Ausgabe anzahl spieler auf LCD
                sprintf(lcdBuffer,"%u",anzahlSpieler);
                writeText(1,3,lcdBuffer);
                //Aktive spieler an Konto LCD anzeigen
                updateKontostand(anzahlSpieler,spielerInfo);
            }
            //anzahl spieler bestätigen
            if (positiveFlanke & TASTE13)
            {
                //Setzt das Geld der Spieler auf 0
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    spielerInfo[i].geld = 0; //Kontostand auf 0 setzen
                    spielerInfo[i].position = 0; //Setzt die mitspielenden Spieler auf das Startfeld
                    setPlayerPosition(spielerInfo[i].position,i);
                }
                //Aktive spieler an Konto LCD anzeigen
                updateKontostand(anzahlSpieler,spielerInfo);
                //Ausgabe bestätigte anzahl spieler auf LCD
                writeText(0,0,"      Spieler   ");
                sprintf(lcdBuffer,"%u",anzahlSpieler);
                writeText(0,4,lcdBuffer);
                //Ausgabe, startgeld wird verteilt auf LCD
                writeText(1,0,"   Startgeld    ");
                writeText(2,0," wird verteilt  ");
                startGeldAnimation(anzahlSpieler); //Startgeld wird ausgeteilt
                zustand = WUERFELSTART; //wechselt den Zustand
            }
        	break;
            //in diesem Zustand wird bestimmt wer zuerst würfelt
            case WUERFELSTART://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //Geldanzeige aller Spieler auschalten
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //Schaltet den Output aller Geld Siebensegmente aus
                setGeld(1500,i,SIEBENSEGMENT_OFF);
            }
            //lässt jeden spieler würfeln
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //LCD Ausgabe
                writeText(0,0,"   Spieler X    ");
                sprintf(lcdBuffer,"%u",i);
                writeText(0,11,lcdBuffer);
                writeText(1,0,"    w"UE"rfelt    ");
                writeText(2,0," wuerfeln A / B ");
                
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
                    if ((positiveFlanke & TASTE9) && !flagWuerfel1)
                    {
                        wuerfelAB(1,flagWuerfel1,flagWuerfel2);
                        flagWuerfel1 = 1;
                    }
                    //würfelt würfel 2 nur wenn taste9 betätigt wurde
                    //und würfel 2 noch nicht gewürfelt wurde
                    else if ((positiveFlanke & TASTE10) && !flagWuerfel2)
                    {
                        wuerfelAB(2,flagWuerfel1,flagWuerfel2);
                        flagWuerfel2 = 1;
                    }
                    //prüfft welche würfel als 2. verwendet wurde
                    if (flagWuerfel1 && !letzterWuerfel)
                    {
                        letzterWuerfel = 2; //würfel 2 wird als letztes verwendet
                    }
                    if (flagWuerfel2 && !letzterWuerfel)
                    {
                        letzterWuerfel = 1; //würfel 1 wird als letztes verwendet
                    }
                }
                
                //Setzt das geld des Spielers auf die Summe der Würfel
                spielerInfo[i].geld = wuerfelArray[0] + wuerfelArray[1];
                
                //falls der aktuelle Spieler die gleiche Summe wie Platz 1 hat
                if (spielerInfo[i].geld == spielerInfo[ersterSpieler].geld)
                {
                    //Wenn 2. würfelzahl > 1, dann wird die zahl um 1 verkleinert
                    if (wuerfelArray[letzterWuerfel-1] > 1) 
                    {
                        //verkleinert die 2. gewürfelte zahl
                        wuerfelArray[letzterWuerfel-1] = wuerfelArray[letzterWuerfel-1] - 1;
                    }
                    else
                    {
                        //ansonsten wird die 2. Zahl um 1 erhöht
                        wuerfelArray[letzterWuerfel-1] = wuerfelArray[letzterWuerfel-1] + 1;
                    }
                    //neuer Wert wird gespeichert
                    spielerInfo[i].geld = wuerfelArray[0] + wuerfelArray[1];
                    //neue zahlen werden an die Würfel siebensegmente ausgegeben
                    wuerfelTransmit(wuerfelArray[0],wuerfelArray[1]); 
                }
                
                //wenn der aktuelle Spieler eine grössere zahl hat als platz 1
                //dann wird deraktuelle spieler zu platz 1
                if (spielerInfo[i].geld > spielerInfo[ersterSpieler].geld)
                {
                    ersterSpieler = i;
                }
                
                //setzt die flags wieder auf 0
                flagWuerfel1 = 0;
                flagWuerfel2 = 0;
                letzterWuerfel = 0;
                //ausgabe der gewürfelten Summe
                updateKontostand(i,spielerInfo);
            }
            spielerAmZug = ersterSpieler;
            _delay_ms(1000);
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                spielerInfo[i].geld = 1500;
            }
            updateKontostand(anzahlSpieler,spielerInfo);
            writeText(0,0,"   Spieler      ");
            sprintf(lcdBuffer,"%u",spielerAmZug);
            writeText(0,11,lcdBuffer);
            writeText(1,0," w"UE"rfeln A / B ");
            writeText(2,0,"    weiter C    ");
            
            zustand = SPIEL;
            break;
            case SPIEL://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
            if (flagSpielLCD)
            {
                clear();//lcd leeren
                //spieler am zug anzeigen
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"weiter C  mehr S");
                flagSpielLCD = 0;
            }
            //Spielzug abschliessen alles zurücksetzen
            if(((positiveFlanke & TASTE_C) || flagGefaengnisWeiter) && flagFertigGewuerfelt && flagZahlungAbgeschlossen && flagKaufAbgechlossen && flagEreignisAbgeschlossen)
            {
                //würfel Siebensegmente ausschalten
                wuerfelTransmit(SIEBENSEGMENT_OFF,SIEBENSEGMENT_OFF);
                //nächster spieler
                spielerAmZug = (spielerAmZug % anzahlSpieler) + 1;
                //spieler am zug anzeigen
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"weiter C  mehr S");
                //flags zurücksetzten
                flagFertigGewuerfelt = 0;
                PORTC &= ~0xC0; //Schaltet das Blaulicht aus
                updateLCD = 0;
                bezahlStatus = 0;
                ereignisfeldRueckgabe = 0;
                aktuellesFeld = spielfeld[spielerInfo[spielerAmZug].position].typ;
                flagGefaengnis = 0; //flagGefaengnis zurücksetzen
                flagGefaengnisLCD = 0;
                flagGefaengnisWeiter = 0;
                if ((aktuellesFeld == GEFAENGNIS) && spielerInfo[spielerAmZug].gefaengnis)
                {
                    flagGefaengnis = 1;//flag setzen wenn spieler im gefängnis ist
                    flagFertigGewuerfelt = 1; //wenn dieses Flag gesetzt ist kann nicht gewürfelt werden

                }
            }
            //ermöglicht es dem spieler bei Pasch zu kaufen
            if ((positiveFlanke & TASTE_C) && !flagWeiter && flagZahlungAbgeschlossen && flagEreignisAbgeschlossen)
            {
                //Würfel Siebensegmente ausschalten
                wuerfelTransmit(SIEBENSEGMENT_OFF,SIEBENSEGMENT_OFF);
                flagWeiter = 1; //Flag setzen
                bezahlStatus = 0;
                ereignisfeldRueckgabe = 0;
            }//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Würfel~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //lässt den Spieler einmal würfel
            //Wenn flagWeiter nicht gesetzt ist, kann man nicht würfeln das
            //flag braucht es, da man ansonsten bei einem pasch nichts kaufen kann
            if (!flagFertigGewuerfelt && flagWeiter) 
            {
                //wartet bis mit beiden Würfel gewürfelt wurde
                warteBisGewuerfelt();
                //lässt den spieler bei einem Pasch nochmal würfeln,
                //ausser der Pasch wurde im Workshop gemacht
                if ((wuerfelArray[0] == wuerfelArray[1]) && !flagGefaengnis) //Pasch
                {   
                    flagWeiter = 0; //bei einem Pasch wird flagWeiter = 0 gesetzt
                    flagPasch = flagPasch + 1; //flagPasch erhöhen
                    //beide würfel flags zurücksetzen
                    flagWuerfel1 = 0;
                    flagWuerfel2 = 0;
                    switch (flagPasch) //schaltet LEDs ein bei Pasch
                    {
                        case 1:
                        PORTC |= 0x40; //eine LED
                    	break;
                        case 2:
                        PORTC |= 0x80; //zwei LEDs
                        break;
                        case 3: //beim 3. Pasch = Gefängnis
                        PORTC &= ~0xC0; //Schaltet beide LEDs aus
                        //würfelzahlen zurücksetzen
                        wuerfelArray[0] = 0;
                        wuerfelArray[1] = 0;
                        //flags zurücksetzen
                        flagWuerfel1 = 0;
                        flagWuerfel2 = 0;
                        flagPasch = 0;
                        flagWeiter = 1;
                        flagFertigGewuerfelt = 1;
                        //nach 3. pasch landet man im Gefängnis
                        abInsGefaengnis(spielerAmZug); 
                        aktuellesFeld = GEFAENGNIS;
                        break;
                    }
                }
                else
                {
                    //flag setzen erlaubt es den spielzug abzuschliessen
                    flagFertigGewuerfelt = 1; 
                    flagPasch = 0; //flagPasch zurücksetzen
                }
                 //wenn die aktuelle spition des Spielers + wüfelsumme grösser gleich 40 is
                 // erhält der spieler 200 CHF
                 if (spielerInfo[spielerAmZug].position + (wuerfelArray[0] + wuerfelArray[1]) >= 40 )
                 {
                     spielerInfo[spielerAmZug].geld += 200; //konntostand wird um 200 erhöht
                     updateKontostand(anzahlSpieler,spielerInfo);
                 }
                 //animiert die fortbewegung des spielers
                 for (uint8_t i = spielerInfo[spielerAmZug].position; i < (spielerInfo[spielerAmZug].position + (wuerfelArray[0] + wuerfelArray[1])); i = i + 1)
                 {
                     setPlayerPosition(i % 40,spielerAmZug);
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
                 flagWuerfel1 = 0;
                 flagWuerfel2 = 0;
            }
            
            //kontostand aktualisieren
            updateKontostand(anzahlSpieler,spielerInfo);
            //Spieler hat fertig gewürfelt ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            
            //Wenn Taste S => mehr betätigt wurde
            if (positiveFlanke & TASTE_S)
            {
                /*writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"weiter C  mehr S");*/
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~VERPFÄNDEN~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                updateLCD = 0;
                zustand = VERWALTEN;
            }
            
            
            switch (aktuellesFeld) //verarbeitung aktuelles feld ~~~~~~~~~~~~~~~~~
            {
                case EREIGNISFELD://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                //prüft ob das aktuelle feld Chance ode Kanzlei ist
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
                    flagEreignisAbgeschlossen = 0;
                    if ((positiveFlanke & TASTE_R) || !ereignisSchritt)//Taste zum scrollen
                    {
                        ereignisfeldRueckgabe = ereignisFeld(flagKanzlei,spielerAmZug,ereignisSchritt,flagEreignisWeiter,chanceKanzlei);
                        if (!ereignisfeldRueckgabe)
                        {
                            ereignisSchritt += 2;
                        }
                    }
                    if (positiveFlanke & xTasten[spielerAmZug - 1])
                    {
                        flagEreignisWeiter = 1;
                        ereignisfeldRueckgabe = ereignisFeld(flagKanzlei,spielerAmZug,ereignisSchritt,flagEreignisWeiter,chanceKanzlei);
                        //speichert den aktuellen Feld typ
                        aktuellesFeld = spielfeld[spielerInfo[spielerAmZug].position].typ;
                        //speichert die aktuelle position
                        aktuellePosition = spielerInfo[spielerAmZug].position;
                        
                        flagEreignisWeiter = 0;
                        ereignisSchritt = 0;
                        flagEreignisAbgeschlossen = 1;
                        flagSpielLCD = 1;
                    }
                }
                break;
                case STRASSE://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                //Feld kann gekauft werden wenn es niemandem gehört und man in dieser runde noch nichts gekauft hat
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
                feldbelastet = spielfeld[aktuellePosition].hypothek;
                //wenn das Aktuelle feld einem spieler gehört muss man bezahlen, ausser es gehört einem selbst
                if((spielfeld[aktuellePosition].besitzer && !(spielfeld[aktuellePosition].besitzer == spielerAmZug)) && !(bezahlStatus == 1) && !feldbelastet)
                {
                    flagZahlungAbgeschlossen = 0;
                    if (!updateLCD)
                    {
                        //besitzer des feldes aus array auslesen
                        feldBesitzer = spielfeld[aktuellePosition].besitzer;
                        //miete anhand von anzahl häuser aus array auslesen
                        zahlBetrag = spielfeld[aktuellePosition].mieten[spielfeld[aktuellePosition].anzahlHaeuser];
                        zahlBetragFarbgruppe = spielfeld[aktuellePosition].mieten[6];
                        flagMieteFarbgruppe = 1;
                        
                        //prüft ob die ganze farbgruppe dem selben SPieler gehhört
                        for (uint8_t i = 0; i < 3; i = i + 1)
                        {
                            if (!(spielfeld[spielfeld[aktuellePosition].farbgruppenFelder[i]].besitzer == feldBesitzer))
                            {
                                flagMieteFarbgruppe = 0;
                            }
                        }
                        //bestimmt ob farbgruppenmiete oder miete mit Häuser bezahlt werden muss
                        if (flagMieteFarbgruppe && (zahlBetragFarbgruppe > zahlBetrag))
                        {
                            zahlBetrag = zahlBetragFarbgruppe;
                        }
                        
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
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag,1);
                        if (bezahlStatus == 1)
                        {
                            updateLCD = 0;
                            flagZahlungAbgeschlossen = 1;
                            flagSpielLCD = 1;
                        }
                    }
                }
                break;
                case STEUERFELD://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
                        bezahlStatus = geldUeberweisen(spielerAmZug,0,zahlBetrag,10);
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
                feldbelastet = spielfeld[aktuellePosition].hypothek;
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
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag,5);
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
                            bezahlStatus = geldUeberweisen(spielerAmZug,0,50,1);
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
                                sprintf(lcdBuffer,"%u",spielerInfo[spielerAmZug].rundenImGefaengnis);
                                writeText(1,0,lcdBuffer);
                                writeText(1,0,"   Runden X=JA  ");
                                writeText(2,0,"  warten  Y=NEIN");
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
                                bezahlStatus = geldUeberweisen(spielerAmZug,0,50,1);
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
                feldbelastet = spielfeld[aktuellePosition].hypothek;
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
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag,1);
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
            case VERSTEIGERUNG://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
                if (((positiveFlanke & xTasten[i]) && !bieter[i]) && (spielerInfo[i+1].geld > (aktuellesGebot + 1))) 
                {
                    aktuellesGebot = aktuellesGebot + 1; //aktuelles Gebot erhöhen
                    for (uint8_t j = 1; j <= anzahlSpieler; j = j + 1)
                    {
                        setGeld(0,j,0); //Geld Siebensegmente ausschalten
                    }
                    //Geld Siebensegmente vom höchstbieter einschalten
                    setGeld(aktuellesGebot,i + 1,1); 
                    bieter[5] = i + 1; //Speichert Spieler Nummer vom Höchstbieter
                }
                //wenn ein spieler die Taste Y betätigt bietet er nicht mehr mit
                if ((positiveFlanke & yTasten[i]) && !bieter[i])
                {
                    bieter[i] = 1; //schliesst spieler aus auktion aus
                    bieter[4] = bieter[4] + 1; //erhöht anzahl zurückgezogene spieler
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
            case BAUEN:
            //Felder nach vollen Farbgruppen absuchen~~~~~~~~~~~~~~~~~~~~~~~~~
            for (uint8_t i = 0; i < 8; i = i + 1)//Alle Farbgruppen werden als voll markiert
            {
                volleFarbgruppen[i] = 1;
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
                        if ((!(spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].besitzer == spielerAmZug)) && (spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]) && !(spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].hypothek))//prüft alle Felder der Farbgruppe
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
                        while (!(positiveFlanke & TASTE_U))
                        {
                            //Flankenerkennung
                            tasteAlt = tasteNeu;
                            tasteNeu = 0;
                            tasteNeu = (PINL << 8) | PINK;
                            positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
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
                            for (uint8_t j = 0; j < 3; j = j + 1)
                            {
                                haeuser = spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].anzahlHaeuser;
                                if (haeuser == maxHaeuser)
                                {
                                    anzahlHauser[j] = 1;
                                }
                                else if (spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j])
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
                                //spieler am zug anzeigen
                                
                                writeText(0,0,PFEIL_L"Abbauen  Bauen"PFEIL_R);
                                writeText(1,0,spielfeld[feldNummer].name);
                                writeText(2,0,PFEIL_O"next    weiter"PFEIL_U);
                                /*writeText(1,0," w"UE"rfeln A / B ");
                                writeText(2,0,"    weiter C    ");*/
                                gruppeAnzahlHaeuser = spielfeld[farbgruppenErstesFeld[i]].anzahlHaeuser; //holt die Anzahl Häuser
                                updateLCD = 1;
                            }
                            if ((positiveFlanke & TASTE_R))//Haus Bauen~~~~~~~~~~~~~~~~~~~~~~~~
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
                            if ((positiveFlanke & TASTE_L))//Haus Verkaufen~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
                            if (positiveFlanke & TASTE_O)//nächstes Feld
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
                        
                    }
                    
                }
            }
            //Felder nach vollen Farbgruppen absuchen~~~~~~~~~~~~~~~~~~~~~~~~~
            clear();//lcd leeren
            //spieler am zug anzeigen
            writeText(0,0,"   Spieler      ");
            sprintf(lcdBuffer,"%u",spielerAmZug);
            writeText(0,11,lcdBuffer);
            writeText(1,0," w"UE"rfeln A / B ");
            writeText(2,0,"    weiter C    ");
            minHaeuser = 5;
            maxHaeuser = 0;
            zustand = SPIEL;
            break;
            case VERWALTEN:
            switch (verwaltung)
            {
                case VERWALTUNG_BAUEN:
                if (!updateLCD)
                {
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",spielerAmZug);
                    writeText(0,11,lcdBuffer);
                    writeText(1,0,"     Bauen      ");
                    writeText(2,0,"next "PFEIL_R"  zur"UE"ck S");
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
                break;
                case HYPOTHEK:
                if (!updateLCD)
                {
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",spielerAmZug);
                    writeText(0,11,lcdBuffer);
                    writeText(1,0,"   verpf"AE"nden X   ");
                    writeText(2,0,"next "PFEIL_R"  zur"UE"ck S");
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
                    zustand = VERPFAENDEN;
                    updateLCD = 0;
                }
                break;
                default:
                break;
            }
            break;
            case VERPFAENDEN://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            if (!updateLCD)
            {
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0,"Hyp. aufnehmen "PFEIL_O);
                writeText(2,0,"Hyp. aufl"OE"sen  "PFEIL_U);
                updateLCD = 1;
                //Spielerinventar zurücksetzen
                for (uint8_t i = 0; i < 28; i = i + 1)
                {
                    spielerInventar[i] = 0;
                }
                anzahlEigentum = 0;
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
                writeText(0,0,spielfeld[spielerInventar[feldZaehler]].name);
            }
            //nächstes Feld
            if (positiveFlanke & TASTE_R)
            {
                feldZaehler = (feldZaehler + 1) % anzahlEigentum;
                //Schreibt den Namen des Feldes auf das LCD
                writeText(0,0,"                ");
                writeText(0,0,spielfeld[spielerInventar[feldZaehler]].name);
            }
            //wenn spieler Feld verpfänden will
            if (positiveFlanke & TASTE_O)
            {
                //prüft ob es auf dem Feld noch Häuser hatt und ob das Feld bereits belastet ist
                if (!(spielfeld[spielerInventar[feldZaehler]].anzahlHaeuser) && !(spielfeld[spielerInventar[feldZaehler]].hypothek))
                {
                    //berechnet den Wert des Feldes
                    zahlBetrag = spielfeld[spielerInventar[feldZaehler]].preis / 2;
                    bezahlStatus = geldUeberweisen(0,spielerAmZug,zahlBetrag,1);
                    if (bezahlStatus == 1) //wenn die Bezahlung erfolgreich war
                    {
                        //vermerkt das feld als verpfändet
                        spielfeld[spielerInventar[feldZaehler]].hypothek = 1;
                        //Markiert das Feld als verpfändet
                        hausNummer = 0;
                        hausNummer = spielfeld[spielerInventar[feldZaehler]].hausnummer;
                        rgbFeldNummer = spielfeld[spielerInventar[feldZaehler]].rgbNummer;
                        if (hausNummer)//wenn es eine Strase ist
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
                if (spielfeld[spielerInventar[feldZaehler]].hypothek)
                {
                    //berechnet den Preis um eine Hypothek aufzulösen
                    zahlBetrag = (spielfeld[spielerInventar[feldZaehler]].preis / 2) * 1.1;
                    //Geld wird an Bank überwiesen
                    bezahlStatus = geldUeberweisen(spielerAmZug,0,zahlBetrag,1);
                    if (bezahlStatus == 1) //wenn die Zahlung erfolgreich war
                    {
                        //vermerkt das feld als nicht mehr verpfändet
                        spielfeld[spielerInventar[feldZaehler]].hypothek = 0;
                        hausNummer = 0;
                        hausNummer = spielfeld[spielerInventar[feldZaehler]].hausnummer;
                        rgbFeldNummer = spielfeld[spielerInventar[feldZaehler]].rgbNummer;
                        if (hausNummer)//wenn es eine Strase ist
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
                spielerInfo[spielerAmZug].geld = spielerInfo[spielerAmZug].geld - spielfeld[feldNummer].preis;
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
    if ((spielfeld[feldNummer].anzahlHaeuser == 4) && hotelsImSpiel < MAX_ANZAHL_HOTELS_IM_SPIEL)
    {
        haeuserImSpiel -= 4;
        hotelsImSpiel += 1;
        
        kaufStatus = geldUeberweisen(spielerAmZug,0,spielfeld[feldNummer].kostenHaus,10);
        setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser + 1); //Anzahl Häuser um 1 erhöhen
        spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser + 1;//Neue anzahl Häuser speichern
        return 1;//Erfolgreich
    }
    //haus bauen
    else if (haeuserImSpiel < MAX_ANZAHL_HAEUSER_IM_SPIEL)
    {
        haeuserImSpiel += 1;
        
        kaufStatus = geldUeberweisen(spielerAmZug,0,spielfeld[feldNummer].kostenHaus,10);
        setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser + 1); //Anzahl Häuser um 1 erhöhen
        spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser + 1;//Neue anzahl Häuser speichern
        return 1;//Erfolgreich
    }
    else
    {
        return 0;//Fehlgeschlagen
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
        kaufStatus = geldUeberweisen(0,spielerAmZug,spielfeld[feldNummer].kostenHaus / 2, 5);
        return 1;
    }
    else if (spielfeld[feldNummer].anzahlHaeuser > 0)
    {
        haeuserImSpiel -= 1;
        setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser - 1); //Anzahl Häuser um 1 erhöhen
        spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser - 1;//Neue anzahl Häuser speichern
        kaufStatus = geldUeberweisen(0,spielerAmZug,spielfeld[feldNummer].kostenHaus / 2, 5);
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