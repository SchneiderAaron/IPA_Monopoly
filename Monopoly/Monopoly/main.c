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

#include "SPI.h"
#include "MonopolyTreiber.h"
#include "LCD.h"
/*--- #define-Konstanten und Makros -----------------------------------------*/
#define MIN_ANZAHL_SPIELER 2
#define MAX_ANZAHL_SPIELER 4

#define SIEBENSEGMENT_OFF 10

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

#define ARROW_R 126
#define ARROW_L 8
#define ARROW_T 0
#define ARROW_B 1

#pragma GCC optimize 0
/*--- Datentypen (typedef) --------------------------------------------------*/
typedef enum {
    STRASSE,
    EREIGNISFELD,
    STEUERFELD,
    FREIPARKEN,
    GEFAENGNIS,
    GEH_INS_GEFAENGNIS,
    WERK,
    HALTESTELLE
} FeldTyp;

typedef enum {
    BRAUN,
    HELLBLAU,
    ROSA,
    ORANGE,
    ROT,
    GELB,
    GRUEN,
    BLAU,
    FARBLOS
} Farbe;

typedef struct {
    char name[50];       // Name des Feldes
    FeldTyp typ;         // Typ des Feldes
    uint16_t preis;           // Kaufpreis (falls relevant)
    uint16_t miete;           // Miete (falls relevant)
    uint8_t besitzer;        // Besitzer (Index des Spielers, 0 = unbesetzt)
    Farbe farbGruppe;
    uint8_t hausnummer;
    uint8_t anzahlHaeuser;
    uint8_t rgbNummer;
} Feld;

typedef enum {SPIELERAUSWAHL, wuerfelStart, RESERVE,} zustand_t;
/*--- Globale Konstanten ----------------------------------------------------*/
/*--- Globale Variablen -----------------------------------------------------*/

uint8_t houses[14][8] = {0};
uint8_t hausRegister[14] = {0};
uint8_t spieler[20][8] = {0};
uint8_t spielerPos[4] = {0};

uint8_t siebensegment[16] = {0};
uint8_t wuerfelArray[2] = {0};
    
/*--- Modullokale Konstanten ------------------------------------------------*/
/*--- Modullokale Variablen -------------------------------------------------*/
uint8_t xTasten[4] = {TASTE2, TASTE3, TASTE6, TASTE8};
uint8_t ersterSpieler = 1;
/*--- Prototypen modullokaler Funktionen ------------------------------------*/
/*--- Funktionsdefinitionen -------------------------------------------------*/

void initSpieler(Spieler spielerInfo[])
{
    //Eigenschaften Spieler 1
    strcpy(spielerInfo[1].name, "Spieler 1");
    spielerInfo[1].geld = 1111;
    spielerInfo[1].position = 0;
    
    //Eigenschaften Spieler 2
    strcpy(spielerInfo[2].name, "Spieler 2");
    spielerInfo[2].geld = 2222;
    spielerInfo[2].position = 0;
    
    //Eigenschaften Spieler 3
    strcpy(spielerInfo[3].name, "Spieler 3");
    spielerInfo[3].geld = 3333;
    spielerInfo[3].position = 0;
    
    //Eigenschaften Spieler 4
    strcpy(spielerInfo[4].name, "Spieler 4");
    spielerInfo[4].geld = 4444;
    spielerInfo[4].position = 0;
}
Spieler spielerInfo[5];

