/**********************************************************************************************************************
File: anttt.c                                                                

Description:
Implements TIC-TAC-TOE using data input from ANT or BLE.



**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxAnttt"
***********************************************************************************************************************/
/* New variables */
u32 G_u32AntttFlags;                                     /* Global state flags */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */
extern volatile u32 G_u32BPEngenuicsFlags;             /* From bleperipheral_engenuics.c  */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Anttt_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Anttt_pfnStateMachine;              /* The application state machine function pointer */

static u32 u32RXD=0xFF;
static u32 u32TXD=0xFF;

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void ChangeMode(void)
{
  u32 u32PinState=((0x00000300&NRF_GPIO->IN)>>8);
  
  switch(u32PinState)
  {
    /*
    1:NOT READY or NOT WORK 
    0:READY
    */

    /*MRDY=0 SRDY=1*/
    case 2:
      NRF_GPIO->OUTCLR = P0_28_;    
      NRF_GPIO->OUTSET = P0_27_;
      NRF_GPIO->OUTCLR = P0_10_;
      NRF_SPI0->EVENTS_READY = 0x00000000;
      Anttt_pfnStateMachine=AntttSM_TXD;
    /*MRDY=1 SRDY=0*/
    case 1:
      NRF_GPIO->OUTCLR = P0_28_;
      NRF_GPIO->OUTSET = P0_26_;
      NRF_GPIO->OUTCLR = P0_10_;
      NRF_SPI0->EVENTS_READY = 0x00000000;
      Anttt_pfnStateMachine=AntttSM_RXD;
      
  default :break;
  }
}

void BackWait(void)
{
  u32 u32PinState=((0x00000300&NRF_GPIO->IN)>>8);
  if(u32PinState==3)
  {
    NRF_GPIO->OUTSET = P0_28_;  
    NRF_GPIO->OUTCLR = P0_27_;
    NRF_GPIO->OUTCLR = P0_26_;
    NRF_GPIO->OUTSET = P0_10_;
    NRF_SPI0->EVENTS_READY = 0x00000001;    
    Anttt_pfnStateMachine=AntttSM_WAIT;
  }
}
/*--------------------------------------------------------------------------------------------------------------------
Function: AntttInitialize

Description:
Initializes the State Machine and its variables.

Requires:

Promises:
*/
void AntttInitialize(void)
{
  NRF_SPI0->PSELMISO     = 12;
  NRF_SPI0->PSELMOSI     = 13;
  NRF_SPI0->PSELSCK      = 11;
  NRF_SPI0->FREQUENCY    = ANT_SPI0_FRE_1Mbps;
  NRF_SPI0->CONFIG       = ANT_SPI0_CONFIG;
  NRF_SPI0->EVENTS_READY = 0x00000001;
  
  NRF_SPI0->TXD = 0xFF;
 
  NRF_TWI0->ENABLE=0x00000000UL;
  NRF_SPI0->ENABLE= NRF_SPI0_ENABLE_CNF;
  NRF_GPIO->OUTSET = P0_28_;
  Anttt_pfnStateMachine = AntttSM_WAIT;
  
} /* end AntttInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function AntttRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void AntttRunActiveState(void)
{
  Anttt_pfnStateMachine();

} /* end AntttRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------------------------------------------*/
/* State Machine definitions                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_Idle
*/
static void AntttSM_Idle(void)
{
      nrf_delay_us(100000);
} 

/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_WAIT
*/
static void AntttSM_WAIT(void)
{
  ChangeMode();
}
/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_TXD
*/
static void AntttSM_TXD(void)
{
  
}

/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_RXD
*/
static void AntttSM_RXD(void)
{
  if((u32RXD>=0x00)&&(u32RXD<=0x08))
  {
   u32TXD=0xF1;
  }  
  NRF_SPI0->TXD=u32TXD;/**/
  u32RXD=NRF_SPI0->RXD;
  if(NRF_SPI0->EVENTS_READY==1)
  {
     NRF_SPI0->EVENTS_READY=0;
  }
  BackWait();
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
