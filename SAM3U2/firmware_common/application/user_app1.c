/*!*********************************************************************************************************************
@file user_app1.c                                                                
@brief User's tasks / applications are written here.  This description
should be replaced by something specific to the task.

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- void UserApp1Initialize(void)
- void UserApp1RunActiveState(void)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                          /*!< @brief Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                   /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                    /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                     /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;                /*!< @brief From main.c */
extern volatile u8 G_u8DebugScanfCharCount;

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_pfStateMachine;               /*!< @brief The state machine function pointer */
//static u32 UserApp1_u32Timeout;                           /*!< @brief Timeout counter used across states */
static SspConfigurationType SPIConfig;
static SspPeripheralType* MyTaskSsp;
static u8 au8RxBuffer[100];
static u8* pu8RxBuffer=au8RxBuffer;
static u8 u8TMessage=0xFF;
static u8 u8DebugMes[4]={0xFF,0,0,0};

static bool bTX=FALSE;

static u8 au8Checkerboard[9]={0,0,0,0,0,0,0,0,0};
static u8 au8String1[]="   |   |   \n\r";
static u8 au8StringA[]=" 0 | 1 | 2 \n\r";
static u8 au8StringB[]=" 3 | 4 | 5 \n\r";
static u8 au8StringC[]=" 6 | 7 | 8 \n\r";
static u8 au8String2[]="--- --- ---\n\r";

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void UserApp1Initialize(void)

@brief
Initializes the State Machine and its variables.

Should only be called once in main init section.

Requires:
- NONE

Promises:
- NONE

