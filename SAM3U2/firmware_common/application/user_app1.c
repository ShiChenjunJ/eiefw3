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
static u8 au8RxBuffer[128];
static u8* pu8RxBuffer=au8RxBuffer;
static u8 u8TMessage=0xFF;
static u8 u8DebugMes[4]={0xFF,0,0,0};

static u8 au8Checkerboard[9]={0,0,0,0,0,0,0,0,0};/*0x0A:PC 0x0B:phone*/
static u32 u32Time=0;
static u8 u8PHOload=0xFF;

static u8 au8StringBoard[][12]={
                              "---|---|---",
                              " 0 | 1 | 2 ",
                              "---|---|---",
                              " 3 | 4 | 5 ",
                              "---|---|---",
                              " 6 | 7 | 8 ",
                              "---|---|---"};
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
  LedOff(WHITE);
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  
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
  
  DebugPrintf("game start");
  DebugLineFeed();
  for(u8 i=0;i<8;i++)
  {
    DebugPrintf(au8StringBoard[i]);
    DebugLineFeed();
  }
  /* If good initialization, set state to Idle */
  if( 1 )
  { /*UserApp1_pfStateMachine = UserApp1SM_Delay;*/
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
  /*
    if(au8RxBuffer[0]==0xF1)
    {
      AT91C_BASE_PIOB->PIO_SODR=0x00800000;
      AT91C_BASE_PIOB->PIO_SODR=0x01000000;
      u8TMessage=0xFF;
      au8RxBuffer[0]=0xFF;
      u32Time=G_u32SystemTime1ms;
      UserApp1_pfStateMachine=UserApp1SM_Delay;
      return;
    }
   */
      if((au8RxBuffer[0]>=0)&&(au8RxBuffer[0]<=8))
      {
          AT91C_BASE_PIOB->PIO_SODR=0x00800000;
          AT91C_BASE_PIOB->PIO_SODR=0x01000000;
 
          u8PHOload=au8RxBuffer[0];
          u8TMessage=0xFF;
          au8RxBuffer[0]=0xFF;
          /*UserApp1_pfStateMachine=UserApp1SM_PHONE2;*/
      }
}
void SlaveTx(void)
{
  
}

