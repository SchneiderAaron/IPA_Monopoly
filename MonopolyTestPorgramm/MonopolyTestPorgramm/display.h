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
* Dateiname: display.h
*
* Projekt  : Monopoly IPA
* Hardware : Monopoly Board
*
* Copyright: MSW, E4
*
* Beschreibung:
* =============
* Diese Datei beinhaltet die Display Klasse welche für die ansteuerung vom Display 
* zuständig ist. Es gibt die möglichkeit via einem "define" aus zu wählen ob 
* der code via reiner OOP implementation laufen soll (kaputt) oder via einem 
* "wrapper" der C funktionen auf ruft.
* 
*
* Verlauf:
* ========
* Datum:      Autor:         Version   Grund der Änderung:
* 22.12.2022  R. Birch       V0.1      Neuerstellung
* 01.03.2023  R. Birch       V1.0      anpassung C Rechtlinien
*\*********************************************************************************/

/*--- #define-Konstanten und Makros -----------------------------------------*/
#ifndef DISPLAY_H_
#define DISPLAY_H_

//#define OBJECTIVE

#include <avr/io.h>
#include "../../../include.h"

/*--- Datentypen (typedef) --------------------------------------------------*/
#ifdef OBJECTIVE

class Display{
public:
    Display(volatile uint8_t* port_data = &PORTD, volatile uint8_t* port_controll = &PORTC){
        _data = port_data;
        _control = port_controll;
        
        //PORTD - 0x1 -> DDRD
        ddr_data = (port_data-0x1);
        ddr_control = (port_controll-0x1);

        initDisplay();
    }
public:
    /*********************************************************************************\
    * CmdDisplay
    *
    * Gibt Befehle an den Display
    *
    * Parameter:
    * Cmd = Welcher Befehl soll geschickt werden
    *
    \*********************************************************************************/
    void cmdDisplay(uint8_t cmd){
        rs_false();
        rw_false();
        (*_data) = cmd;
        if ((cmd==1)||(cmd==2)||(cmd==3))
        {
            _delay_us(1100);
        }else{
            _delay_us(200);
        }
        disable();
    }

    /*********************************************************************************\
    * DataDisplay
    *
    * Schickt Daten an den Display
    *
    * Parameter:
    * Data = Welche Daten sollen übertragen werden
    *
    \*********************************************************************************/
    void DataDisplay(uint8_t Data)
    {
        rs_true();
        _delay_us(100);
        rw_false();
        _delay_us(100);
        (*_data) = Data;
        _delay_us(100);
        enable();
        _delay_us(100);
        disable();
    }

    /*********************************************************************************\
    * initCustomChar
    *
    * Initialisiert individuelle Zeichen für den Display die nicht im ASCII satz sind
    *
    \*********************************************************************************/
    void initCustomChar(void)
    {
        uint8_t customChar[8][8] = {{0b1, 0b11, 0b11, 0b111, 0b111, 0b1111, 0b11111, 0b11111},
        {31, 31, 31, 31, 31, 31, 31, 31},
        {0b1, 0b11, 0b11, 0b111, 0b111, 0b1111, 0b11111, 0b11111},
        {31, 31, 31, 31, 31, 31, 31, 31},
        {31, 31, 31, 31, 31, 31, 31, 31},
        {31, 31, 31, 31, 31, 31, 31, 31},
        {31, 31, 31, 31, 31, 31, 31, 31},
        {31, 31, 31, 31, 31, 31, 31, 31},};
        for (uint8_t Char = 0; Char < 8; Char++)
        {
            for (uint8_t row = 0; row < 8; row ++)
            {
                cmdDisplay(64 + (Char << 3) + row);
                DataDisplay(customChar[Char][row]);
                _delay_us(200);
            }
        }
    }

    /*********************************************************************************\
    * initDisplay
    *
    * Initialisiert das Display damit Anzeige funktioniert und der Kontrast stimmt
    *
    \*********************************************************************************/
    void initDisplay(void)
    {
        init_port_data();
        init_port_controll();
        psb_true();
        reset_off();
        chip_select();

        _delay_ms(50);
        uint8_t initCMD[9] = {0x39, 0x1D, 0x50, 0x6C, 0x75, 0x38, 0x0C, 0x01,0x06};
        for (uint8_t Command = 0; Command < 9; Command++)
        {
            cmdDisplay(initCMD[Command]);
            _delay_us(100);
        }
        initCustomChar();
    }

