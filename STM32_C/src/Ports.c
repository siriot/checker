#include "bsp.h"

/*
 * GPIO-k inicializálása, az egyes adatvonalak közvetlen, alacsony szintû beállítása, hardware elfedése.
 */

void InitPorts()
{
	GPIO_InitTypeDef gpio;

	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();

	//adat bitek, input
	gpio.Mode = GPIO_MODE_INPUT; // az adatbitek az összehajtás elkerülése végett kezdetben inputok
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Pin = DBall;
	HAL_GPIO_Init(GPIOD,&gpio);
	// vezérlõ bitek
	gpio.Mode = GPIO_MODE_OUTPUT_PP;

	gpio.Pin = E | DI | RST;
	HAL_GPIO_Init(GPIOD,&gpio);
	HAL_GPIO_WritePin(RSTPort,RST,GPIO_PIN_RESET); //Az LCD-t resetben tartjuk kezdetben.
	HAL_GPIO_WritePin(EPort,E,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DIPort,DI,GPIO_PIN_RESET);

	gpio.Pin = RW;
	HAL_GPIO_Init(GPIOE,&gpio);
	HAL_GPIO_WritePin(RWPort, RW, GPIO_PIN_RESET); // Alapértelmezésben írunk.

	gpio.Pin = EN | CS1 | CS2;
	HAL_GPIO_Init(GPIOB,&gpio);
	HAL_GPIO_WritePin(ENPort,EN,GPIO_PIN_RESET); // Engedélyezzük a meghajtót.
	HAL_GPIO_WritePin(CSPort,CS1 | CS2, GPIO_PIN_SET);// Töröljük a chip kiválasztása.

	// gombok
	gpio.Mode=GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLUP;
	gpio.Pin = BTN0 | BTN1 | BTN2 | BTN3 | BTN4;
	HAL_GPIO_Init(GPIOD,&gpio);

	//PWM láb inicializálása, C port 8-as pin
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Alternate = GPIO_AF3_TIM8;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Pull = GPIO_NOPULL;
	gpio.Pin = GPIO_PIN_8;
	HAL_GPIO_Init(GPIOC,&gpio);

	HAL_GPIO_WritePin(RSTPort,RST,GPIO_PIN_SET); //Feloldjuk az LCD reset-jét.
}

void SelectChip1() // Az LCD-ben található 1. vezérlõ chip kiválasztása.
{
	HAL_GPIO_WritePin(CSPort,CS1,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CSPort,CS2,GPIO_PIN_SET);
}

void SelectChip2()// Az LCD-ben található 1. vezérlõ chip kiválasztása.
{
	HAL_GPIO_WritePin(CSPort,CS2,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CSPort,CS1,GPIO_PIN_SET);
}


void WriteData(uint16_t data)
{
	SetDataDirOut(); // Kifelé állítjuk az adatvonalakat.
	WaitBusy(); // Megvárjuk, míg mindkét chip készen van.
	data &= 0x00FF;
	DPort->ODR &= 0x00FF;
	DPort->ODR |= data<<8; //Adatok kiírása
	EPulse();
}
void SetDataDirOut() // A GPIO újrainicializálásával outputtá fordítjuk az adatvonalakat.
{
	GPIO_InitTypeDef gpio;

	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Pin = DBall;

	HAL_GPIO_WritePin(RWPort,RW,GPIO_PIN_RESET);
	HAL_GPIO_Init(DPort,&gpio);
}

void SetDataDirIn() // A GPIO újrainicializálásával inputtá fordítjuk az adatvonalakat.
{
	GPIO_InitTypeDef gpio;

	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_LOW;
	gpio.Pin = DBall;
	HAL_GPIO_Init(DPort,&gpio);

	HAL_GPIO_WritePin(RWPort,RW,GPIO_PIN_SET);
}

#define ELOW 76 // Adatlapból számolt késleltetés (tEWL)
#define EHIGH 93 // Adatlapból számolt késleletetés (tCYC-tEWL)
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

void WaitBusy() // Megvárjuk, amíg a kiválasztott chip busy állapotban van.
{
	volatile int i;
	// Lementjük a kiválasztott regisztert.
	int di = HAL_GPIO_ReadPin(DIPort,DI);

	SetDataDirIn();
	SetInstructionReg();

		i = 1;
		while(i) //Addig várunk, amíg a Busy Flag értéke nem 0.
			{
				SetE();
				i = HAL_GPIO_ReadPin(DPort,DB7);
				ClearE();
			}

	HAL_GPIO_WritePin(DIPort,DI,(di==0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
	SetDataDirOut(); // Az alapértelmezett irány az output, így visszafordítjuk az adatvonalakat.
}