bool CheckBoard(u8 u8Location,u8 *pau8Board)
{
  if(*(pau8Board+u8Location))
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

bool PrintBoard(u8 u8Location,u8 u8ChessPlayer)
{
  /* 1pc = * ; 0phone= + 
    u8StringBoard[][14]={      0123456789
                            0 "---|---|---
                            1 " 0 | 1 | 2 
                            2 "---|---|---
                            3 " 3 | 4 | 5 
                            4 "---|---|---
                            5 " 6 | 7 | 8 
                            6 "---|---|---
  */
  switch(u8Location)
  {
  case 0:
    if(u8ChessPlayer)
    {
      *(&au8StringBoard[1][1])='*';
    }
    else
    {
      *(&au8StringBoard[1][1])='X';
    }
    return TRUE;
    
  case 1:
    if(u8ChessPlayer)
    {
      *(&au8StringBoard[1][5])='*';
    }
    else
    {
      *(&au8StringBoard[1][5])='X';
    }  
    return TRUE;
    
  case 2:
    if(u8ChessPlayer)
    {
      *(&au8StringBoard[1][9])='*';
    }
    else
    {
      *(&au8StringBoard[1][9])='X';
    }
    return TRUE;   

  case 3:
    if(u8ChessPlayer)
    {
      *(&au8StringBoard[3][1])='*';
    }
    else
    {
      *(&au8StringBoard[3][1])='X';
    }
    return TRUE;
    
  case 4:
    if(u8ChessPlayer)
    {
      *(&au8StringBoard[3][5])='*';
    }
    else
    {
      *(&au8StringBoard[3][5])='X';
    }
    return TRUE;    
    
  case 5:
    if(u8ChessPlayer)
    {
      *(&au8StringBoard[3][9])='*';
    }
    else
    {
      *(&au8StringBoard[3][9])='X';
    } 
    return TRUE;
    
  case 6:
    if(u8ChessPlayer)
    {
      *(&au8StringBoard[5][1])='*';
    }
    else
    {
      *(&au8StringBoard[5][1])='X';
    }
    return TRUE;
    
  case 7:
    if(u8ChessPlayer)
    {
      *(&au8StringBoard[5][5])='*';
    }
    else
    {
      *(&au8StringBoard[5][5])='X';
    }    
    return TRUE;
    
  case 8:
    if(u8ChessPlayer)
    {
      *(&au8StringBoard[5][9])='*';
    }
    else
    {
      *(&au8StringBoard[5][9])='X';
    }  
    return TRUE;

  default:return FALSE;
  }
}

void Printfgame(void)
{
  DebugLineFeed();
  for(u8 i=0;i<8;i++)
  {
    DebugPrintf(au8StringBoard[i]);
    DebugLineFeed();
  }
}

void CheckWinner(u8 *u8Board,PlayerType Player)
{
  /*012*/
  if((*(u8Board+0)==*(u8Board+1))&&(*(u8Board+1)==*(u8Board+2))&&(*(u8Board+2)!=0))
  {
    if(*(u8Board+0)==0x0A)
    {
      DebugLineFeed();
      DebugPrintf("PC winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
    if(*(u8Board+1)==0x0B)
    {
      DebugLineFeed();
      DebugPrintf("Phone winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
  }
 
  /*345*/
  if((*(u8Board+3)==*(u8Board+4))&&(*(u8Board+4)==*(u8Board+5))&&(*(u8Board+5)!=0))
  {
    if(*(u8Board+3)==0x0A)
    {
      DebugLineFeed();
      DebugPrintf("PC winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
    if(*(u8Board+1)==0x0B)
    {
      DebugLineFeed();
      DebugPrintf("Phone winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
  }  

  /*678*/
  if((*(u8Board+6)==*(u8Board+7))&&(*(u8Board+7)==*(u8Board+8))&&(*(u8Board+8)!=0))
  {
    if(*(u8Board+6)==0x0A)
    {
      DebugLineFeed();
      DebugPrintf("PC winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
    if(*(u8Board+1)==0x0B)
    {
      DebugLineFeed();
      DebugPrintf("Phone winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
  }  

  /*036*/
  if((*(u8Board+0)==*(u8Board+3))&&(*(u8Board+3)==*(u8Board+6))&&(*(u8Board+6)!=0))
  {
    if(*(u8Board+0)==0x0A)
    {
      DebugLineFeed();
      DebugPrintf("PC winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
    if(*(u8Board+1)==0x0B)
    {
      DebugLineFeed();
      DebugPrintf("Phone winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
  }  
  
  /*147*/
  if((*(u8Board+1)==*(u8Board+4))&&(*(u8Board+4)==*(u8Board+7))&&(*(u8Board+7)!=0))
  {
    if(*(u8Board+1)==0x0A)
    {
      DebugLineFeed();
      DebugPrintf("PC winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
    if(*(u8Board+1)==0x0B)
    {
      DebugLineFeed();
      DebugPrintf("Phone winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
  } 

  /*258*/
  if((*(u8Board+2)==*(u8Board+5))&&(*(u8Board+5)==*(u8Board+8))&&(*(u8Board+8)!=0))
  {
    if(*(u8Board+2)==0x0A)
    {
      DebugLineFeed();
      DebugPrintf("PC winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
    if(*(u8Board+1)==0x0B)
    {
      DebugLineFeed();
      DebugPrintf("Phone winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
  }
  

  /*048*/
  if((*(u8Board+0)==*(u8Board+4))&&(*(u8Board+4)==*(u8Board+8))&&(*(u8Board+8)!=0))
  {
    if(*(u8Board+0)==0x0A)
    {
      DebugLineFeed();
      DebugPrintf("PC winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
    if(*(u8Board+1)==0x0B)
    {
      DebugLineFeed();
      DebugPrintf("Phone winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
  } 

  /*246*/
  if((*(u8Board+2)==*(u8Board+4))&&(*(u8Board+4)==*(u8Board+6))&&(*(u8Board+6)!=0))
  {
    if(*(u8Board+2)==0x0A)
    {
      DebugLineFeed();
      DebugPrintf("PC winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
    if(*(u8Board+1)==0x0B)
    {
      DebugLineFeed();
      DebugPrintf("Phone winner");
      UserApp1_pfStateMachine = UserApp1SM_ReStart;
      return;
    }
  }   

  if(Player==PCplayer)
  {
    /*
    AT91C_BASE_PIOB->PIO_SODR=0x00800000;
    AT91C_BASE_PIOB->PIO_CODR=0x01000000;
    */
    UserApp1_pfStateMachine = UserApp1SM_Delay;
  }
  else if(Player==IPHONEplayer)
  {   
    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }

}

/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------------*/
/* What does this state do? */
static void Chat(void)
{
  u32 u32PB22=(AT91C_BASE_PIOB->PIO_PDSR&0x00400000);
  
  if(!u32PB22)
  {
    u32Time=G_u32SystemTime1ms;
    UserApp1_pfStateMachine=Chat2;
  }
}

static void Chat2(void)
{
  if(IsTimeUp(&u32Time,200))
  {
    /*SspWriteByte(MyTaskSsp,u8TMessage);*/
    u32Time=G_u32SystemTime1ms;
  } 
}
static void UserApp1SM_Idle(void)
{  
  DebugScanf(u8DebugMes);

  if(u8DebugMes[0]>='0'&&u8DebugMes[0]<='8')
    {
      u8TMessage = u8DebugMes[0]-'0';
      u8DebugMes[0]=0xFF;
      if(CheckBoard(u8TMessage,au8Checkerboard))
        {
          au8Checkerboard[u8TMessage]=0x0A;/* 0x0A PC*/
          if(PrintBoard(u8TMessage,1))
          {
            Printfgame();
            CheckWinner(au8Checkerboard,PCplayer);
          }
        }
      else
      {
        DebugLineFeed();
        DebugPrintf("CAN NOT LOAD!");
        DebugLineFeed();
      }
    }/*end bebug*/     

} /* end UserApp1SM_Idle() */


static void UserApp1SM_PHONE2(void)
{
  u32 u32PB22=(AT91C_BASE_PIOB->PIO_PDSR&0x00400000);
  if(u32PB22)
  {
    if(CheckBoard(u8PHOload,au8Checkerboard))
      {
        au8Checkerboard[u8PHOload]=0x0B;
        if((PrintBoard(u8PHOload,0)))
        {
        Printfgame();  
        CheckWinner(au8Checkerboard,IPHONEplayer);
        }
      }
    else
    {
      DebugLineFeed();
      DebugPrintf("CAN NOT LOAD!");
      DebugLineFeed();
      UserApp1_pfStateMachine = UserApp1SM_Delay; 
    }
    u8PHOload=0xFF;
  }
}
static void UserApp1SM_Delay(void)
{
  if(IsTimeUp(&u32Time,100))
  {
     AT91C_BASE_PIOB->PIO_CODR=0x00800000;
     AT91C_BASE_PIOB->PIO_SODR=0x01000000;
     UserApp1_pfStateMachine = UserApp1SM_PHONE2;
     /*UserApp1_pfStateMachine = Chat;*/
  }
}


static void UserApp1SM_ReStart(void)
{
  if(WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);
    AT91C_BASE_PIOB->PIO_SODR=0x00800000;
    AT91C_BASE_PIOB->PIO_SODR=0x01000000;
    G_u8DebugScanfCharCount=0;
    u8 *u8BoardClear;
    u8BoardClear=au8Checkerboard;
    for(u8 i=0;i<9;i++)
    {
      *(u8BoardClear+i)=0;
    }

  DebugPrintf("\n\rgame restart");
  *(&au8StringBoard[1][1])='0';
  *(&au8StringBoard[1][5])='1';
  *(&au8StringBoard[1][9])='2';
  *(&au8StringBoard[3][1])='3';
  *(&au8StringBoard[3][5])='4';
  *(&au8StringBoard[3][9])='5';  
  *(&au8StringBoard[5][1])='6';
  *(&au8StringBoard[5][5])='7';
  *(&au8StringBoard[5][9])='8';
  DebugLineFeed();
    for(u8 i=0;i<9;i++)
  {
    DebugPrintf(au8StringBoard[i]);
    DebugLineFeed();
  }
  
  UserApp1_pfStateMachine = UserApp1SM_Idle;
  }
}
     
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