void initialisiereSpielfeld(Feld spielfeld[])
{
    //Eigenschaften des Feldes: Los
    strcpy(spielfeld[0].name, "Los");
    spielfeld[0].typ = FREIPARKEN;



    //Eigenschaften des Feldes: Eingangshalle
    strcpy(spielfeld[1].name, "Eingangshalle");
    spielfeld[1].typ = STRASSE;
    spielfeld[1].preis = 60;
    spielfeld[1].miete = 2;
    spielfeld[1].besitzer = 0;
    spielfeld[1].farbGruppe = BRAUN;
    spielfeld[1].hausnummer = 0;
    spielfeld[1].anzahlHaeuser = 0;
    spielfeld[1].rgbNummer = 0;
    
    //Eigenschaften des Feldes: Kanzlei
    strcpy(spielfeld[2].name, "Kanzlei1");
    spielfeld[2].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Velokeller
    strcpy(spielfeld[3].name, "Velokeller");
    spielfeld[3].typ = STRASSE;
    spielfeld[3].preis = 60;
    spielfeld[3].miete = 2;
    spielfeld[3].besitzer = 0;
    spielfeld[3].farbGruppe = BRAUN;
    spielfeld[3].hausnummer = 1;
    spielfeld[3].anzahlHaeuser = 0;
    spielfeld[3].rgbNummer = 1;
    
    //Eigenschaften des Feldes: Laptopgebühr
    strcpy(spielfeld[4].name, "Laptopgebuehr");
    spielfeld[4].typ = STEUERFELD;
    spielfeld[4].preis = 200;
    
    //Eigenschaften des Feldes: Zeughausstrasse
    strcpy(spielfeld[4].name, "Zeughausstrasse");
    spielfeld[5].typ = HALTESTELLE;
    spielfeld[5].preis = 200;
    spielfeld[5].miete = 200;
    spielfeld[5].besitzer = 0;
    spielfeld[5].farbGruppe = FARBLOS;
    spielfeld[5].rgbNummer = 2;
    
    //Eigenschaften des Feldes: Raucherzelt
    strcpy(spielfeld[3].name, "Raucherzelt");
    spielfeld[6].typ = STRASSE;
    spielfeld[6].preis = 100;
    spielfeld[6].miete = 2;
    spielfeld[6].besitzer = 0;
    spielfeld[6].farbGruppe = HELLBLAU;
    spielfeld[6].hausnummer = 2;
    spielfeld[6].anzahlHaeuser = 0;
    spielfeld[6].rgbNummer = 3;
    
    //Eigenschaften des Feldes: Chance1
    strcpy(spielfeld[7].name, "Chance1");
    spielfeld[7].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Pausenraum
    strcpy(spielfeld[8].name, "Pausenraum");
    spielfeld[8].typ = STRASSE;
    spielfeld[8].preis = 100;
    spielfeld[8].miete = 2;
    spielfeld[8].besitzer = 0;
    spielfeld[8].farbGruppe = HELLBLAU;
    spielfeld[8].hausnummer = 3;
    spielfeld[8].anzahlHaeuser = 0;
    spielfeld[8].rgbNummer = 4;
    
    //Eigenschaften des Feldes: Garderobe
    strcpy(spielfeld[9].name, "Garderobe");
    spielfeld[9].typ = STRASSE;
    spielfeld[9].preis = 120;
    spielfeld[9].miete = 2;
    spielfeld[9].besitzer = 0;
    spielfeld[9].farbGruppe = HELLBLAU;
    spielfeld[9].hausnummer = 4;
    spielfeld[9].anzahlHaeuser = 0;
    spielfeld[9].rgbNummer = 5;
    
    //Eigenschaften des Feldes: Gefängnis
    strcpy(spielfeld[10].name, "Gefaengnis");
    spielfeld[10].typ = GEFAENGNIS;
    
    
    
    
    //Eigenschaften des Feldes: Herren Wc
    strcpy(spielfeld[11].name, "Herren WC");
    spielfeld[11].typ = STRASSE;
    spielfeld[11].preis = 140;
    spielfeld[11].miete = 2;
    spielfeld[11].besitzer = 0;
    spielfeld[11].farbGruppe = ROSA;
    spielfeld[11].hausnummer = 5;
    spielfeld[11].anzahlHaeuser = 0;
    spielfeld[11].rgbNummer = 6;
    
    //Eigenschaften des Feldes: Informatikdienst
    strcpy(spielfeld[11].name, "Informatikdienst");
    spielfeld[12].typ = WERK;
    spielfeld[12].preis = 150;
    spielfeld[12].miete = 2;
    spielfeld[12].besitzer = 0;
    spielfeld[12].farbGruppe = FARBLOS;
    spielfeld[12].rgbNummer = 7;
    
    //Eigenschaften des Feldes: Frauen WC
    strcpy(spielfeld[13].name, "Frauen WC");
    spielfeld[13].typ = STRASSE;
    spielfeld[13].preis = 140;
    spielfeld[13].miete = 2;
    spielfeld[13].besitzer = 0;
    spielfeld[13].farbGruppe = ROSA;
    spielfeld[13].hausnummer = 6;
    spielfeld[13].anzahlHaeuser = 0;
    spielfeld[13].rgbNummer = 8;
    
    //Eigenschaften des Feldes: Lehrer WC
    strcpy(spielfeld[14].name, "Lehrer WC");
    spielfeld[14].typ = STRASSE;
    spielfeld[14].preis = 160;
    spielfeld[14].miete = 2;
    spielfeld[14].besitzer = 0;
    spielfeld[14].farbGruppe = ROSA;
    spielfeld[14].hausnummer = 7;
    spielfeld[14].anzahlHaeuser = 0;
    spielfeld[14].rgbNummer = 9;
    
    //Eigenschaften des Feldes: Fotozentrum
    strcpy(spielfeld[15].name, "Fotozentrum");
    spielfeld[15].typ = HALTESTELLE;
    spielfeld[15].preis = 200;
    spielfeld[15].miete = 200;
    spielfeld[15].besitzer = 0;
    spielfeld[15].farbGruppe = FARBLOS;
    spielfeld[15].rgbNummer = 10;
    
    //Eigenschaften des Feldes: BFS Polimechaniker
    strcpy(spielfeld[16].name, "BFS Polimechaniker");
    spielfeld[16].typ = STRASSE;
    spielfeld[16].preis = 180;
    spielfeld[16].miete = 2;
    spielfeld[16].besitzer = 0;
    spielfeld[16].farbGruppe = ORANGE;
    spielfeld[16].hausnummer = 8;
    spielfeld[16].anzahlHaeuser = 0;
    spielfeld[16].rgbNummer = 11;
    
    //Eigenschaften des Feldes: Kanzlei2
    strcpy(spielfeld[17].name, "Kanzlei 2");
    spielfeld[17].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: BFS Automatiker
    strcpy(spielfeld[18].name, "BFS Automatiker");
    spielfeld[18].typ = STRASSE;
    spielfeld[18].preis = 180;
    spielfeld[18].miete = 2;
    spielfeld[18].besitzer = 0;
    spielfeld[18].farbGruppe = ORANGE;
    spielfeld[18].hausnummer = 9;
    spielfeld[18].anzahlHaeuser = 0;
    spielfeld[18].rgbNummer = 12;
    
    //Eigenschaften des Feldes: BFS Elektroniker
    strcpy(spielfeld[19].name, "BFS Elektroniker");
    spielfeld[19].typ = STRASSE;
    spielfeld[19].preis = 200;
    spielfeld[19].miete = 2;
    spielfeld[19].besitzer = 0;
    spielfeld[19].farbGruppe = ORANGE;
    spielfeld[19].hausnummer = 10;
    spielfeld[19].anzahlHaeuser = 0;
    spielfeld[19].rgbNummer = 13;
    
    //Eigenschaften des Feldes: Freiparken
    strcpy(spielfeld[20].name, "Freiparken");
    spielfeld[20].typ = FREIPARKEN;
    
    //Eigenschaften des Feldes: Grundausbildung Automatiker
    strcpy(spielfeld[21].name, "Grundausbildung Automatiker");
    spielfeld[21].typ = STRASSE;
    spielfeld[21].preis = 220;
    spielfeld[21].miete = 2;
    spielfeld[21].besitzer = 0;
    spielfeld[21].farbGruppe = ROT;
    spielfeld[21].hausnummer = 11;
    spielfeld[21].anzahlHaeuser = 0;
    spielfeld[21].rgbNummer = 14;
    
    //Eigenschaften des Feldes: Chance2
    strcpy(spielfeld[22].name, "Chance 2");
    spielfeld[22].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Produkton Automatiker
    strcpy(spielfeld[23].name, "Produktion Automatiker");
    spielfeld[23].typ = STRASSE;
    spielfeld[23].preis = 220;
    spielfeld[23].miete = 2;
    spielfeld[23].besitzer = 0;
    spielfeld[23].farbGruppe = ROT;
    spielfeld[23].hausnummer = 12;
    spielfeld[23].anzahlHaeuser = 0;
    spielfeld[23].rgbNummer = 15;
    
    //Eigenschaften des Feldes: Mechatronik Labor
    strcpy(spielfeld[24].name, "Mechatronik Labor");
    spielfeld[24].typ = STRASSE;
    spielfeld[24].preis = 240;
    spielfeld[24].miete = 2;
    spielfeld[24].besitzer = 0;
    spielfeld[24].farbGruppe = ROT;
    spielfeld[24].hausnummer = 13;
    spielfeld[24].anzahlHaeuser = 0;
    spielfeld[24].rgbNummer = 16;
    
    //Eigenschaften des Feldes: Technikum
    strcpy(spielfeld[25].name, "Technikum");
    spielfeld[25].typ = HALTESTELLE;
    spielfeld[25].preis = 200;
    spielfeld[25].miete = 2;
    spielfeld[25].besitzer = 0;
    spielfeld[25].farbGruppe = FARBLOS;
    spielfeld[25].rgbNummer = 17;
    
    //Eigenschaften des Feldes: Lager Polymechaniker
    strcpy(spielfeld[26].name, "Lager Polymechaniker");
    spielfeld[26].typ = STRASSE;
    spielfeld[26].preis = 260;
    spielfeld[26].miete = 2;
    spielfeld[26].besitzer = 0;
    spielfeld[26].farbGruppe = GELB;
    spielfeld[26].hausnummer = 14;
    spielfeld[26].anzahlHaeuser = 0;
    spielfeld[26].rgbNummer = 18;
    
    //Eigenschaften des Feldes: Lager Automatiker
    strcpy(spielfeld[27].name, "Lager Automatiker");
    spielfeld[27].typ = STRASSE;
    spielfeld[27].preis = 260;
    spielfeld[27].miete = 2;
    spielfeld[27].besitzer = 0;
    spielfeld[27].farbGruppe = GELB;
    spielfeld[27].hausnummer = 15;
    spielfeld[27].anzahlHaeuser = 0;
    spielfeld[27].rgbNummer = 19;
    
    //Eigenschaften des Feldes: Putzdienst
    strcpy(spielfeld[28].name, "Putzdienst");
    spielfeld[28].typ = WERK;
    spielfeld[28].preis = 150;
    spielfeld[28].miete = 2;
    spielfeld[28].besitzer = 0;
    spielfeld[28].farbGruppe = FARBLOS;
    spielfeld[28].rgbNummer = 20;

    
    //Eigenschaften des Feldes: Lager Elektroniker
    strcpy(spielfeld[29].name, "Lager Elektroniker");
    spielfeld[29].typ = STRASSE;
    spielfeld[29].preis = 280;
    spielfeld[29].miete = 2;
    spielfeld[29].besitzer = 0;
    spielfeld[29].farbGruppe = GELB;
    spielfeld[29].hausnummer = 16;
    spielfeld[29].anzahlHaeuser = 0;
    spielfeld[29].rgbNummer = 21;
    
    //Eigenschaften des Feldes: Geh ins Gefängnis
    strcpy(spielfeld[30].name, "Geh ins Gefaengnis");
    spielfeld[30].typ = GEH_INS_GEFAENGNIS;
    
    //Eigenschaften des Feldes: Grundausbildung Elektroniker
    strcpy(spielfeld[31].name, "Grundausbildung Elektroniker");
    spielfeld[31].typ = STRASSE;
    spielfeld[31].preis = 300;
    spielfeld[31].miete = 2;
    spielfeld[31].besitzer = 0;
    spielfeld[31].farbGruppe = GRUEN;
    spielfeld[31].hausnummer = 17;
    spielfeld[31].anzahlHaeuser = 0;
    spielfeld[31].rgbNummer = 22;
    
    //Eigenschaften des Feldes: Produktion Elektroniker
    strcpy(spielfeld[32].name, "Produktion Elektroniker");
    spielfeld[32].typ = STRASSE;
    spielfeld[32].preis = 300;
    spielfeld[32].miete = 2;
    spielfeld[32].besitzer = 0;
    spielfeld[32].farbGruppe = GRUEN;
    spielfeld[32].hausnummer = 18;
    spielfeld[32].anzahlHaeuser = 0;
    spielfeld[32].rgbNummer = 23;
    
    //Eigenschaften des Feldes: Kanzlei2
    strcpy(spielfeld[33].name, "Kanzlei 2");
    spielfeld[33].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Entwicklung Elektroniker
    strcpy(spielfeld[34].name, "Entwicklung Elektroniker");
    spielfeld[34].typ = STRASSE;
    spielfeld[34].preis = 320;
    spielfeld[34].miete = 2;
    spielfeld[34].besitzer = 0;
    spielfeld[34].farbGruppe = GRUEN;
    spielfeld[34].hausnummer = 19;
    spielfeld[34].anzahlHaeuser = 0;
    spielfeld[34].rgbNummer = 24;
    
    //Eigenschaften des Feldes: Gewerbeschule
    strcpy(spielfeld[35].name, "Gewerbeschule");
    spielfeld[35].typ = HALTESTELLE;
    spielfeld[35].preis = 200;
    spielfeld[35].miete = 2;
    spielfeld[35].besitzer = 0;
    spielfeld[35].farbGruppe = FARBLOS;
    spielfeld[35].rgbNummer = 25;
    
    //Eigenschaften des Feldes: Chance 3
    strcpy(spielfeld[36].name, "Chance 3");
    spielfeld[36].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Grundausbildung Polymechaniker
    strcpy(spielfeld[37].name, "Grundausbildung Polymechaniker");
    spielfeld[37].typ = STRASSE;
    spielfeld[37].preis = 350;
    spielfeld[37].miete = 2;
    spielfeld[37].besitzer = 0;
    spielfeld[37].farbGruppe = BLAU;
    spielfeld[37].hausnummer = 20;
    spielfeld[37].anzahlHaeuser = 0;
    spielfeld[37].rgbNummer = 26;
    
    //Eigenschaften des Feldes: Schulmaterialkosten
    strcpy(spielfeld[38].name, "Schulmaterialkosten");
    spielfeld[38].typ = STEUERFELD;
    spielfeld[38].preis = 100;
    
    //Eigenschaften des Feldes: Produktion Polymechaniker
    strcpy(spielfeld[39].name, "Produktion Polimechaniker");
    spielfeld[39].typ = STRASSE;
    spielfeld[39].preis = 400;
    spielfeld[39].miete = 2;
    spielfeld[39].besitzer = 0;
    spielfeld[39].farbGruppe = BLAU;
    spielfeld[39].hausnummer = 21;
    spielfeld[39].anzahlHaeuser = 0;
    spielfeld[39].rgbNummer = 27;
}

