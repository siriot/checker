/*
 * Lcd.h
 *
 *  Created on: 2016. �pr. 14.
 *      Author: Tibi
 */

#ifndef LCD_H_
#define LCD_H_

#include "bsp.h"

// LCD utas�t�sk�dok �s c�mek
#define DISPLAY_ON  0b00111111
#define DISPLAY_OFF 0b00111110
#define STARTLINE 0b11000000
#define PAGE_ADDRESS   0b10111000
#define COLUMN_ADDRESS 0b01000000
#define BUSY_MASK 		0b10000000
#define ON_OFF_MASK 	0b00100000
#define RESET_MASK		0b00010000
#define DISPLAY_START_LINE 0b11000000

#define IS_COORD_VALID(x,y) ((x<8) && (y<8))

//#define TOGGLE_FULL_SCREEN // define-olt esetben nem csak a b�buk, hanem a teljes tartalom villog
typedef enum {KOR,KOR_KIRALY,NEGYZET,NEGYZET_KIRALY,URES, KIINDULASI_PONT} FigureType;

// A k�t oldals�vban megjelen� adatok helyzete. A sorokat 0-t�l kezdve fel�lr�l sz�mozzuk.
#define PLAYER_SIGNAL_LINE 1
#define MESSAGE1_LINE 3
#define MESSAGE2_LINE 4
#define POINT_LINE 6
char *Text1;
char *Text2;

void InitLcd();
void TogglePieces();
void SetupScreen();
#define PATTERN_ON 1
#define PATTERN_OFF 0
#define SCROLL 1
#define NO_SCROLL 0
void DrawTable(uint8_t PatternOn, uint8_t Scroll);

void ShowPlayerSignal(uint8_t player);
#define SIDE_BOTH 3
#define SIDE_LEFT 1
#define SIDE_RIGHT 2
void ShowMessage(uint8_t Line, uint8_t Offset,char *Text,uint8_t Length, uint8_t Side);
#define NO_INVERT 0x00
#define INVERT 0xFF
void DrawMaze(uint8_t x, uint8_t y,FigureType figure, uint8_t Mask);
void MoveCursor(uint8_t x, uint8_t y);
void MoveCursorRight();
void MoveCursorLeft();
void MoveCursorUp();
void MoveCursorDown();
void CarryFigureRight();
void CarryFigureLeft();
void CarryFigureUp();
void CarryFigureDown();

#endif /* LCD_H_ */
