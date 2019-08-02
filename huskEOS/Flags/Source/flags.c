/*************************************************************************/
/*  File Name: flags.c                                                   */
/*  Purpose: Flags services for application layer tasks.                 */
/*  Created by: Garrett Sculthorpe on 3/24/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

   
/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "flags_internal_IF.h"
#include "flags.h"
#include "sch_internal_IF.h"
#include "sch.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define FLAGS_RESET_VALUE           (0x00)
#define FLAGS_NULL_PTR              ((void*)ZERO)
#define FLAGS_EVENT_TYPE_MIN_VALID  (1)
#define FLAGS_EVENT_TYPE_MAX_VALID  (2)

/*************************************************************************/
/*  External References                                                  */
/*************************************************************************/
extern void OSTaskFault(void);

/*************************************************************************/
/*  Static Global Variables, Constants                                   */
/*************************************************************************/
static FlagsObj flags_s_flagsList[RTOS_CFG_NUM_FLAG_OBJECTS];

/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/


/*************************************************************************/

/*************************************************************************/
/*  Function Name: u1_OSflags_init                                       */
/*  Purpose:       Initialize flags object.                              */
/*  Arguments:     FlagsObj** flags:                                     */
/*                        Address of flags object.                       */
/*                 U1 flagInitValues:                                    */
/*                        Initial values for flags.                      */
/*  Return:        N/A                                                   */
/*************************************************************************/
U1 u1_OSflags_init(struct FlagsObj** flags, U1 flagInitValues)
{
         U1 u1_t_index;
         U1 u1_t_returnSts;
  static U1 u1_s_numFlagsAllocated = (U1)ZERO;
  
  u1_t_returnSts = (U1)FLAGS_NO_OBJ_AVAILABLE;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Have flags pointer point to available object. */
  if(u1_s_numFlagsAllocated < (U1)RTOS_CFG_NUM_FLAG_OBJECTS)
  {  
    (*flags) = &flags_s_flagsList[u1_s_numFlagsAllocated];
    
    ++u1_s_numFlagsAllocated;
    
    (*flags)->flags = flagInitValues;
    
    for(u1_t_index = (U1)ZERO; u1_t_index < (U1)RTOS_CFG_MAX_NUM_TASKS_PEND_FLAGS; u1_t_index++)
    {
      (*flags)->pendingList[u1_t_index].event         = (U1)ZERO;
      (*flags)->pendingList[u1_t_index].tcb           = FLAGS_NULL_PTR;
      (*flags)->pendingList[u1_t_index].eventPendType = (U1)ZERO;
    }
    
    u1_t_returnSts = (U1)FLAGS_INIT_SUCCESS;    
  }
  else
  {
    
  }
  
  OS_SCH_EXIT_CRITICAL();
  
  return (u1_t_returnSts);
}

