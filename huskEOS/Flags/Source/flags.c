/*************************************************************************/
/*  File Name: flags.c                                                   */
/*  Purpose: Flags services for application layer tasks.                 */
/*  Created by: Garrett Sculthorpe on 3/24/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

   
/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "flags.h"
#include "sch.h"

/* One task can pend on each FlagsObj */

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define FLAGS_RESET_VALUE        (0x00)
#define FLAGS_SCH_TASK_ID_OFFSET (1)

/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/


/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_OSflags_init                                       */
/*  Purpose:       Initialize flags object.                              */
/*  Arguments:     FlagsObj* flags:                                      */
/*                        Pointer to flags object.                       */
/*                 U1     flagInitValues:                                */
/*                        Initial values for flags.                      */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSflags_init(FlagsObj* flags, U1 flagInitValues)
{
  OS_CPU_ENTER_CRITICAL();
  flags->flags        = flagInitValues;
  flags->pendedTaskID = (U1)ZERO;
  flags->pendEvent    = (U1)ZERO;
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: u1_OSflags_postFlags                                  */
/*  Purpose:       Write to flags object as specified.                   */
/*  Arguments:     FlagsObj* flags:                                      */
/*                    Pointer to flags object.                           */
/*                 U1 flagMask:                                          */
/*                    Number of time units for task to sleep if blocked. */
/*                 U1 set_clear:                                         */
/*                    FLAGS_WRITE_SET                 OR                 */
/*                    FLAGS_WRITE_CLEAR                                  */
/*  Return:        U4 FLAGS_WRITE_COMMAND_INVALID     OR                 */
/*                    FLAGS_WRITE_SUCCESS                                */
/*************************************************************************/
U1 u1_OSflags_postFlags(FlagsObj* flags, U1 flagMask, U1 set_clear)
{
  U1 u1_t_returnSts;
  
  OS_CPU_ENTER_CRITICAL();
  
  if(set_clear == (U1)FLAGS_WRITE_SET)
  {
    flags->flags  |= flagMask;
    u1_t_returnSts = (U1)FLAGS_WRITE_SUCCESS;
  }
  else if(set_clear == (U1)FLAGS_WRITE_CLEAR)
  {
    flags->flags  &= ~(flagMask);
    u1_t_returnSts = (U1)FLAGS_WRITE_SUCCESS;
  }
  else
  {
    /* Default return status */
    u1_t_returnSts = (U1)FLAGS_WRITE_COMMAND_INVALID;
  }
  
  /* check if task waiting on event, if condition met then wake flag, set reason, reset flags data */
  if(flags->pendedTaskID)
  {
    if((flags->flags & flags->pendEvent) == flags->pendEvent)
    {
      vd_sch_setReasonForWakeup((U1)SCH_TASK_WAKEUP_FLAGS_EVENT, (flags->pendedTaskID - (U1)FLAGS_SCH_TASK_ID_OFFSET));
      
      flags->pendedTaskID = (U1)ZERO;
      flags->pendEvent    = (U1)ZERO;      
    }
    else
    {
      //NOP
    }
  }
  else
  {
    //NOP
  }
  
  OS_CPU_EXIT_CRITICAL();
  
  return (u1_t_returnSts);
}

/*************************************************************************/
/*  Function Name: vd_OSflags_pendOnFlags                                */
/*  Purpose:       Set task to pend on certain flags.                    */
/*  Arguments:     FlagsObj* flags:                                      */
/*                      Pointer to flags object.                         */
/*                 U1 eventMask:                                         */
/*                      Event that will cause wakeup.                    */
/*                 U4 timeOut:                                           */
/*                      Maximum wait time.                               */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSflags_pendOnFlags(FlagsObj* flags, U1 eventMask, U4 timeOut) 
{
  OS_CPU_ENTER_CRITICAL();
  
  /* Set event */
  flags->pendEvent    = eventMask;
  flags->pendedTaskID = u1_OSsch_getCurrentTask() + (U1)FLAGS_SCH_TASK_ID_OFFSET;
  OS_CPU_EXIT_CRITICAL();
  
  /* Update scheduler that task is pending */
  vd_sch_setReasonForSleep(flags, (U1)SCH_TASK_SLEEP_RESOURCE_FLAGS);
  vd_OSsch_taskSleep(timeOut);
}

/*************************************************************************/
/*  Function Name: vd_flags_pendTimeout                                  */
/*  Purpose:       Timeout hander for pending task.                      */
/*  Arguments:     FlagsObj* flags:                                      */
/*                      Pointer to flags object.                         */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_flags_pendTimeout(FlagsObj* flags)
{
  flags->pendedTaskID = (U1)ZERO;
  flags->pendEvent    = (U1)ZERO;
}

/*************************************************************************/
/*  Function Name: vd_flags_clearAll                                     */
/*  Purpose:       Clear all flags.                                      */
/*  Arguments:     Flags* flags:                                         */
/*                     Pointer to flags object.                          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void  vd_OSflags_clearAll(FlagsObj* flags)
{
  OS_CPU_ENTER_CRITICAL();
  flags->flags = (U1)FLAGS_RESET_VALUE;
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: u1_flags_checkFlags                                   */
/*  Purpose:       Check flags but do not modify.                        */
/*  Arguments:     Flags* flags:                                         */
/*                     Pointer to flags object.                          */
/*  Return:        U1 u1_t_returnVal:                                    */
/*                    Value of flags at time they were read.             */
/*************************************************************************/
U1 u1_OSflags_checkFlags(FlagsObj* flags)
{
  U1 u1_t_returnVal;
  
  OS_CPU_ENTER_CRITICAL();
  u1_t_returnVal = (U1)(flags->flags);
  OS_CPU_EXIT_CRITICAL();
  
  return (u1_t_returnVal);
}

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                3/25/19     Initial implementation of flags module.                      */
/*                                                                                             */
/* 0.2                5/23/19     Added pend API and necessary handling.                       */
/*                                                                                             */
