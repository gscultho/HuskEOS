/*************************************************************************/
/*  File Name: semaphore.c                                               */
/*  Purpose: Semaphore services for application layer tasks.             */
/*  Created by: Garrett Sculthorpe on 3/24/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

   
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
#define SEMA_BLOCKED_TASK_ID_OFFSET    (1)
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
static void vd_sema_blockTask(Semaphore* semaphore);
static void vd_sema_unblockTask(Semaphore* semaphore);


/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_OSsema_init                                        */
/*  Purpose:       Initialize specified semaphore.                       */
/*  Arguments:     Semaphore** semaphore:                                */
/*                            Address of semaphore object.               */
/*                 S1 initValue:                                         */
/*                            Initial value for semsphore.               */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsema_init(struct Semaphore** semaphore, S1 initValue)
{
         U1 u1_t_index;
  static U1 u1_s_numSemaAllocated = ZERO;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Have semaphore pointer point to available object */
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
  
  OS_SCH_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: u1_OSsema_wait                                        */
/*  Purpose:       Claim semaphore referenced by pointer.                */
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*                 U4 blockPeriod:                                       */
/*                    Number of time units for task to sleep if blocked. */
/*                                                                       */
/*  Return:        U1 SEMA_SEMAPHORE_TAKEN      OR                       */
/*                    SEMA_SEMAPHORE_SUCCESS                             */
/*************************************************************************/
U1 u1_OSsema_wait(struct Semaphore* semaphore, U4 blockPeriod)
{
  U1 u1_t_returnSts;
  
  u1_t_returnSts = (U1)SEMA_SEMAPHORE_TAKEN;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Check if available */
  if(!(semaphore->sema))
  {  
    /* If non-blocking function call, exit critical section and return immediately */
    if(blockPeriod == (U4)SEMA_NO_BLOCK)
    {  
      OS_SCH_EXIT_CRITICAL();
    }
    /* Else block task */
    else
    {
      vd_sema_blockTask(semaphore); /* Add task to resource blocked list */
      vd_sch_setReasonForSleep(semaphore, (U1)SCH_TASK_SLEEP_RESOURCE_SEMA); /* Tell scheduler the reason for task block state */
      OS_SCH_EXIT_CRITICAL();
      vd_OSsch_taskSleep(blockPeriod); /* Set sleep timer and change task state */
      
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
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*  Return:        U1 SEMA_SEMAPHORE_TAKEN     OR                        */
/*                    SEMA_SEMAPHORE_SUCCESS                             */
/*************************************************************************/
U1 u1_OSsema_check(struct Semaphore* semaphore)
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
/*  Function Name: u1_OSsema_post                                        */
/*  Purpose:       Release semaphore                                     */
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
U1 u1_OSsema_post(struct Semaphore* semaphore)
{
  OS_SCH_ENTER_CRITICAL();
  
  ++(semaphore->sema);
  vd_sema_unblockTask(semaphore);
  OS_SCH_EXIT_CRITICAL();
  
  return ((U1)SEMA_SEMAPHORE_SUCCESS);
}

/*************************************************************************/
/*  Function Name: vd_sema_blockedTimeout                                */
/*  Purpose:       API for scheduler to call when sleeping task times out*/
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*                 Sch_Task* taskTCB:                                    */
/*                     Pointer to TCB of blocked task.                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_sema_blockedTimeout(struct Semaphore* semaphore, struct Sch_Task* taskTCB)
{
  ListNode* node_t_tempPtr;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Remove node from block list and clear its contents */
  node_t_tempPtr      = node_list_removeNodeByTCB(&(semaphore->blockedListHead), taskTCB);
  node_t_tempPtr->TCB = SEMA_NULL_PTR;
  
  OS_SCH_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_sema_blockTask                                     */
/*  Purpose:       Add task to blocked list of semaphore.                */
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_sema_blockTask(struct Semaphore* semaphore)
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
    (semaphore->blockedTasks[u1_t_index].TCB) = tcb_OSsch_getCurrentTCB();
    vd_list_addTaskByPrio(&(semaphore->blockedListHead), &(semaphore->blockedTasks[u1_t_index]));
  }
}

/*************************************************************************/
/*  Function Name: vd_sema_unblockTasks                                  */
/*  Purpose:       Wake up tasks blocked on semaphore.                   */
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_sema_unblockTask(struct Semaphore* semaphore)
{
  ListNode* node_t_p_highPrioTask;
  
  /* If blocked list is not empty */
  if(semaphore->blockedListHead != SEMA_NULL_PTR)
  { 
    /* Remove highest priority task */    
    node_t_p_highPrioTask = node_list_removeFirstNode(&(semaphore->blockedListHead));
    
    /* Notify scheduler to change state. If woken task is higher priority than running task, context switch will occur after critical section. */    
    vd_sch_setReasonForWakeup((U1)SCH_TASK_WAKEUP_SEMA_READY, node_t_p_highPrioTask->TCB);
    
    /* Clear TCB pointer. This frees this node for future use. */
    node_t_p_highPrioTask->TCB = SEMA_NULL_PTR;   
  }
}

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                3/24/19     Module implemented                                           */
/*                                                                                             */
/* 0.2                5/24/19     Added API for scheduler to use when blocked task times out.  */
/*                                                                                             */
/* 0.3                7/26/19     Block list structure changed to utilize list module.         */
/*                                                                                             */
