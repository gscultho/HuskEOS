/*************************************************************************/
/*  File Name: semaphore.c                                               */
/*  Purpose: Semaphore services for application layer tasks.             */
/*  Created by: Garrett Sculthorpe on 3/24/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

#include "rtos_cfg.h"

#if(RTOS_CFG_OS_SEMAPHORE_ENABLED == RTOS_CONFIG_TRUE)

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "listMgr_internal.h"
#include "semaphore_internal_IF.h"
#include "semaphore.h"
#include "sch_internal_IF.h"
#include "sch.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define SEMA_NO_BLOCK                  (0)
#define SEMA_NULL_PTR                  ((void*)0)
#define SEMA_NO_BLOCKED_TASKS          (0)
#define SEMA_NUM_SEMAPHORES            (RTOS_CFG_NUM_SEMAPHORES)

/*************************************************************************/
/*  Static Global Variables, Constants                                   */
/*************************************************************************/
static Semaphore sema_s_semaList[SEMA_NUM_SEMAPHORES]; 
  
/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
static void vd_OSsema_blockTask(OSSemaphore* semaphore);
static void vd_OSsema_unblockTask(OSSemaphore* semaphore);


/*************************************************************************/

/*************************************************************************/
/*  Function Name: u1_OSsema_init                                        */
/*  Purpose:       Initialize specified semaphore.                       */
/*  Arguments:     OSSemaphore** semaphore:                              */
/*                            Address of semaphore object.               */
/*                 S1 initValue:                                         */
/*                            Initial value for semsphore.               */
/*  Return:        U1: SEMA_SEMAPHORE_SUCCESS   OR                       */
/*                     SEMA_NO_SEMA_OBJECTS_AVAILABLE                    */
/*************************************************************************/
U1 u1_OSsema_init(OSSemaphore** semaphore, S1 initValue)
{
         U1 u1_t_index;
         U1 u1_t_returnSts; 
  static U1 u1_s_numSemaAllocated = (U1)ZERO;
  
  u1_t_returnSts = (U1)SEMA_NO_SEMA_OBJECTS_AVAILABLE;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Have semaphore pointer point to available object */
  if(u1_s_numSemaAllocated < (U1)SEMA_NUM_SEMAPHORES)
  {  
    (*semaphore) = &sema_s_semaList[u1_s_numSemaAllocated];
    
    ++u1_s_numSemaAllocated;
    
    (*semaphore)->sema            = initValue;
    (*semaphore)->blockedListHead = SEMA_NULL_PTR;
    
    for(u1_t_index = (U1)ZERO; u1_t_index < (U1)SEMA_MAX_NUM_BLOCKED; u1_t_index++)
    {
      (*semaphore)->blockedTasks[u1_t_index].nextNode     = SEMA_NULL_PTR;
      (*semaphore)->blockedTasks[u1_t_index].previousNode = SEMA_NULL_PTR;
      (*semaphore)->blockedTasks[u1_t_index].TCB          = SEMA_NULL_PTR;
    }
    
    u1_t_returnSts = (U1)SEMA_SEMAPHORE_SUCCESS;    
  }
  else
  {
    
  }
  
  OS_SCH_EXIT_CRITICAL();
  
  return (u1_t_returnSts);
}

/*************************************************************************/
/*  Function Name: u1_OSsema_wait                                        */
/*  Purpose:       Claim semaphore referenced by pointer.                */
/*  Arguments:     OSSemaphore* semaphore:                               */
/*                     Pointer to semaphore.                             */
/*                 U4 blockPeriod:                                       */
/*                    Number of time units for task to sleep if blocked. */
/*                                                                       */
/*  Return:        U1 SEMA_SEMAPHORE_TAKEN      OR                       */
/*                    SEMA_SEMAPHORE_SUCCESS                             */
/*************************************************************************/
U1 u1_OSsema_wait(OSSemaphore* semaphore, U4 blockPeriod)
{
  U1 u1_t_returnSts;
  
  u1_t_returnSts = (U1)SEMA_SEMAPHORE_TAKEN;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Check if available */
  if(semaphore->sema == (U1)ZERO)
  {  
    /* If non-blocking function call, exit critical section and return immediately */
    if(blockPeriod == (U4)SEMA_NO_BLOCK)
    {  
      OS_SCH_EXIT_CRITICAL();
    }
    /* Else block task */
    else
    {
      vd_OSsema_blockTask(semaphore); /* Add task to resource blocked list */
      /* Tell scheduler the reason for task block state, 
      set sleep timer and change task state */
      vd_OSsch_setReasonForSleep(semaphore, (U1)SCH_TASK_SLEEP_RESOURCE_SEMA, blockPeriod);  
      OS_SCH_EXIT_CRITICAL();
      
      /* Check again after task wakes up */
      OS_SCH_ENTER_CRITICAL();
      
      if(semaphore->sema) /* If available */
      {
        --(semaphore->sema);
        u1_t_returnSts = (U1)SEMA_SEMAPHORE_SUCCESS;        
      }
      
      OS_SCH_EXIT_CRITICAL();
    }
  }
  else /* Semaphore is available */
  {  
    --(semaphore->sema);
    OS_SCH_EXIT_CRITICAL();
    
    u1_t_returnSts = (U1)SEMA_SEMAPHORE_SUCCESS;
  }
  
  return (u1_t_returnSts);
}

