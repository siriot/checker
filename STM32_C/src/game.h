/*
 * game.h
 *
 *  Created on: 2016. �pr. 15.
 *      Author: Tibi
 */

#ifndef GAME_H_
#define GAME_H_

#include "bsp.h"

typedef void(*EventHandlerType)(void);
typedef enum {Search, Carry, End} GameStateType;
EventHandlerType EventHandler[3][5];
EventHandlerType ReverseEventHandler[3][5];
GameStateType GameState;

FigureType Table[8][8];
uint8_t CursorX;
uint8_t CursorY;
FigureType CarriedFigure;

uint8_t Points[2];

void SetFigure(uint8_t X, uint8_t Y, FigureType figure);

void SelectFigure();
void PlaceFigure();
void RestartGame();

#define NEGYZET_PLAYER 1
#define KOR_PLAYER 0
uint8_t ActualPlayer;
#define Reverse !ActualPlayer


#endif /* GAME_H_ */