int main(void)
{
    //Variabeln
    //char
    char lcdBuffer[16];
    //8-Bit Variabeln
    uint8_t spielerAmZug = 1;
    uint8_t flagNextPlayer, flagPasch = 0;
    uint8_t aktuellePosition = 0;
    //16-Bit Variabeln
    uint16_t tasteAlt, tasteNeu, positiveFlanke = 0; //Variabeln Flankenerkennung
    //Eigene Datentypen
    Feld spielfeld[40];
    FeldTyp aktuellesFeld = FREIPARKEN;
    zustand_t zustand = SPIELERAUSWAHL;
    //Initialisierung
    PortInitialisierung();
    lcdInitAll();
    initialisiereSpielfeld(spielfeld);
    initSpieler(spielerInfo);
    SPI_init_all(9600);
    resetMonopoly();
    //random Seed setzen
    srand(adm_ADC_read(0));
    
    
    
    //LCD ausgabe Testen
    /*writeText(0,0,"     Wuerfel    ");
    writeText(1,0,"----------------");
    writeText(2,0,"Spieler ");*/
    //wuerfel();
    uint8_t spielerSetup = 0;
    uint8_t anzahlSpieler = 2;
    uint8_t flagFertigGewuerfelt, flagWuerfel1, flagWuerfel2 = 0;
    while (1) 
    {
        tasteAlt = tasteNeu;
        tasteNeu = 0;
        tasteNeu = (PINL << 8) | PINK;
        positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
        
        switch (zustand)
        {
            case SPIELERAUSWAHL:
            if (!spielerSetup)
            {
                writeText(0,0," Anzahl Spieler ");
                writeText(1,1,"- 2 Spieler  +");
                displayCharacterAt(1,0,ARROW_L);
                displayCharacterAt(1,15,ARROW_R);
                writeText(2,0,"weiter Taste S ");
                updateKontostand(anzahlSpieler,spielerInfo);
                spielerSetup = 1;
            }
            //Anzahl Spieler verkleinern
            if ((positiveFlanke & TASTE14) && anzahlSpieler > MIN_ANZAHL_SPIELER)
            {
                anzahlSpieler = anzahlSpieler - 1;
                sprintf(lcdBuffer,"%u",anzahlSpieler);
                writeText(1,3,lcdBuffer);
                updateKontostand(anzahlSpieler,spielerInfo);
            }
            //anzuahlspieler vergrössern
            if ((positiveFlanke & TASTE15) && anzahlSpieler < MAX_ANZAHL_SPIELER)
            {
                anzahlSpieler = anzahlSpieler + 1;
                sprintf(lcdBuffer,"%u",anzahlSpieler);
                writeText(1,3,lcdBuffer);
                updateKontostand(anzahlSpieler,spielerInfo);
            }
            //anzahl spieler bestätigen
            if (positiveFlanke & TASTE13)
            {
                for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
                {
                    spielerInfo[i].geld = 0;
                }
                updateKontostand(anzahlSpieler,spielerInfo);
                writeText(0,0,"      Spieler   ");
                writeText(0,4,lcdBuffer);
                writeText(1,0,"   Startgeld    ");
                writeText(2,0," wird verteilt  ");
                startGeldAnimation(anzahlSpieler);
                zustand = wuerfelStart;
            }
        	break;
            case wuerfelStart:
            //Geldanzeige aller Spieler auschalten
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                //Schaltet den Output aller Geld Siebensegmente aus
                setGeld(1500,i,SIEBENSEGMENT_OFF);
            }
            for (uint8_t i = 1; i <= anzahlSpieler; i = i + 1)
            {
                writeText(0,0,"   Spieler X    ");
                sprintf(lcdBuffer,"%u",i);
                writeText(0,11,lcdBuffer);
                writeText(1,0,"    wuerfelt    ");
                writeText(2,0,"  X = wuerfeln  ");
                
                while (!(flagWuerfel1 & flagWuerfel2))
                {
                    tasteAlt = tasteNeu;
                    tasteNeu = 0;
                    tasteNeu = (PINL << 8) | PINK;
                    positiveFlanke = (tasteAlt ^ tasteNeu) & tasteNeu;
                    if (positiveFlanke & TASTE9)
                    {
                        wuerfelAB(1);
                        flagWuerfel1 = 1;
                    }
                    else if (positiveFlanke & TASTE10)
                    {
                        wuerfelAB(2);
                        flagWuerfel2 = 1;
                    }
                }
                flagWuerfel1 = 0;
                flagWuerfel2 = 0;
                spielerInfo[i].geld = wuerfelArray[0] + wuerfelArray[1];
                if (spielerInfo[i].geld > spielerInfo[ersterSpieler].geld)
                {
                    ersterSpieler = i;
                }
                if((spielerInfo[4].geld == spielerInfo[ersterSpieler].geld) && !(spielerInfo[ersterSpieler].geld == 12))
                {
                    
                }
                
                
                
                
                
                updateKontostand(i,spielerInfo);
                
                
            }
            zustand = RESERVE;
            break;
            case RESERVE:
            break;
            default:
            break;
        }
    }
}

