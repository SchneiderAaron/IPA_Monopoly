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

#define TASTE_U TASTE12 //Taste Hoch
#define TASTE_S TASTE13 //Taste Select
#define TASTE_D TASTE16 //Taste Runter
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


//LCD Pfeile
#define PFEIL_R 126
#define PFEIL_L 8
#define PFEIL_O 0
#define PFEIL_U 1
//#define UMLAUT_U "\u00DC"
//#define UMLAUT_U "š"


/*--- Datentypen (typedef) --------------------------------------------------*/


typedef enum {SPIELERAUSWAHL, WUERFELSTART, SPIEL, VERSTEIGERUNG, BAUEN} zustand_t;
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

uint8_t anzahlSpieler = 2;

uint8_t globalUpdateLCD = 0;


void initSpieler(Spieler spielerInfo[])
{
    //Eigenschaften Spieler 1
    strcpy(spielerInfo[1].name, "Spieler 1");
    spielerInfo[1].geld = 1111;
    spielerInfo[1].position = 40;
    
    //Eigenschaften Spieler 2
    strcpy(spielerInfo[2].name, "Spieler 2");
    spielerInfo[2].geld = 2222;
    spielerInfo[2].position = 40;
    
    //Eigenschaften Spieler 3
    strcpy(spielerInfo[3].name, "Spieler 3");
    spielerInfo[3].geld = 3333;
    spielerInfo[3].position = 40;
    
    //Eigenschaften Spieler 4
    strcpy(spielerInfo[4].name, "Spieler 4");
    spielerInfo[4].geld = 4444;
    spielerInfo[4].position = 40;
}
Spieler spielerInfo[5];