    /*********************************************************************************\
    * writeText
    *
    * Zeigt den entsprechenden Text auf dem Display an. Buchstaben müssen im ASCII
    * satz vorhanden sein
    *
    * Parameter:
    * Zeile = Auf Welcher Zeile soll angefangen werden zu schreiben
    * Spalte = Auf Welcher Spalte soll angefangen werden zu schreiben
    * Text = Welcher Text soll übertragen werden
    *
    \*********************************************************************************/
    void writeText(uint8_t Zeile, uint8_t Spalte, char* Text)
    {
        if (Zeile == 2)
        {
            cmdDisplay(128 + 0x20 + Spalte);
        }
        else if (Zeile == 1)
        {
            cmdDisplay(128 + 0x10 + Spalte);
        }
        else
        {
            cmdDisplay(128 + Spalte);
        }
        
        uint16_t i = 0;
        while(Text[i])
        {
            DataDisplay(Text[i]);
            i++;
            _delay_us(500);
        }
    }

    /*********************************************************************************\
    * WriteChar
    *
    * Schreibt ein spezielles Zeichen auserhalb des ASCII Satzes
    *
    * Parameter:
    * Zeile = Auf Welcher Zeile soll angefangen werden zu schreiben
    * Spalte = Auf Welcher Spalte soll angefangen werden zu schreiben
    * Zeichen = Welches Zeichen soll geschrieben werden
    *
    \*********************************************************************************/
    void WriteChar(uint8_t Zeile, uint8_t Spalte, uint8_t Zeichen)
    {
        if (Zeile)
        {
            cmdDisplay(128 + 0x40 + Spalte);
        }
        else
        {
            cmdDisplay(128 + Spalte);
        }
        DataDisplay(Zeichen);
    }

    /*********************************************************************************\
    * WriteNumber
    *
    * Schreibt ein Zahl
    *
    * Parameter:
    * Zeile = Auf Welcher Zeile soll angefangen werden zu schreiben
    * Spalte = Auf Welcher Spalte soll angefangen werden zu schreiben
    * Zahl = Welche Zahl soll geschrieben werden
    *
    \*********************************************************************************/
    void WriteNumber(uint8_t Zeile, uint8_t Spalte, uint16_t zahl)
    {
        uint8_t e,z,h,t;
        char send_buffer[4];
        uint8_t i;
        //Zerlegt Zahl in einzelne Ziffer und schreibt (+48) zu ascii
        t = (zahl / 1000) + 48;
        h = ((zahl % 1000) / 100) + 48;
        z = ((zahl % 100) / 10) + 48;
        e = (zahl % 10) + 48;
        //Prüft nach vorgestellte nullen der Zahl
        for (i=0; i<=3; i++)
        {
            if (t == 48)
            {
                //Rückt Stellen der Ziffern nach oben
                t = h;
                h = z;
                z = e;
                //Schreibt Leerzeichen bei nichtbentigtem Platz
                e = 0;
            }
            else
            {
                i = 4;
            }
        }
        send_buffer[0] = t;
        send_buffer[1] = h;
        send_buffer[2] = z;
        send_buffer[3] = e;
        writeText(Zeile, Spalte, send_buffer);
    }

    /*********************************************************************************\
    * clearDisplay
    *
    * Löscht das ganze Display
    *
    \*********************************************************************************/
    void clearDisplay(void)
    {
        writeText(0,0,(char*)"                ");
        _delay_us(500);
        writeText(1,0,(char*)"                ");
        _delay_us(500);
        writeText(2,0,(char*)"                ");
    }
private:
    // define ports
    void __attribute__((always_inline)) init_port_data(){
        (*ddr_data) = 0xFF;
    }
    void __attribute__((always_inline)) init_port_controll(){
        (*ddr_control) = 0b11111111;
    }

