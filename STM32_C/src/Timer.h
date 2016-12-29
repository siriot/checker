/*
 * Timer.h
 *
 *  Created on: 2016. ápr. 15.
 *      Author: Tibi
 */

#ifndef TIMER_H_
#define TIMER_H_

#define BACKLIGHT 60 //(0-99)

volatile int ButtonEvent;
volatile int NewEvent;
volatile int ToggleSignal;

void InitTimers();
void StartBlinkTimer();
void StopBlinkTimer();


#endif /* TIMER_H_ */