*/
void UserApp1Initialize(void)
{
  AT91C_BASE_PIOB->PIO_SODR=0x00800000;
  AT91C_BASE_PIOB->PIO_SODR=0x01000000;
  
  SPIConfig.SspPeripheral=USART2;
  SPIConfig.pCsGpioAddress=AT91C_BASE_PIOB;
  SPIConfig.u32CsPin=PB_22_ANT_USPI2_CS;
  SPIConfig.eBitOrder=LSB_FIRST;
  SPIConfig.eSspMode=SPI_SLAVE_FLOW_CONTROL;
  SPIConfig.fnSlaveRxFlowCallback=&SlaveRx;
  SPIConfig.fnSlaveTxFlowCallback=&SlaveTx;
  SPIConfig.pu8RxBufferAddress=au8RxBuffer;
  SPIConfig.ppu8RxNextByte=&pu8RxBuffer;
  SPIConfig.u16RxBufferSize=128;

  MyTaskSsp=SspRequest(&SPIConfig);
  if(MyTaskSsp!=NULL)
  {
    LedOn(GREEN);
  }
  else
  {
    LedOn(WHITE);
  }
  
  DebugPrintf("game start\n\r");
  DebugPrintf(au8String1);
  DebugPrintf(au8StringA);
  DebugPrintf(au8String1);
  DebugPrintf(au8String2);
  DebugPrintf(au8String1);
  DebugPrintf(au8StringB);
  DebugPrintf(au8String1);
  DebugPrintf(au8String2);
  DebugPrintf(au8String1);
  DebugPrintf(au8StringC);
  DebugPrintf(au8String1);  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_pfStateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */

  
/*!----------------------------------------------------------------------------------------------------------------------
@fn void UserApp1RunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void UserApp1RunActiveState(void)
{
    UserApp1_pfStateMachine();
} /* end UserApp1RunActiveState */


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
void SlaveRx(void)
{
    if(au8RxBuffer[0]==0xF1)
    {
      AT91C_BASE_PIOB->PIO_SODR=0x00800000;
      AT91C_BASE_PIOB->PIO_SODR=0x01000000;
      u8TMessage=0xFF;
      u8DebugMes[0]=0xFF;
      au8RxBuffer[0]=0xFF;
      bTX=FALSE;
      DebugPrintf("\r\n");
      LedOff(BLUE);
    }
}
void SlaveTx()
{
  
}

bool CheckBoard(u8 u8Location,u8 *pau8Board)
{
  if(*(pau8Board+u8Location)==0)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

bool PrintBoard(u8 u8Location,u8 u8ChessPlayer)
{
  /* 1pc = * ; 0phone= + 
 
               0123456789
 au8StringA[]=" 0 | 1 | 2 \n\r";
 au8StringB[]=" 3 | 4 | 5 \n\r";
 au8StringC[]=" 6 | 7 | 8 \n\r";
  */
  switch(u8Location)
  {
  case 0:
    if(u8ChessPlayer)
    {
      *(au8StringA+1)='*';
    }
    else
    {
      *(au8StringA+1)='+';
    }
    break;
  case 1:
    if(u8ChessPlayer)
    {
      *(au8StringA+5)='*';
    }
    else
    {
      *(au8StringA+5)='+';
    }    
    ;break;
  case 2:
    if(u8ChessPlayer)
    {
      *(au8StringA+9)='*';
    }
    else
    {
      *(au8StringA+9)='+';
    }    
    ;
    break;

  case 3:
    if(u8ChessPlayer)
    {
      *(au8StringB+1)='*';
    }
    else
    {
      *(au8StringB+1)='+';
    }
    break;
  case 4:
    if(u8ChessPlayer)
    {
      *(au8StringB+5)='*';
    }
    else
    {
      *(au8StringB+5)='+';
    }    
    ;break;
  case 5:
    if(u8ChessPlayer)
    {
      *(au8StringB+9)='*';
    }
    else
    {
      *(au8StringB+9)='+';
    }    
    ;
    break;    
    
  case 6:
    if(u8ChessPlayer)
    {
      *(au8StringC+1)='*';
    }
    else
    {
      *(au8StringC+1)='+';
    }
    break;
  case 7:
    if(u8ChessPlayer)
    {
      *(au8StringC+5)='*';
    }
    else
    {
      *(au8StringC+5)='+';
    }    
    ;break;
  case 8:
    if(u8ChessPlayer)
    {
      *(au8StringC+9)='*';
    }
    else
    {
      *(au8StringC+9)='+';
    }    
    ;
    break;
    
  default:break;
  }
  DebugPrintf("\n\r");
  DebugPrintf(au8String1);
  DebugPrintf(au8StringA);
  DebugPrintf(au8String1);
  DebugPrintf(au8String2);
  DebugPrintf(au8String1);
  DebugPrintf(au8StringB);
  DebugPrintf(au8String1);
  DebugPrintf(au8String2);
  DebugPrintf(au8String1);
  DebugPrintf(au8StringC);
  DebugPrintf(au8String1);  
  
  return TRUE;
}
/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------------*/
/* What does this state do? */
static void UserApp1SM_Idle(void)
{  
    if(!bTX)
    {
      if(G_u8DebugScanfCharCount)
      {
        DebugScanf(u8DebugMes);
        
        if(u8DebugMes[0]>='0'&&u8DebugMes[0]<='8')
        {
          u8TMessage = u8DebugMes[0]-'0';
          LedOn(BLUE);
     
          if(CheckBoard(u8TMessage,au8Checkerboard))
          {
            au8Checkerboard[u8TMessage]=0x0A;/* 0x0A PC*/
            if(PrintBoard(u8TMessage,1))
            {
              AT91C_BASE_PIOB->PIO_SODR=0x00800000;
              AT91C_BASE_PIOB->PIO_CODR=0x01000000;
              bTX=TRUE;
            }
          }
          else
          {
            DebugPrintf("\n\r CAN NOT LOAD! \n\r");
          }
         }
        else
        {
          DebugPrintf("\n\r Please input 0~8 \n\r");
        }/*end bebug*/     
      }
    }
    else
    {
     SspWriteByte(MyTaskSsp,u8TMessage);
    }

} /* end UserApp1SM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