    // paralell / seriall ansteuerung
    void __attribute__((always_inline)) psb_true(){
        (*_control) |= 0b00001000;
    }
    void __attribute__((always_inline)) psb_false(){
        (*_control) &= 0b11110111;
    }

    //enable / disable
    void __attribute__((always_inline)) enable(){
        (*_control) |= 0b00000010;
    }
    void __attribute__((always_inline)) disable(){
        (*_control) &= 0b11111101;
    }

    //read / !write!
    void __attribute__((always_inline)) rw_true(){
        (*_control) |= 0b00000100;
    }
    void __attribute__((always_inline)) rw_false(){
        (*_control) &= 0b11111011;
    }

    // csb signal
    void __attribute__((always_inline)) chip_deselect(){
        (*_control) |= 0b00000001;
    }
    void __attribute__((always_inline)) chip_select(){
        (*_control) &= 0b11111110;
    }

    // rs signal
    void __attribute__((always_inline)) rs_true(){
        (*_control) |= 0b00010000;
    }
    void __attribute__((always_inline)) rs_false(){
        (*_control) &= 0b11101111;
    }

    // reset whole lcd
    void __attribute__((always_inline)) reset_off(){
        (*_control) |= 0b00100000;
    }
    void __attribute__((always_inline)) reset_on(){
        (*_control) &= 0b11011111;
    }

private:
    volatile uint8_t* _data;
    volatile uint8_t* _control;

    volatile uint8_t* ddr_data;
    volatile uint8_t* ddr_control;
};
#endif


#ifndef OBJECTIVE

class Display{
public:
    /******************************************************************************\
    * <<Display>>
    *
    * Description:
    *	initializes the Display class and the Display
    *
    * Parameter:
    *
    * return value: 
    *	<Display>
    *
    \******************************************************************************/
    Display();
    
    
    /******************************************************************************\
    * <<WriteText>>
    *
    * Description:
    *	writes a given c string to the display. The string starts a the position 
    *   given to it via the row and column variables.
    *
    * Parameter:
    * <uint8_t><<Row>> = <y Position on the display>
    * <uint8_t><<Column>> = <x Position on the display>
    * <char *><<Text>> = <the text to be displayed>
    *
    * return value: 
    *	<void>
    *
    \******************************************************************************/
    void writeText(uint8_t Row, uint8_t Column, char* Text);

    /******************************************************************************\
    * <<WriteText>>
    *
    * Description:
    *	writes a given const c string to the display. The string starts a the position
    *   given to it via the row and column variables.
    *
    * Parameter:
    * <uint8_t><<Row>> = <y Position on the display>
    * <uint8_t><<Column>> = <x Position on the display>
    * <char *><<Text>> = <the text to be displayed>
    *
    * return value: 
    *	<void>
    *
    \******************************************************************************/
    void writeText(uint8_t Row, uint8_t Column, const char* Text);

    /******************************************************************************\
    * <<WriteChar>>
    *
    * Description:
    *	writes a given char to the display at the positions given to it via
    *   the variables Row and Column.
    *
    * Parameter:
    * <uint8_t><<Row>> = <y Position on the display>
    * <uint8_t><<Column>> = <x Position on the display>
    * <uint8_t><<Character>> = <the character to be displayed>
    *
    * return value: 
    *	<void>
    *
    \******************************************************************************/
    void WriteChar(uint8_t Row, uint8_t Column, uint8_t Character);

    /******************************************************************************\
    * <<WriteNumber>>
    *
    * Description:
    *	writes a given Number to the display at the position given to it by the 
    *   variables Row and Column.
    *
    * Parameter:
    * <uint8_t><<Row>> = <y Position on the display>
    * <uint8_t><<Column>> = <x Position on the display>
    * <uint16_t><<Number>> = <Number to be displayed>
    *
    * return value: 
    *	<void>
    *
    \******************************************************************************/
    void WriteNumber(uint8_t Row, uint8_t Column, uint16_t Number);

    /******************************************************************************\
    * <<ClearDisplay>>
    *
    * Description:
    *	Cleares the whole display
    *
    * Parameter:
    *
    * return value: 
    *	<void>
    *
    \******************************************************************************/
    void clearDisplay();
};

#endif


#endif /* DISPLAY_H_ */