
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SI_EFM8BB1_Register_Enums.h>                  // SFR declarations
#include "InitDevice.h"
// $[Generated Includes]
// [Generated Includes]$

//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
int main (void)
{
	// Call hardware initialization routine
	enter_DefaultMode_from_RESET();

	while (1) 
   {
     // $[Generated Run-time code]
     // [Generated Run-time code]$
		PCON0 |= PCON0_IDLE__IDLE; // Sleep. Action goes on in UART interrupt.
   }                             
}