/*************************************************************************/
/*  Function Name: u1_OSflags_postFlags                                  */
/*  Purpose:       Write to flags object as specified.                   */
/*  Arguments:     FlagsObj* flags:                                      */
/*                    Pointer to flags object.                           */
/*                 U1 flagMask:                                          */
/*                    Mask to set/clear.                                 */
/*                 U1 set_clear:                                         */
/*                    FLAGS_WRITE_SET                 OR                 */
/*                    FLAGS_WRITE_CLEAR                                  */
/*  Return:        U4 FLAGS_WRITE_COMMAND_INVALID     OR                 */
/*                    FLAGS_WRITE_SUCCESS                                */
/*************************************************************************/
U1 u1_OSflags_postFlags(struct FlagsObj* flags, U1 flagMask, U1 set_clear)
{
  U1 u1_t_returnSts;
  U1 u1_t_index;
  
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
  
  if(u1_t_returnSts == (U1)FLAGS_WRITE_SUCCESS)
  {
    /* Check if there is a task waiting on event. */
    for(u1_t_index = (U1)ZERO; u1_t_index < (U1)FLAGS_MAX_NUM_TASKS_PENDING; u1_t_index++)
    {
      if(flags->pendingList[u1_t_index].tcb != FLAGS_NULL_PTR)
      {
        switch(flags->pendingList[u1_t_index].eventPendType)
        {
          case (U1)FLAGS_EVENT_ANY:          
            if((flags->pendingList[u1_t_index].event & flags->flags) != (U1)ZERO)
            {
              /* Wake up task and notify scheduler of reason. */
              vd_OSsch_setReasonForWakeup((U1)(flags->flags), (flags->pendingList[u1_t_index].tcb));
              
              /* Notify scheduler to change task state. If woken task is higher priority than running task, context switch will occur after critical section. */
              vd_OSsch_taskWake(flags->pendingList[u1_t_index].tcb->taskID);
              
              /* Clear data in flags object. */
              flags->pendingList[u1_t_index].event         = (U1)ZERO;
              flags->pendingList[u1_t_index].tcb           = FLAGS_NULL_PTR;
              flags->pendingList[u1_t_index].eventPendType = (U1)ZERO;
            }
            else
            {
               
            }
            break;
          
          case (U1)FLAGS_EVENT_EXACT:
            if((flags->pendingList[u1_t_index].event & flags->flags) == flags->flags)
            {
              /* Wake up task and notify scheduler of reason. */
              vd_OSsch_setReasonForWakeup((U1)(flags->flags), (flags->pendingList[u1_t_index].tcb));
              
              /* Notify scheduler to change task state. If woken task is higher priority than running task, context switch will occur after critical section. */
              vd_OSsch_taskWake(flags->pendingList[u1_t_index].tcb->taskID);
              
              /* Clear data in flags object. */
              flags->pendingList[u1_t_index].event         = (U1)ZERO;
              flags->pendingList[u1_t_index].tcb           = FLAGS_NULL_PTR;
              flags->pendingList[u1_t_index].eventPendType = (U1)ZERO;
            }
            else
            {
               
            }  
            break;
            
          default: /* Only occurs if there is data corruption. */
            OSTaskFault();
            break;            
        }/* End switch{} */
        
      }
      else
      {
        
      }/*flags->pendingList[u1_t_index].tcb != FLAGS_NULL_PTR */
    }/* End for{} */
  }
  else
  {
    
  }/* u1_t_returnSts = (U1)FLAGS_WRITE_SUCCESS */
  
  OS_CPU_EXIT_CRITICAL();
  
  return (u1_t_returnSts);
}

/*************************************************************************/
/*  Function Name: u1_OSflags_pendOnFlags                                */
/*  Purpose:       Set task to pend on certain flags. When task is woken,*/
/*                 it can retrieve the waking event by calling           */
/*                 u1_OSsch_getReasonForWakeup()                         */
/*                                                                       */
/*  Arguments:     FlagsObj* flags:                                      */
/*                      Pointer to flags object.                         */
/*                 U1 eventMask:                                         */
/*                      Event that will cause wakeup.                    */
/*                 U4 timeOut:                                           */
/*                      Maximum wait time. If 0, then wait is indefinite.*/
/*                 U1 eventType:                                         */
/*                      Type of event - FLAGS_EVENT_ANY,                 */
/*                                      FLAGS_EVENT_EXACT                */
/*                                                                       */
/*  Return:        U1: FLAGS_PEND_LIST_FULL     OR                       */
/*                     FLAGS_PEND_SUCCESS                                */
/*************************************************************************/
U1 u1_OSflags_pendOnFlags(struct FlagsObj* flags, U1 eventMask, U4 timeOut, U1 eventType) 
{
  U1 u1_t_returnSts;
  U1 u1_t_index;
  
  u1_t_returnSts = (U1)FLAGS_PEND_LIST_FULL;
  
  if((eventType >= (U1)FLAGS_EVENT_TYPE_MIN_VALID) && (eventType <= (U1)FLAGS_EVENT_TYPE_MAX_VALID))
  {
    OS_CPU_ENTER_CRITICAL();
  
    /* Check if there is a task waiting on event. */
    for(u1_t_index = (U1)ZERO; u1_t_index < (U1)FLAGS_MAX_NUM_TASKS_PENDING; u1_t_index++)
    {
      if(flags->pendingList[u1_t_index].tcb == FLAGS_NULL_PTR)
      {
        u1_t_returnSts = (U1)FLAGS_PEND_SUCCESS;
        
        /* Set event conditions */
        flags->pendingList[u1_t_index].event         = eventMask;
        flags->pendingList[u1_t_index].tcb           = SCH_CURRENT_TCB_ADDR;
        flags->pendingList[u1_t_index].eventPendType = eventType;
        
        /* Notify scheduler the reason for sleep state. */
        vd_OSsch_setReasonForSleep(flags, (U1)SCH_TASK_SLEEP_RESOURCE_FLAGS);
        
        /* If indefinite timeout */
        if(timeOut == (U4)ZERO)
        {
          vd_OSsch_taskSuspend(SCH_CURRENT_TASK_ID);
        }
        /* If defined timeout */
        else
        {
          vd_OSsch_taskSleep(timeOut);
        }
        
        break; /* Break loop */
      }
      else
      {

      }/* flags->pendingList[u1_t_index].tcb == FLAGS_NULL_PTR */
    }/* End for{} */
    
    OS_CPU_EXIT_CRITICAL();
  }
  else
  {
    
  }/* if eventType */  
  
  return (u1_t_returnSts);
}

