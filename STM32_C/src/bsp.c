/*
 * bsp.c
 */

#include "bsp.h"

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_cortex.h"

/*
 * Hardver inicializ�l� l�p�sek
 */
void HAL_MspInit(void)
{
	InitPorts();
	InitLcd();
	InitTimers();
	HAL_NVIC_SetPriorityGrouping(5);
}
