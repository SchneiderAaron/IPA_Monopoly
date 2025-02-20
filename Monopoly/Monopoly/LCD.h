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
* Dateiname: LCD.h
*
* Projekt  : IPA_Monopoly
* Hardware : Monopoly-Board, ATmega2560v von Atmel
*
*
* Copyright: MSW, AE3
*
* Beschreibung:
* =============
* Treiber zur ansteuerung des LCDs auf der Monopoly Hardware
* um den buchstben ü zu schreiben muss das zeichen š verwendet werden
* Verlauf:
* ========
* Datum:      Autor:         Version   Grund der Änderung:
* 10.01.2025  A.Schneider    V1.0      Neuerstellung
*
\*********************************************************************************/
/*
„ = ä
Ž = Ä
” = ö
™ = Ö
š = ü
*/
//LCD Pfeile
#define PFEIL_R "\x7E"  // ASCII 126 (Pfeil nach rechts)
#define PFEIL_L "\x08"  // ASCII 8 (Pfeil nach links)
#define PFEIL_O "\x01"  // Custom Character 1 (Pfeil nach oben)
#define PFEIL_U "\x02"  // Custom Character 2 (Pfeil nach unten)
#include <string.h>

#ifndef LCD_H_
#define LCD_H_

void initDisplay(void);
void CmdDisplay(uint8_t Cmd);
void DataDisplay(uint8_t Data);
void clear(void);
void home(void);
void displayOnOff(uint8_t DisplayOn,uint8_t CursorOn, uint8_t BlinkOn);
void shift(void);
void writeText(uint8_t Zeile, uint8_t Spalte, const char *Text);
void displayCharacterAt(uint8_t zeile, uint8_t spalte, uint8_t charAddress);
void defineCustomCharacters(void);
void lcdInitAll(void);
uint8_t lcdLauftext(const char *text, uint8_t schritt);
void lcdReInit(void);

#endif /* LCD_H_ */