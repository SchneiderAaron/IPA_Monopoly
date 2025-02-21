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
* Dateiname: MonopolyTreiber.h
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


#ifndef MONOPOLYTREIBER_H_  /* Abfrage, ob der Name schon definiert wurde */
#define MONOPOLYTREIBER_H_  /* Definiere den Namen und führe alles weitere aus */
/*--- #includes der Form <...> -----------------------------------------------*/
#include <avr/io.h>
#define F_CPU 16000000UL
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#include <avr/pgmspace.h>  // Benötigt für PROGMEM und pgm_read Funktionen
/*--- #includes der Form "..." -----------------------------------------------*/
/*--- #define-Konstanten und Makros ------------------------------------------*/
#define UE "š"
#define AE "„"
#define OE "”"

#define CHANCE1 7
#define CHANCE2 22
#define CHANCE3 36
/*--- Datentypen (typedef) ---------------------------------------------------*/
typedef struct {
    char name[50];      // Name des Spielers
    uint16_t geld;      //Kontostand des Spielers
    uint8_t position;   //Position des spielers
    uint8_t gefaengnis;
    uint8_t rundenImGefaengnis;
    uint8_t freikarte;
    uint8_t haeuser;
    uint8_t hotels;
    uint8_t pleite;
} Spieler;

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
    uint8_t farbgruppenFelder[4]; //FeldNummern der Farbgruppen hinterlegt
    uint8_t hausnummer;
    uint8_t anzahlHaeuser;
    uint8_t rgbNummer;
    uint8_t kostenHaus;
    uint8_t feldBelastet;
    uint8_t hypothek;
    uint8_t hypothekAufloesen;
} Feld;
typedef enum
{
    WORKSHOP,//gehe direkt in den workshop
    FREIKARTE,//workshop freikarte
    RENOVIEREN,//für jedes haus un jedes hotel bezahlen
    GELD_AN_BANK,//geld erhalten/bezahlen -> Bank
    GELD_VON_BANK,//geld erhalten/bezahlen -> Bank
    GELD_AN_MITSPIELER,//geld erhalten/bezahlen -> mitspieler
    GELD_VON_MITSPIELER,//geld erhalten/bezahlen -> mitspieler
    BEWEGEN,//anzahl velder vor oder zurück
    TELEPORTIEREN//auf zielfeld vorrücken
} KartenTyp;

typedef struct {
    KartenTyp typ;      // Typ der Karte
    int8_t bewegung;    // Anzahl der Felder (positiv = vorwärts, negativ = rückwärts)
    int16_t geld;       // Geldbetrag (+ für Gewinn, - für Strafe)
    int16_t geld2;      //2. geldbetrag
    uint8_t zielFeld;   // Falls die Karte den Spieler auf ein bestimmtes Feld schickt
    FeldTyp zielFeldTyp;
} Karte;


typedef struct  
{
    uint8_t spielerNr;//in haendler werden die Händler gespeichert
    uint8_t feldNummern[28];//in feldNummern werden die gehandelten felder gespeichert
    uint16_t barGeld;//in barGeld wird das gehandelte Geld gespeichert
    uint8_t freikarte;//in freikarte wird gespeichert ob eine Freikarte gehandelt wird
}handelInventar;


//typedef enum geldBeschaffen{HAEUSER, FELDER, PLEITE, MITSPIELER, BANK}pleite_t;
/*--- Globale Konstanten (extern) --------------------------------------------*/
/*--- Globale Variablen (extern) ---------------------------------------------*/
extern uint8_t houses[14][8];           //Globales Array zur Ausgabe der Immobilien
extern uint8_t hausRegister[14];
extern uint8_t spieler[20][8];    //Globales Array zur Ausgabe der Spieler Position
extern uint8_t spielerPos[4];     //Globales Array zur SpielerInformationen
extern uint8_t siebensegment[16];
extern uint8_t wuerfelArray[2];
extern Spieler spielerInfo[5];
extern uint8_t anzahlSpieler;
extern uint8_t spielerImGefaengnis[5];

extern Feld spielfeld[40];
extern uint16_t tasteAlt, tasteNeu, positiveFlanke; //Variabeln Flankenerkennung
extern uint8_t flagGeldBeschaffen;

extern uint8_t xTasten[4];
extern uint8_t yTasten[4];
/*--- Prototypen globaler Funktionen -----------------------------------------*/

void resetMonopoly(void);

void writeHaus(uint8_t data[14]);
void setHaus(uint8_t FeldNr, uint8_t anzahlHaus);

void setPropertyRgb(uint8_t FeldNummer, uint8_t spielerNr);

void setPlayerPosition(uint8_t feld, uint8_t spielerNummer);
int8_t spielerPosFehlerAusgleich(uint8_t spielerNummer);

void setGeld(uint16_t geld, uint8_t spieler, uint8_t siebensegmentOnOff);
void updateKontostand(uint8_t anzahlSpieler, Spieler spielerInfo[5]);

uint8_t zufallsGenerator(void);
void wuerfel(void);
void wuerfelAB(uint8_t wuerfelNummer, uint8_t flagWuerfel1, uint8_t flagWuerfel2);
void wuerfelTransmit(uint8_t zahl1, uint8_t zahl2);

void adm_ADC_init(void);
uint16_t adm_ADC_read(uint8_t kanal);

void abInsGefaengnis(uint8_t Spieler);
void blaulicht(uint8_t delay, uint8_t anzahlWiederholungen);

void PortInitialisierung(void);

void startGeldAnimation(uint8_t anzahlSpieler);

void initialisiereSpielfeld(Feld spielfeld[]);
void initialisiereKarten(Karte chanceKanzlei[]);
void read_string(char *buf, size_t i);
uint8_t ereignisFeld(uint8_t kanzlei, uint8_t spielerAmZug, uint8_t schritt, uint8_t flagWeiter, Karte chanceKanzlei[]);
uint8_t geldUeberweisen(uint8_t zahler, uint8_t empfaenger, uint16_t betrag, uint8_t schritt);
void initialisiereHandelInventar(handelInventar handel[]);
uint8_t geldBeschaffen(uint8_t spielerNr, uint8_t schuldenBei, uint16_t mindestBetrag);
void hausBauen(uint8_t spielerAmZug);
#endif /* MONOPOLYTREIBER_H_ */