/*************************************************************************/
/*  Function Name: vd_flags_pendTimeout                                  */
/*  Purpose:       Timeout hander for pending task. Called by scheduler. */
/*  Arguments:     FlagsObj* flags:                                      */
/*                      Pointer to flags object.                         */
/*                 Sch_Task pendingTCB:                                  */
/*                      Pointer to timed out task's TCB.                 */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_flags_pendTimeout(struct FlagsObj* flags, struct Sch_Task* pendingTCB)
{
  U1 u1_t_index;
  
  OS_CPU_ENTER_CRITICAL();
  
  /* Check if there is a task waiting on event. */
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)FLAGS_MAX_NUM_TASKS_PENDING; u1_t_index++)
  {
    if(flags->pendingList[u1_t_index].tcb == pendingTCB)
    {
      flags->pendingList[u1_t_index].tcb           = FLAGS_NULL_PTR;
      flags->pendingList[u1_t_index].event         = (U1)ZERO;
      flags->pendingList[u1_t_index].eventPendType = (U1)ZERO;
    }
    else
    {
      
    }
  }/* End for{} */
  
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_OSflags_reset                                      */
/*  Purpose:       Reset flags object to init state.                     */
/*  Arguments:     Flags* flags:                                         */
/*                     Pointer to flags object.                          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void  vd_OSflags_reset(struct FlagsObj* flags)
{
  U1 u1_t_index;
  
  OS_CPU_ENTER_CRITICAL();
  
  flags->flags = (U1)FLAGS_RESET_VALUE;
  
  /* Check if there is a task waiting on event. Clear the spot if so. */
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)FLAGS_MAX_NUM_TASKS_PENDING; u1_t_index++)
  {
    if(flags->pendingList[u1_t_index].tcb != FLAGS_NULL_PTR)
    {
      vd_OSsch_setReasonForWakeup((U1)SCH_TASK_WAKEUP_FLAGS_CLEARED, (flags->pendingList[u1_t_index].tcb));
      vd_OSsch_taskWake(flags->pendingList[u1_t_index].tcb->taskID);
      
      flags->pendingList[u1_t_index].tcb           = FLAGS_NULL_PTR;
      flags->pendingList[u1_t_index].event         = (U1)ZERO;
      flags->pendingList[u1_t_index].eventPendType = (U1)ZERO;
    }
    else
    {
      
    }
  }/* End for{} */
  
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_OSflags_clearAll                                   */
/*  Purpose:       Clear all flags.                                      */
/*  Arguments:     Flags* flags:                                         */
/*                     Pointer to flags object.                          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void  vd_OSflags_clearAll(struct FlagsObj* flags)
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
U1 u1_OSflags_checkFlags(struct FlagsObj* flags)
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
/* 1.0                7/29/19     Re-wrote flags module to handle a user-configured number of  */
/*                                tasks that can pend on each flags object.                    */
