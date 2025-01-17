/*
 * LCD.h
 *
 * Created: 16.01.2025 10:12:54
 *  Author: e1Schnei
 */ 


#ifndef LCD_H_
#define LCD_H_

void initDisplay(void);
void CmdDisplay(uint8_t Cmd);
void DataDisplay(uint8_t Data);
void clear(void);
void home(void);
void displayOnOff(uint8_t DisplayOn,uint8_t CursorOn, uint8_t BlinkOn);
void shift(void);
void writeText(uint8_t Zeile, uint8_t Spalte, uint8_t * Text);




#endif /* LCD_H_ */