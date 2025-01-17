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


#ifndef MONOPOLYTREIBER_H_
#define MONOPOLYTREIBER_H_

#include <avr/io.h>
#define F_CPU 16000000UL
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>





extern uint8_t houses[14][8];           //Globales Array zur Ausgabe der Immobilien
extern uint8_t hausRegister[14];
extern uint8_t spieler[20][8];    //Globales Array zur Ausgabe der Spieler Position
extern uint8_t spielerPos[4];     //Globales Array zur SpielerInformationen
extern uint8_t siebensegment[16];
extern uint8_t wuerfelArray[2];

void resetMonopoly(void);

void writeHouse(uint8_t data[14]);
void setHouse(uint8_t FeldNr, uint8_t anzahlHaus);

void setPropertyRgb(uint8_t FeldNummer, uint8_t spielerNr);

void setPlayerPosition(uint8_t feld, uint8_t spielerNummer);
int8_t spielerPosFehlerAusgleich(uint8_t spielerNummer);

void setGeld(uint16_t geld, uint8_t spieler);

uint8_t wuerfel(void);
uint8_t sibensegmentWuerfel(void);


void adm_ADC_init(void);
uint16_t adm_ADC_read(uint8_t kanal);


#endif /* MONOPOLYTREIBER_H_ */