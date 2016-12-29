#include "bsp.h"

volatile int ButtonEvent = 0;
volatile int NewEvent = 0;
volatile int ToggleSignal = 0;

/*
 * Idõzítõk inicializálása, interrupt rutinok.
 */
TIM_HandleTypeDef tim4;
TIM_HandleTypeDef tim3;
TIM_HandleTypeDef tim8;

char btn[5] = {1,1,1,1,1};

void InitTimers()
{
	//Gombokat mintavételezõ timer, periódusidõ: 10ms.
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

	// Bábuk villogását idõzítõ timer. Periódusidõ: 1s
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


	// Háttérvilágítás PWM-es szabályozása
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
	tim8oc.Pulse = BACKLIGHT;	// Kitöltési tényezõ
	tim8oc.OCIdleState = TIM_OCIDLESTATE_RESET;
	tim8oc.OCFastMode = TIM_OCFAST_DISABLE;
	tim8oc.OCIdleState = TIM_OCIDLESTATE_RESET;
	tim8oc.OCNIdleState = TIM_OCNIDLESTATE_SET;
	tim8oc.OCNPolarity = TIM_OCNPOLARITY_LOW;
	tim8oc.OCPolarity = TIM_OCPOLARITY_HIGH;


	HAL_TIM_PWM_ConfigChannel(&tim8,&tim8oc,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&tim8,TIM_CHANNEL_3);


}

void StartBlinkTimer() // Elindítja a bábuk villogását.
{
	HAL_TIM_Base_Start_IT(&tim3);
}
void StopBlinkTimer()// Leállítja a bábuk villogását.
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
			state = HAL_GPIO_ReadPin(BTNPort,BTN0<<i); //Egyesével beolvassuk a gombok állapotát.
			if(btn[i] == 1 && state == 0) // Lefutó élt keresünk. (A gombok alacsony aktívak.)
			{
				ButtonEvent = i;
				NewEvent = 1; // Üzenünk a main-nek, hogy új esemény következett be.
				btn[i]=0;
			}
			else if(btn[i] == 0 && state == 1) btn[i] = 1; //Felengedés
		}
	}else if(htim->Instance == TIM3)
	{
		ToggleSignal = 1; //Toggle üzenet.
	}
}