int main(void)
{
    /*--- Modullokale Konstanten ------------------------------------------------*/
    /*--- Modullokale Variablen -------------------------------------------------*/
    //char
    char lcdBuffer[16];
    //8-Bit Variabeln
    uint8_t spielerAmZug = 1;
    uint8_t flagNextPlayer, flagPasch = 0;
    uint8_t flagWeiter = 1;
    uint8_t aktuellePosition = 0;
    uint8_t xTasten[4] = {TASTE_X1, TASTE_X2, TASTE_X3, TASTE_X4};
    uint8_t yTasten[4] = {TASTE_Y1, TASTE_Y2, TASTE_Y3, TASTE_Y4};
    uint8_t bieter[6] = {0};//0-3 Bieter 4 anz. spieler raus 5 höchstbieter
    uint8_t ersterSpieler = 0;
    
    
    
    uint8_t spielerSetup = 0;
    
    uint8_t flagFertigGewuerfelt, flagWuerfel1, flagWuerfel2, letzterWuerfel = 0;
    
    uint8_t flagKaufAbgechlossen, flagVersteigert, verkaufSpielerEingabe = 0;
    
    uint8_t updateLCD = 0;
    
    uint8_t feldBesitzer = 0;
    uint8_t bezahlStatus = 0;
    uint8_t flagZahlungAbgeschlossen = 0;
    
    uint8_t farbgruppenErstesFeld[8] = {1,6,11,16,21,26,31,37}; //Jeweil das erste Feld einer Strassen Farbgruppe
    uint8_t flagFarbgruppeKomplett = 0;
    uint8_t volleFarbgruppen[8] = {0};
    uint8_t farbgruppenCounter = 0;
    uint8_t feldNummer = 0;
    
    uint16_t zahlBetrag = 0;
    
    //16-Bit Variabeln
    uint16_t geldZwischenspeicher[5] = {0};
    uint16_t aktuellesGebot = 0;
    //Eigene Datentypen
    Feld spielfeld[40];
    FeldTyp aktuellesFeld = FREIPARKEN;
    
    uint8_t flagGefaengnis = 0;
    
    
    /*--- Prototypen modullokaler Funktionen ------------------------------------*/
    uint8_t feldKaufen(uint8_t feldNummer, Feld spielfeld[40], uint8_t spielerAmZug);
    /*--- Funktionsdefinitionen -------------------------------------------------*/
    
    
    //Initialisierung
    PortInitialisierung();
    lcdInitAll();
    initialisiereSpielfeld(spielfeld);
    initSpieler(spielerInfo);
    SPI_init_all(9600);
    resetMonopoly();
    //random Seed setzen
    adm_ADC_init();
    srand(adm_ADC_read(0));
    while (1) 
    {
        //Flankenerkennung
        tasteAlt = tasteNeu;
        tasteNeu = 0;
        tasteNeu = (PINL << 8) | PINK;
        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        
        if (positiveFlanke & TASTE_U)//Wenn Taste runter gedrückt wird => Bauen
        {
            zustand = BAUEN;
        }
        //verarbeitung verschiedener zustände
        switch (zustand)//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        {
            case SPIELERAUSWAHL://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            if (!spielerSetup)
            {
                //LCD ausgabe
                writeText(0,0,"Spielerauswahl");
                writeText(1,1,"- 2 Spieler  +");
                displayCharacterAt(1,0,PFEIL_L);
                displayCharacterAt(1,15,PFEIL_R);
                writeText(2,0,"weiter Taste S ");
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
            if (flagVersteigert || flagZahlungAbgeschlossen)
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
            }
            //Spielzug abschliessen alles zurücksetzen
            if(((positiveFlanke & TASTE_C) && flagFertigGewuerfelt))
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
                writeText(2,0,"    weiter C    ");
                //flags zurücksetzten
                flagFertigGewuerfelt = 0;
                flagKaufAbgechlossen = 0;
                PORTC &= ~0xC0; //Schaltet das Blaulicht aus
                updateLCD = 0;
                bezahlStatus = 0;
                aktuellesFeld = spielfeld[spielerInfo[spielerAmZug].position].typ;
                if ((aktuellesFeld == GEFAENGNIS) && spielerImGefaengnis[spielerAmZug])
                {
                    flagGefaengnis = 1;
                    flagFertigGewuerfelt = 1; //wenn dieses Flag gesetzt ist kann nicht gewürfelt werden
                }
            }
            //ermöglicht es dem spieler bei Pasch zu kaufen
            if ((positiveFlanke & TASTE_C) && !flagWeiter)
            {
                //Würfel Siebensegmente ausschalten
                wuerfelTransmit(SIEBENSEGMENT_OFF,SIEBENSEGMENT_OFF);
                flagWeiter = 1; //Flag setzen
                flagKaufAbgechlossen = 0;
                bezahlStatus = 0;
            }//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Würfel~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //lässt den Spieler einmal würfel
            //Wenn flagWeiter nicht gesetzt ist, kann man nicht würfeln das
            //flag braucht es, da man ansonsten bei einem pasch nichts kaufen kann
            if (!flagFertigGewuerfelt && flagWeiter) 
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
                if (wuerfelArray[0] == wuerfelArray[1]) //Pasch
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
            }
            //Kann möglicherweise in nach oben verschiben werden ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!!!!!!!
            flagWuerfel1 = 0;
            flagWuerfel2 = 0;
            
            //kontostand aktualisieren
            updateKontostand(anzahlSpieler,spielerInfo);
            //Spieler hat fertig gewürfelt ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            
            
            switch (aktuellesFeld) //verarbeitung aktuelles feld ~~~~~~~~~~~~~~~~~
            {
                case EREIGNISFELD://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                break;
                case STRASSE://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                //Feld kann gekauft werden wenn es niemandem gehört und man in dieser runde noch nichts gekauft hat
                if (!flagKaufAbgechlossen && (spielfeld[aktuellePosition].besitzer == 0))
                {
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
                 //wenn das Aktuelle feld einem spieler gehört muss man bezahlen, ausser es gehört einem selbst
                if((spielfeld[aktuellePosition].besitzer && !(spielfeld[aktuellePosition].besitzer == spielerAmZug)) && !(bezahlStatus == 1))
                {
                    flagZahlungAbgeschlossen = 0;
                    if (!updateLCD)
                    {
                        //besitzer des feldes aus array auslesen
                        feldBesitzer = spielfeld[aktuellePosition].besitzer;
                        //miete anhand von anzahl häuser aus array auslesen
                        zahlBetrag = spielfeld[aktuellePosition].mieten[spielfeld[aktuellePosition].anzahlHaeuser];
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
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag);
                        if (bezahlStatus == 1)
                        {
                            updateLCD = 0;
                            flagZahlungAbgeschlossen = 1;
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
                        }
                    }
                }
                break;
                case HALTESTELLE://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                if (!flagKaufAbgechlossen && (spielfeld[aktuellePosition].besitzer == 0))
                {
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
                
                if((spielfeld[aktuellePosition].besitzer && !(spielfeld[aktuellePosition].besitzer == spielerAmZug)) && !(bezahlStatus == 1))
                {
                    flagZahlungAbgeschlossen = 0;
                    if (!updateLCD)
                    {
                        //besitzer des feldes aus array auslesen
                        feldBesitzer = spielfeld[aktuellePosition].besitzer;
                        //miete aus array auslesen
                        zahlBetrag = spielfeld[aktuellePosition].mieten[0];
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
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag);
                        if (bezahlStatus == 1)
                        {
                            updateLCD = 0;
                            flagZahlungAbgeschlossen = 1;
                        }
                    }
                }
                break;
                case GEFAENGNIS://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                if (spielerImGefaengnis[spielerAmZug] && flagGefaengnis)
                {
                    writeText(0,0,"   Spieler      ");
                    sprintf(lcdBuffer,"%u",spielerAmZug);
                    writeText(0,11,lcdBuffer);
                    writeText(1,0,"   Du bist im   ");
                    writeText(2,0,"    Workshop    ");
                    flagGefaengnis = 0;
                }
                break;
                case GEH_INS_GEFAENGNIS://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                
                abInsGefaengnis(spielerAmZug);
                aktuellesFeld = GEFAENGNIS;
                break;
                case FREIPARKEN://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                break;
                case WERK://~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                if (!flagKaufAbgechlossen && (spielfeld[aktuellePosition].besitzer == 0))
                {
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
                
                if((spielfeld[aktuellePosition].besitzer && !(spielfeld[aktuellePosition].besitzer == spielerAmZug)) && !(bezahlStatus == 1))
                {
                    flagZahlungAbgeschlossen = 0;
                    if (!updateLCD)
                    {
                        //besitzer des feldes aus array auslesen
                        feldBesitzer = spielfeld[aktuellePosition].besitzer;
                        //miete aus array auslesen
                        zahlBetrag = spielfeld[aktuellePosition].preis;
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
                        bezahlStatus = geldUeberweisen(spielerAmZug,feldBesitzer,zahlBetrag);
                        if (bezahlStatus == 1)
                        {
                            updateLCD = 0;
                            flagZahlungAbgeschlossen = 1;
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
                    flagVersteigert = 1;
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
                    flagVersteigert = 1;
                    //zum spiel zurückkehren
                    zustand = SPIEL;
                }
            }
            break;
            case BAUEN:
            
            //zurück zum Spiel~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~     
            /*if (positiveFlanke & TASTE_C) //zurück zum Spiel
            {
                clear();//lcd leeren
                //spieler am zug anzeigen
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0," w"UE"rfeln A / B ");
                writeText(2,0,"    weiter C    ");
                zustand = SPIEL;
            }
            if (positiveFlanke & TASTE_L)//Nächste bebaubare Farbgruppe
            {
                
            }*/
            
            
            //Felder nach vollen Farbgruppen absuchen~~~~~~~~~~~~~~~~~~~~~~~~~
            for (uint8_t i = 0; i < 8; i = i + 1)//Alle Farbgruppen werden als voll markiert
            {
                volleFarbgruppen[i] = 1;
            }
            for (uint8_t i = 0;  i < 8; i = i + 1)
            {
                flagFarbgruppeKomplett = 1;
                if (spielfeld[farbgruppenErstesFeld[i]].besitzer == spielerAmZug) //überprüft ob erstes feld einer Farbgrup0pe dem Spieler gehört
                {
                    for (uint8_t j = 0; j < 3; j = j + 1)//Prüft ob Farbgruppen tatsächlich voll sind
                    {
                        if (!spielfeld[spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[j]].besitzer == spielerAmZug)//prüft alle Felder der Farbgruppe
                        {
                            flagFarbgruppeKomplett = 0; //setzt flag auf 0 wenn ein Feld nicht dem Spieler gehört
                            volleFarbgruppen[i] = 0; //Markiert Farbgruppe als unvollständig
                        }
                    }
                    if (flagFarbgruppeKomplett)
                    {
                        updateLCD = 0;
                        //nächstes Feld
                        feldNummer = farbgruppenErstesFeld[i];
                        while (!(positiveFlanke & TASTE_D))
                        {
                            //Flankenerkennung
                            tasteAlt = tasteNeu;
                            tasteNeu = 0;
                            tasteNeu = (PINL << 8) | PINK;
                            positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                            
                            if (!updateLCD)
                            {
                                clear();//lcd leeren
                                //spieler am zug anzeigen
                                writeText(0,0,"   Haus Bauen   ");
                                writeText(1,0,spielfeld[feldNummer].name);
                                writeText(2,0,"S=Kaufen");
                                /*writeText(1,0," w"UE"rfeln A / B ");
                                writeText(2,0,"    weiter C    ");*/
                                updateLCD = 1;
                            }
                            if (positiveFlanke & TASTE_S)//Haus Bauen
                            {
                                setHaus(spielfeld[feldNummer].hausnummer,spielfeld[feldNummer].anzahlHaeuser + 1); //Anzahl Häuser um 1 erhöhen
                                spielfeld[feldNummer].anzahlHaeuser = spielfeld[feldNummer].anzahlHaeuser + 1;//Neue anzahl Häuser speichern
                            }
                            if (positiveFlanke & TASTE_R)//nächstes Feld
                            {
                                farbgruppenCounter = (farbgruppenCounter + 1) % 3;
                                feldNummer = spielfeld[farbgruppenErstesFeld[i]].farbgruppenFelder[farbgruppenCounter];
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
            zustand = SPIEL;
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