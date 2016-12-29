#include "bsp.h"

/*
 * GPIO-k inicializ�l�sa, az egyes adatvonalak k�zvetlen, alacsony szint� be�ll�t�sa, hardware elfed�se.
 */

void InitPorts()
{
	GPIO_InitTypeDef gpio;

	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();

	//adat bitek, input
	gpio.Mode = GPIO_MODE_INPUT; // az adatbitek az �sszehajt�s elker�l�se v�gett kezdetben inputok
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Pin = DBall;
	HAL_GPIO_Init(GPIOD,&gpio);
	// vez�rl� bitek
	gpio.Mode = GPIO_MODE_OUTPUT_PP;

	gpio.Pin = E | DI | RST;
	HAL_GPIO_Init(GPIOD,&gpio);
	HAL_GPIO_WritePin(RSTPort,RST,GPIO_PIN_RESET); //Az LCD-t resetben tartjuk kezdetben.
	HAL_GPIO_WritePin(EPort,E,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DIPort,DI,GPIO_PIN_RESET);

	gpio.Pin = RW;
	HAL_GPIO_Init(GPIOE,&gpio);
	HAL_GPIO_WritePin(RWPort, RW, GPIO_PIN_RESET); // Alap�rtelmez�sben �runk.

	gpio.Pin = EN | CS1 | CS2;
	HAL_GPIO_Init(GPIOB,&gpio);
	HAL_GPIO_WritePin(ENPort,EN,GPIO_PIN_RESET); // Enged�lyezz�k a meghajt�t.
	HAL_GPIO_WritePin(CSPort,CS1 | CS2, GPIO_PIN_SET);// T�r�lj�k a chip kiv�laszt�sa.

	// gombok
	gpio.Mode=GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLUP;
	gpio.Pin = BTN0 | BTN1 | BTN2 | BTN3 | BTN4;
	HAL_GPIO_Init(GPIOD,&gpio);

	//PWM l�b inicializ�l�sa, C port 8-as pin
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Alternate = GPIO_AF3_TIM8;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Pull = GPIO_NOPULL;
	gpio.Pin = GPIO_PIN_8;
	HAL_GPIO_Init(GPIOC,&gpio);

	HAL_GPIO_WritePin(RSTPort,RST,GPIO_PIN_SET); //Feloldjuk az LCD reset-j�t.
}

void SelectChip1() // Az LCD-ben tal�lhat� 1. vez�rl� chip kiv�laszt�sa.
{
	HAL_GPIO_WritePin(CSPort,CS1,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CSPort,CS2,GPIO_PIN_SET);
}

void SelectChip2()// Az LCD-ben tal�lhat� 1. vez�rl� chip kiv�laszt�sa.
{
	HAL_GPIO_WritePin(CSPort,CS2,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CSPort,CS1,GPIO_PIN_SET);
}


void WriteData(uint16_t data)
{
	SetDataDirOut(); // Kifel� �ll�tjuk az adatvonalakat.
	WaitBusy(); // Megv�rjuk, m�g mindk�t chip k�szen van.
	data &= 0x00FF;
	DPort->ODR &= 0x00FF;
	DPort->ODR |= data<<8; //Adatok ki�r�sa
	EPulse();
}
void SetDataDirOut() // A GPIO �jrainicializ�l�s�val outputt� ford�tjuk az adatvonalakat.
{
	GPIO_InitTypeDef gpio;

	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Pin = DBall;

	HAL_GPIO_WritePin(RWPort,RW,GPIO_PIN_RESET);
	HAL_GPIO_Init(DPort,&gpio);
}

void SetDataDirIn() // A GPIO �jrainicializ�l�s�val inputt� ford�tjuk az adatvonalakat.
{
	GPIO_InitTypeDef gpio;

	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Pin = DBall;
	HAL_GPIO_Init(DPort,&gpio);

	HAL_GPIO_WritePin(RWPort,RW,GPIO_PIN_SET);
}

#define ELOW 76 // Adatlapb�l sz�molt k�sleltet�s (tEWL)
#define EHIGH 93 // Adatlapb�l sz�molt k�sleletet�s (tCYC-tEWL)
void SetE()
{
	volatile int i;
	for(i=0;i<ELOW;i++);
	HAL_GPIO_WritePin(EPort,E,GPIO_PIN_SET);
	for(i=0;i<EHIGH;i++);
}

void ClearE()
{
	HAL_GPIO_WritePin(EPort,E,GPIO_PIN_RESET);
}

void EPulse()
{
	SetE();
	ClearE();
}

void SetDataReg()
{
	HAL_GPIO_WritePin(DIPort,DI,GPIO_PIN_SET);
}

void SetInstructionReg()
{
	HAL_GPIO_WritePin(DIPort,DI,GPIO_PIN_RESET);
}

void LcdReset(int state)
{
	if(!state) HAL_GPIO_WritePin(RSTPort,RST,GPIO_PIN_SET);
	else HAL_GPIO_WritePin(RSTPort,RST,GPIO_PIN_RESET);
}

void WaitBusy() // Megv�rjuk, am�g a kiv�lasztott chip busy �llapotban van.
{
	volatile int i;
	// Lementj�k a kiv�lasztott regisztert.
	int di = HAL_GPIO_ReadPin(DIPort,DI);

	SetDataDirIn();
	SetInstructionReg();

		i = 1;
		while(i) //Addig v�runk, am�g a Busy Flag �rt�ke nem 0.
			{
				SetE();
				i = HAL_GPIO_ReadPin(DPort,DB7);
				ClearE();
			}

	HAL_GPIO_WritePin(DIPort,DI,(di==0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
	SetDataDirOut(); // Az alap�rtelmezett ir�ny az output, �gy visszaford�tjuk az adatvonalakat.
}
