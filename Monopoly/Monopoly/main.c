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
#define ARROW_R 126
#define ARROW_L 8
#define ARROW_T 0
#define ARROW_B 1
//#define UMLAUT_U "\u00DC"
#define UMLAUT_U "š"

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
    //uint16_t miete;
    uint16_t mieten[7];
    uint8_t besitzer;        // Besitzer (Index des Spielers, 0 = unbesetzt)
    Farbe farbGruppe;
    uint8_t hausnummer;
    uint8_t anzahlHaeuser;
    uint8_t rgbNummer;
    uint8_t kostenHaus;
    uint8_t feldBelastet;
    uint8_t hypothek;
    uint8_t hypothekAufloesen;
} Feld;

typedef enum {SPIELERAUSWAHL, WUERFELSTART, SPIEL, VERSTEIGERUNG} zustand_t;
/*--- Globale Konstanten ----------------------------------------------------*/
/*--- Globale Variablen -----------------------------------------------------*/

//uint8_t houses[14][8] = {0};
uint8_t hausRegister[14] = {0};
uint8_t spieler[20][8] = {0};
uint8_t spielerPos[4] = {0};

uint8_t siebensegment[16] = {0};
uint8_t wuerfelArray[2] = {0};
    


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
    spielfeld[1].mieten[0] = 2;     //Feld einzeln
    spielfeld[1].mieten[1] = 10;    //Feld mit 1 Haus
    spielfeld[1].mieten[2] = 30;    //Feld mit 2 Häuser
    spielfeld[1].mieten[3] = 90;    //Feld mit 3 Häuser
    spielfeld[1].mieten[4] = 160;   //Feld mit 4 Häuser
    spielfeld[1].mieten[5] = 250;   //Feld mit 1 Hotel
    spielfeld[1].mieten[6] = 4;     //Feld mit Farbgrupp
    spielfeld[1].besitzer = 0;
    spielfeld[1].farbGruppe = BRAUN;
    spielfeld[1].hausnummer = 0;
    spielfeld[1].anzahlHaeuser = 0;
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
    spielfeld[3].hausnummer = 1;
    spielfeld[3].anzahlHaeuser = 0;
    spielfeld[3].rgbNummer = 1;
    spielfeld[3].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    
    //Eigenschaften des Feldes: Laptopgebühr
    strcpy(spielfeld[4].name, "Laptopgeb"UMLAUT_U"hr");
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
    spielfeld[6].hausnummer = 2;
    spielfeld[6].anzahlHaeuser = 0;
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
    spielfeld[8].hausnummer = 3;
    spielfeld[8].anzahlHaeuser = 0;
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
    spielfeld[9].hausnummer = 4;
    spielfeld[9].anzahlHaeuser = 0;
    spielfeld[9].rgbNummer = 5;
    spielfeld[9].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Gefängnis
    strcpy(spielfeld[10].name, "Gef„ngnis");
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
    spielfeld[11].hausnummer = 5;
    spielfeld[11].anzahlHaeuser = 0;
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
    spielfeld[13].hausnummer = 6;
    spielfeld[13].anzahlHaeuser = 0;
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
    spielfeld[14].hausnummer = 7;
    spielfeld[14].anzahlHaeuser = 0;
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
    spielfeld[15].rgbNummer = 10;
    spielfeld[15].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: BFS Polimechaniker
    strcpy(spielfeld[16].name, "BFS Polymechaniker");
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
    spielfeld[16].hausnummer = 8;
    spielfeld[16].anzahlHaeuser = 0;
    spielfeld[16].rgbNummer = 11;
    spielfeld[16].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Kanzlei
    strcpy(spielfeld[17].name, "Kanzlei");
    spielfeld[17].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: BFS Automatiker
    strcpy(spielfeld[18].name, "BFS Automatiker");
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
    spielfeld[18].hausnummer = 9;
    spielfeld[18].anzahlHaeuser = 0;
    spielfeld[18].rgbNummer = 12;
    spielfeld[18].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: BFS Elektroniker
    strcpy(spielfeld[19].name, "BFS Elektroniker");
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
    spielfeld[19].hausnummer = 10;
    spielfeld[19].anzahlHaeuser = 0;
    spielfeld[19].rgbNummer = 13;
    spielfeld[19].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Freiparken
    strcpy(spielfeld[20].name, "Freiparken");
    spielfeld[20].typ = FREIPARKEN;
    
    //Eigenschaften des Feldes: Grundausbildung Automatiker
    strcpy(spielfeld[21].name, "Grundausbildung Automatiker");
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
    spielfeld[21].hausnummer = 11;
    spielfeld[21].anzahlHaeuser = 0;
    spielfeld[21].rgbNummer = 14;
    spielfeld[21].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Chance
    strcpy(spielfeld[22].name, "Chance");
    spielfeld[22].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Produkton Automatiker
    strcpy(spielfeld[23].name, "Produktion Automatiker");
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
    spielfeld[23].hausnummer = 12;
    spielfeld[23].anzahlHaeuser = 0;
    spielfeld[23].rgbNummer = 15;
    spielfeld[23].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Mechatronik Labor
    strcpy(spielfeld[24].name, "Mechatronik Labor");
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
    spielfeld[24].hausnummer = 13;
    spielfeld[24].anzahlHaeuser = 0;
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
    spielfeld[25].rgbNummer = 17;
    spielfeld[25].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Lager Polymechaniker
    strcpy(spielfeld[26].name, "Lager Polymechaniker");
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
    spielfeld[26].hausnummer = 14;
    spielfeld[26].anzahlHaeuser = 0;
    spielfeld[26].rgbNummer = 18;
    spielfeld[26].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Lager Automatiker
    strcpy(spielfeld[27].name, "Lager Automatiker");
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
    spielfeld[27].hausnummer = 15;
    spielfeld[27].anzahlHaeuser = 0;
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
    strcpy(spielfeld[29].name, "Lager Elektroniker");
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
    spielfeld[29].hausnummer = 16;
    spielfeld[29].anzahlHaeuser = 0;
    spielfeld[29].rgbNummer = 21;
    spielfeld[29].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Geh ins Gefängnis
    strcpy(spielfeld[30].name, "Geh ins Gefaengnis");
    spielfeld[30].typ = GEH_INS_GEFAENGNIS;
    
    //Eigenschaften des Feldes: Grundausbildung Elektroniker
    strcpy(spielfeld[31].name, "Grundausbildung Elektroniker");
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
    spielfeld[31].hausnummer = 17;
    spielfeld[31].anzahlHaeuser = 0;
    spielfeld[31].rgbNummer = 22;
    spielfeld[31].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Produktion Elektroniker
    strcpy(spielfeld[32].name, "Produktion Elektroniker");
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
    spielfeld[32].hausnummer = 18;
    spielfeld[32].anzahlHaeuser = 0;
    spielfeld[32].rgbNummer = 23;
    spielfeld[32].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Kanzlei
    strcpy(spielfeld[33].name, "Kanzlei");
    spielfeld[33].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Entwicklung Elektroniker
    strcpy(spielfeld[34].name, "Entwicklung Elektroniker");
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
    spielfeld[34].hausnummer = 19;
    spielfeld[34].anzahlHaeuser = 0;
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
    spielfeld[35].rgbNummer = 25;
    spielfeld[35].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Chance
    strcpy(spielfeld[36].name, "Chance");
    spielfeld[36].typ = EREIGNISFELD;
    
    //Eigenschaften des Feldes: Grundausbildung Polymechaniker
    strcpy(spielfeld[37].name, "Grundausbildung Polymechaniker");
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
    spielfeld[37].hausnummer = 20;
    spielfeld[37].anzahlHaeuser = 0;
    spielfeld[37].rgbNummer = 26;
    spielfeld[37].feldBelastet = 0;  //wenn das Feld belastet ist = 1
    
    //Eigenschaften des Feldes: Schulmaterialkosten
    strcpy(spielfeld[38].name, "Schulmaterialkosten");
    spielfeld[38].typ = STEUERFELD;
    spielfeld[38].preis = 100;
    
    //Eigenschaften des Feldes: Produktion Polymechaniker
    strcpy(spielfeld[39].name, "Produktion Polimechaniker");
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
    spielfeld[39].hausnummer = 21;
    spielfeld[39].anzahlHaeuser = 0;
    spielfeld[39].rgbNummer = 27;
    spielfeld[39].feldBelastet = 0;  //wenn das Feld belastet ist = 1
}

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
    uint8_t anzahlSpieler = 2;
    uint8_t flagFertigGewuerfelt, flagWuerfel1, flagWuerfel2, letzterWuerfel = 0;
    
    uint8_t flagKaufAbgechlossen, flagVersteigert = 0;
    
    uint8_t updateLCD = 0;
    
    //16-Bit Variabeln
    uint16_t tasteAlt, tasteNeu, positiveFlanke = 0; //Variabeln Flankenerkennung
    uint16_t geldZwischenspeicher[5] = {0};
    uint16_t aktuellesGebot = 0;
    //Eigene Datentypen
    Feld spielfeld[40];
    FeldTyp aktuellesFeld = FREIPARKEN;
    zustand_t zustand = SPIELERAUSWAHL;
    
    
    /*--- Prototypen modullokaler Funktionen ------------------------------------*/
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
        
        //verarbeitung verschiedener zustände
        switch (zustand)
        {
            case SPIELERAUSWAHL:
            if (!spielerSetup)
            {
                //LCD ausgabe
                writeText(0,0,"Hallo" UMLAUT_U "Test");
                writeText(1,1,"- 2 Spieler  +");
                displayCharacterAt(1,0,ARROW_L);
                displayCharacterAt(1,15,ARROW_R);
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
                    spielerInfo[i].geld = 0;
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
            case WUERFELSTART:
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
                writeText(1,0,"    wšrfelt    ");
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
            writeText(1,0," wšrfeln A / B ");
            writeText(2,0,"    weiter C    ");
            
            zustand = SPIEL;
            break;
            case SPIEL:
            //nach einer Versteigerung info an lcd anzeigen
            if (flagVersteigert)
            {
                clear();//lcd leeren
                //spieler am zug anzeigen
                writeText(0,0,"   Spieler      "); 
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0," wšrfeln A / B ");
                writeText(2,0,"    weiter C    ");
                flagVersteigert = 0;
            }
            //Spielzug abschliessen
            if((positiveFlanke & TASTE_C) && flagFertigGewuerfelt)
            {
                //würfel Siebensegmente ausschalten
                wuerfelTransmit(SIEBENSEGMENT_OFF,SIEBENSEGMENT_OFF);
                //nächster spieler
                spielerAmZug = (spielerAmZug % anzahlSpieler) + 1;
                //spieler am zug anzeigen
                writeText(0,0,"   Spieler      ");
                sprintf(lcdBuffer,"%u",spielerAmZug);
                writeText(0,11,lcdBuffer);
                writeText(1,0," wšrfeln A / B ");
                writeText(2,0,"    weiter C    ");
                //flags zurücksetzten
                flagFertigGewuerfelt = 0;
                flagKaufAbgechlossen = 0;
                //Schaltet das Blaulicht aus
                PORTC &= ~0xC0;
            }
            //ermöglicht es dem spieler bei Pasch zu kaufen
            if ((positiveFlanke & TASTE_C) && !flagWeiter)
            {
                //Würfel Siebensegmente ausschalten
                wuerfelTransmit(SIEBENSEGMENT_OFF,SIEBENSEGMENT_OFF);
                flagWeiter = 1; //Flag setzen
            }
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
                        case 3:
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
                        flagWeiter = 1;
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
            flagWuerfel1 = 0;
            flagWuerfel2 = 0;
            //kontostand aktualisieren
            updateKontostand(anzahlSpieler,spielerInfo);
           
            
            switch (aktuellesFeld) //verarbeitung aktuelles feld
            {
                case EREIGNISFELD:
                break;
                case STRASSE:
                //Feld kann gekauft werden wenn es niemandem gehört
                if (!flagKaufAbgechlossen && (spielfeld[aktuellePosition].besitzer == 0))
                {
                    if (!updateLCD) //LCD 1 mal aktualisieren
                    {
                        clear();
                        _delay_ms(100);
                        writeText(0,0,"   Spieler      ");
                        sprintf(lcdBuffer,"%u",spielerAmZug);
                        writeText(0,11,lcdBuffer);
                        writeText(1,0,spielfeld[aktuellePosition].name);
                        writeText(2,0,"kaufen? <=N >=J");
                        updateLCD = 1;
                    }
                    if (positiveFlanke & TASTE_R)//kaufen
                    {
                        //wenn das feld noch nicht verkauft ist und der Spieler genug geld hat, kann er es kaufen
                        if((spielerInfo[spielerAmZug].geld >= (spielfeld[aktuellePosition].preis)) && (spielfeld[aktuellePosition].besitzer == 0))
                        {
                            //zieht den betrag vom Konto des spielers ab
                            spielerInfo[spielerAmZug].geld = spielerInfo[spielerAmZug].geld - spielfeld[aktuellePosition].preis;
                            //besitz wird umgeschrieben
                            spielfeld[aktuellePosition].besitzer = spielerAmZug;
                            //Besitz RGB setzen
                            setPropertyRgb(spielfeld[aktuellePosition].rgbNummer,spielerAmZug);
                            //konto aktualisieren
                            updateKontostand(anzahlSpieler,spielerInfo);
                            //flag setzen
                            flagKaufAbgechlossen = 1;
                            //LCD leeren und neu beschreiben
                            clear();
                            _delay_ms(1);
                            writeText(0,0,"   Spieler      ");
                            sprintf(lcdBuffer,"%u",spielerAmZug);
                            writeText(0,11,lcdBuffer);
                            writeText(1,0," wšrfeln A / B ");
                            writeText(2,0,"    weiter C    ");
                            updateLCD = 0;
                        }
                    }
                    else if (positiveFlanke & TASTE_L)//nicht Kaufen
                    {
                        zustand = VERSTEIGERUNG; //bei nicht kauf wird versteigert
                        //flags setzen
                        updateLCD = 0;
                        flagKaufAbgechlossen = 1;
                    }
                }
                break;
                case STEUERFELD:
                break;
                case HALTESTELLE:
                if (positiveFlanke & TASTE_L)
                {
                    if((spielerInfo[spielerAmZug].geld >= (spielfeld[aktuellePosition].preis)) && (spielfeld[aktuellePosition].besitzer == 0))
                    {
                        spielerInfo[spielerAmZug].geld = spielerInfo[spielerAmZug].geld - spielfeld[aktuellePosition].preis;
                        spielfeld[aktuellePosition].besitzer = spielerAmZug;
                        setPropertyRgb(spielfeld[aktuellePosition].rgbNummer,spielerAmZug);
                        //setGeld(spielerInfo[spielerAmZug].geld,spielerAmZug,1);
                    }
                }
                break;
                case GEFAENGNIS:
                break;
                case GEH_INS_GEFAENGNIS:
                abInsGefaengnis(spielerAmZug);
                aktuellesFeld = GEFAENGNIS;
                break;
                case FREIPARKEN:
                break;
                case WERK:
                if (positiveFlanke & TASTE_L)
                {
                    if((spielerInfo[spielerAmZug].geld >= (spielfeld[aktuellePosition].preis)) && (spielfeld[aktuellePosition].besitzer == 0))
                    {
                        spielerInfo[spielerAmZug].geld = spielerInfo[spielerAmZug].geld - spielfeld[aktuellePosition].preis;
                        spielfeld[aktuellePosition].besitzer = spielerAmZug;
                        setPropertyRgb(spielfeld[aktuellePosition].rgbNummer,spielerAmZug);
                        //setGeld(spielerInfo[spielerAmZug].geld,spielerAmZug,1);
                    }
                }
                break;
            }
            break;
            case VERSTEIGERUNG:
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
            default:
            break;
        }
    }
}