/*************************************************************************/
/*  Function Name: u1_OSsema_check                                       */
/*  Purpose:       Check status of semaphore.                            */
/*  Arguments:     OSSemaphore* semaphore:                               */
/*                     Pointer to semaphore.                             */
/*  Return:        U1 SEMA_SEMAPHORE_TAKEN     OR                        */
/*                    SEMA_SEMAPHORE_SUCCESS                             */
/*************************************************************************/
U1 u1_OSsema_check(OSSemaphore* semaphore)
{
  U1 u1_t_sts;
  
  OS_SCH_ENTER_CRITICAL();
  
  switch (semaphore->sema)
  {  
    case SEMA_SEMAPHORE_TAKEN:
      u1_t_sts = (U1)SEMA_SEMAPHORE_TAKEN;
      break;
    
    default:
      u1_t_sts = (U1)SEMA_SEMAPHORE_SUCCESS;
      break;
  }
  
  OS_SCH_EXIT_CRITICAL();
  
  return (u1_t_sts);
}

/*************************************************************************/
/*  Function Name: vd_OSsema_post                                        */
/*  Purpose:       Release semaphore                                     */
/*  Arguments:     OSSemaphore* semaphore:                               */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsema_post(OSSemaphore* semaphore)
{
  OS_SCH_ENTER_CRITICAL();
  
  ++(semaphore->sema);
  
  /* If blocked list is not empty */
  if(semaphore->blockedListHead != SEMA_NULL_PTR)
  { 
    vd_OSsema_unblockTask(semaphore);
  }
  else{}
  
  OS_SCH_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_OSsema_blockedTimeout                              */
/*  Purpose:       API for scheduler to call when sleeping task times out*/
/*  Arguments:     OSSemaphore* semaphore:                               */
/*                     Pointer to semaphore.                             */
/*                 Sch_Task* taskTCB:                                    */
/*                     Pointer to TCB of blocked task.                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsema_blockedTimeout(OSSemaphore* semaphore, struct Sch_Task* taskTCB)
{
  ListNode* node_t_tempPtr;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Remove node from block list and clear its contents */
  node_t_tempPtr      = node_list_removeNodeByTCB(&(semaphore->blockedListHead), taskTCB);
  node_t_tempPtr->TCB = SEMA_NULL_PTR;
  
  OS_SCH_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_OSsema_blockTask                                   */
/*  Purpose:       Add task to blocked list of semaphore.                */
/*  Arguments:     OSSemaphore* semaphore:                               */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_OSsema_blockTask(OSSemaphore* semaphore)
{
  U1 u1_t_index;
  
  u1_t_index = (U1)ZERO;
  
  /* Find available node to store data */
  while((semaphore->blockedTasks[u1_t_index].TCB != SEMA_NULL_PTR) && (u1_t_index < (U1)SEMA_MAX_NUM_BLOCKED)) 
  {    
    ++u1_t_index;
  }
  /* If node found, then store TCB pointer and add to blocked list */
  if(u1_t_index < (U1)SEMA_MAX_NUM_BLOCKED)
  {
    (semaphore->blockedTasks[u1_t_index].TCB) = SCH_CURRENT_TCB_ADDR;
    vd_list_addTaskByPrio(&(semaphore->blockedListHead), &(semaphore->blockedTasks[u1_t_index]));
  }
}

/*************************************************************************/
/*  Function Name: vd_OSsema_unblockTask                                 */
/*  Purpose:       Wake up tasks blocked on semaphore.                   */
/*  Arguments:     OSSemaphore* semaphore:                               */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_OSsema_unblockTask(OSSemaphore* semaphore)
{
  ListNode* node_t_p_highPrioTask;
  
  /* Remove highest priority task */    
  node_t_p_highPrioTask = node_list_removeFirstNode(&(semaphore->blockedListHead));
    
  /*  Notify scheduler the reason that task is going to be woken. */    
  vd_OSsch_setReasonForWakeup((U1)SCH_TASK_WAKEUP_SEMA_READY, node_t_p_highPrioTask->TCB);
  
  /* Notify scheduler to change task state. If woken task is higher priority than running task, context switch will occur after critical section. */
  vd_OSsch_taskWake(node_t_p_highPrioTask->TCB->taskID);
  
  /* Clear TCB pointer. This frees this node for future use. */
  node_t_p_highPrioTask->TCB = SEMA_NULL_PTR;   
}

#endif /* Conditional compile */

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                3/24/19     Module implemented                                           */
/*                                                                                             */
/* 0.2                5/24/19     Added API for scheduler to use when blocked task times out.  */
/*                                                                                             */
/* 1.0                7/26/19     Block list structure changed to utilize list module.         */
/*                                                                                             */
