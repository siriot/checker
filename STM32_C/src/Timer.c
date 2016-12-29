#include "bsp.h"

volatile int ButtonEvent = 0;
volatile int NewEvent = 0;
volatile int ToggleSignal = 0;

/*
 * Id�z�t�k inicializ�l�sa, interrupt rutinok.
 */
TIM_HandleTypeDef tim4;
TIM_HandleTypeDef tim3;
TIM_HandleTypeDef tim8;

char btn[5] = {1,1,1,1,1};

void InitTimers()
{
	//Gombokat mintav�telez� timer, peri�dusid�: 10ms.
	__TIM4_CLK_ENABLE();
	tim4.Channel = TIM_CHANNEL_1;
	tim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim4.Init.CounterMode=TIM_COUNTERMODE_UP;
	tim4.Init.Period=83;
	tim4.Init.Prescaler=9999;
	tim4.Instance = TIM4;
	HAL_TIM_Base_Init(&tim4);
	HAL_TIM_Base_Start_IT(&tim4);
	HAL_NVIC_SetPriority(TIM4_IRQn,0,0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);

	// B�buk villog�s�t id�z�t� timer. Peri�dusid�: 1s
	__TIM3_CLK_ENABLE();
	tim3.Channel = TIM_CHANNEL_1;
	tim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim3.Init.CounterMode=TIM_COUNTERMODE_UP;
	tim3.Init.Period=8399;
	tim3.Init.Prescaler=9999;
	tim3.Instance = TIM3;
	HAL_TIM_Base_Init(&tim3);
	HAL_NVIC_SetPriority(TIM3_IRQn,0,1);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);


	// H�tt�rvil�g�t�s PWM-es szab�lyoz�sa
	__TIM8_CLK_ENABLE();
	tim8.Channel = TIM_CHANNEL_3;
	tim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim8.Init.Period = 99;
	tim8.Init.Prescaler = 999;
	tim8.Init.RepetitionCounter = 0;
	tim8.Instance = TIM8;
	tim8.State = HAL_TIM_STATE_RESET;
	HAL_TIM_Base_Init(&tim8);
	HAL_TIM_PWM_Init(&tim8);

	TIM_OC_InitTypeDef tim8oc;
	tim8oc.OCMode = TIM_OCMODE_PWM1;
	tim8oc.Pulse = BACKLIGHT;	// Kit�lt�si t�nyez�
	tim8oc.OCIdleState = TIM_OCIDLESTATE_RESET;
	tim8oc.OCFastMode = TIM_OCFAST_DISABLE;
	tim8oc.OCIdleState = TIM_OCIDLESTATE_RESET;
	tim8oc.OCNIdleState = TIM_OCNIDLESTATE_SET;
	tim8oc.OCNPolarity = TIM_OCNPOLARITY_LOW;
	tim8oc.OCPolarity = TIM_OCPOLARITY_HIGH;


	HAL_TIM_PWM_ConfigChannel(&tim8,&tim8oc,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&tim8,TIM_CHANNEL_3);


}

void StartBlinkTimer() // Elind�tja a b�buk villog�s�t.
{
	HAL_TIM_Base_Start_IT(&tim3);
}
void StopBlinkTimer()// Le�ll�tja a b�buk villog�s�t.
{
	HAL_TIM_Base_Stop_IT(&tim3);
}

void TIM4_IRQHandler()
{
	HAL_TIM_IRQHandler(&tim4);
}

void TIM3_IRQHandler()
{
	HAL_TIM_IRQHandler(&tim3);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM4)
	{
		int i;
		volatile char state;
		for(i=0;i<5;i++)
		{
			state = HAL_GPIO_ReadPin(BTNPort,BTN0<<i); //Egyes�vel beolvassuk a gombok �llapot�t.
			if(btn[i] == 1 && state == 0) // Lefut� �lt keres�nk. (A gombok alacsony akt�vak.)
			{
				ButtonEvent = i;
				NewEvent = 1; // �zen�nk a main-nek, hogy �j esem�ny k�vetkezett be.
				btn[i]=0;
			}
			else if(btn[i] == 0 && state == 1) btn[i] = 1; //Felenged�s
		}
	}else if(htim->Instance == TIM3)
	{
		ToggleSignal = 1; //Toggle �zenet.
	}
}
