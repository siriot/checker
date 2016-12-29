//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include "diag/Trace.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_cortex.h"

#include "bsp.h"
// ----------------------------------------------------------------------------
//
// STM32F4 empty sample (trace via ITM).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the ITM output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#include "bsp.h"


int main(int argc, char* argv[])
{
	SetupScreen(); // Inicializáljuk a képernyõt.
	ButtonEvent = 0;
	NewEvent = 0;
	ToggleSignal = 0;
	while(1)
	{
		if(NewEvent)
		{
			NewEvent = 0;
			if(!Reverse) EventHandler[GameState][ButtonEvent](); 	// Attól függõen, hogy melyik játékos aktív, más a gombok jelentése,
			else ReverseEventHandler[GameState][ButtonEvent]();	// így mások az eseménykezelõ függvények is.
		}
		if(ToggleSignal)
		{
			ToggleSignal = 0;
			TogglePieces(); //Villogtatjuk a bábukat.
		}
	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
