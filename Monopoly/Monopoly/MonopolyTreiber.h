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
#define RGB_BANK 9
/*--- Datentypen (typedef) ---------------------------------------------------*/
typedef struct {
    char name[50];              // Name des Spielers
    uint16_t geld;              // Kontostand des Spielers
    uint8_t position;           // Position des spielers
    uint8_t gefaengnis;         // 1 = Spieler ist im Gefängnis
    uint8_t rundenImGefaengnis; // Anzahl runden, die im Gefängnis verbracht wurden 
    uint8_t freikarte;          // Anzahl Freikarten, die der Spieler besitzt
    uint8_t haeuser;            // Anzahl Häuser, die der Spieler besitzt
    uint8_t hotels;             // Anzahl Hotels, die der Spieler besitzt
    uint8_t pleite;             // 1 = Spieler ist Pleite und Spielt nicht mehr mit
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
    char name[50];                  // Name des Feldes
    FeldTyp typ;                    // Typ des Feldes
    uint16_t preis;                 // Kaufpreis
    uint16_t mieten[7];             // Mieten auf dem Feld -> Einzeln, 1 Haus, 2 Häuser, 3 Häuser, 4 Häuser, 1 Hotel, Farbgruppe
    uint8_t besitzer;               // Spielernummer des Besitzers (0 = Unbesetzt)
    Farbe farbGruppe;               // Die Farbe der Farbgruppe
    uint8_t farbgruppenFelder[4];   // FeldNummern aller Felder der Farbgruppe
    uint8_t hausnummer;             // Hausnummer (Wird verwendet um Häuser LEDs anzusteuern)
    uint8_t anzahlHaeuser;          // Anzahl Häuser auf dem Feld -> 0 = Keine Häuser, 1-4 = 1-4 Häuser, 5 = 1 Hotel
    uint8_t rgbNummer;              // Wird verwendet um die RGB Led des Feldes zu setzen
    uint8_t kostenHaus;             // Preis für ein Haus
    uint8_t feldBelastet;           // Ist das Feld belastet? -> 0 = unbelastet, 1 = Belastet
    //uint8_t hypothek;
    //uint8_t hypothekAufloesen;
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

//in der Vorbereitung erstellt
void writeHaus(uint8_t data[14]);
//in der Vorbereitung erstellt
void setHaus(uint8_t FeldNr, uint8_t anzahlHaus);
//in der Vorbereitung erstellt
void setPropertyRgb(uint8_t FeldNummer, uint8_t spielerNr);
//in der Vorbereitung erstellt
void setzeSpielerPosition(uint8_t feld, uint8_t spielerNummer);
//in der Vorbereitung erstellt
int8_t spielerPosFehlerAusgleich(uint8_t spielerNummer);
//in der Vorbereitung erstellt
void setGeld(uint16_t geld, uint8_t spieler, uint8_t siebensegmentOnOff);

void updateKontostand(uint8_t anzahlSpieler, Spieler spielerInfo[5]);

uint8_t zufallsGenerator(void);
//in der Vorbereitung erstellt
void wuerfel(void);
//in der Vorbereitung erstellt
void wuerfelAB(uint8_t wuerfelNummer, uint8_t flagWuerfel1, uint8_t flagWuerfel2);
//in der Vorbereitung erstellt
void wuerfelTransmit(uint8_t zahl1, uint8_t zahl2);
//in der Vorbereitung erstellt
void adm_ADC_init(void);
//in der Vorbereitung erstellt
uint16_t adm_ADC_read(uint8_t kanal);

void abInsGefaengnis(uint8_t Spieler);
void blaulicht(uint8_t delay, uint8_t anzahlWiederholungen);

//in der Vorbereitung erstellt
void PortInitialisierung(void);
//in der Vorbereitung erstellt
void startGeldAnimation(uint8_t anzahlSpieler);

void initialisiereSpielfeld(Feld spielfeld[]);
void initialisiereKarten(Karte chanceKanzlei[]);
void read_string(char *buf, size_t i);
uint8_t ereignisFeld(uint8_t kanzlei, uint8_t spielerAmZug, uint8_t schritt, uint8_t flagWeiter, Karte chanceKanzlei[]);
uint8_t ueberweisungsSchritt(uint16_t betrag);
uint8_t geldUeberweisen(uint8_t zahler, uint8_t empfaenger, uint16_t betrag);
void initialisiereHandelInventar(handelInventar handel[]);
uint8_t geldBeschaffen(uint8_t spielerNr, uint8_t schuldenBei, uint16_t mindestBetrag);
void hausKaufenVerkaufen(uint8_t spielerAmZug);
#endif /* MONOPOLYTREIBER_H_ */