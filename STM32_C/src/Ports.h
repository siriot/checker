/*
 * Ports.h
 *
 *  Created on: 2016. ápr. 14.
 *      Author: Tibi
 */

#ifndef PORTS_H_
#define PORTS_H_

#include "bsp.h"

#define RW GPIO_PIN_7
#define RWPort GPIOE

#define DPort GPIOE
#define DB0 GPIO_PIN_8
#define DB1 GPIO_PIN_9
#define DB2 GPIO_PIN_10
#define DB3 GPIO_PIN_11
#define DB4 GPIO_PIN_12
#define DB5 GPIO_PIN_13
#define DB6 GPIO_PIN_14
#define DB7 GPIO_PIN_15
#define DBall ((uint16_t)0xFF00)

#define BTNPort GPIOD
#define BTN0 GPIO_PIN_11
#define BTN1 GPIO_PIN_12
#define BTN2 GPIO_PIN_13
#define BTN3 GPIO_PIN_14
#define BTN4 GPIO_PIN_15

#define ENPort GPIOB
#define EN GPIO_PIN_7
#define CSPort GPIOB
#define CS1 GPIO_PIN_4
#define CS2 GPIO_PIN_5
#define EPort GPIOD
#define E GPIO_PIN_7
#define DIPort GPIOD
#define DI GPIO_PIN_6
#define RSTPort GPIOD
#define RST GPIO_PIN_3

#define PWMPort GPIOC
#define PWM GPIO_PIN_8

void InitPorts();
void WriteData(uint16_t data);
void SetDataDirOut();
void SetDataDirIn();
void SetE();
void ClearE();
void EPulse();
void SetDataReg();
void SetInstructionReg();
void LcdReset(int state);
void SelectChip1();
void SelectChip2();
void WaitBusy();


#endif /* PORTS_H_ */
