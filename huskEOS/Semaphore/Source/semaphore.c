/*************************************************************************/
/*  File Name: semaphore.c                                               */
/*  Purpose: Binary semaphore services for application layer tasks.      */
/*  Created by: Garrett Sculthorpe on 3/24/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

   
/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "semaphore.h"
#include "sch.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define SEMA_SEMAPHORE_STS_INIT        (1)
#define SEMA_NO_BLOCK                  (0)
#define SEMA_NO_BLOCKED_TASK           (0)
#define SEMA_BLOCKED_TASK_SLOT_OPEN    (1)
#define SEMA_BLOCKED_TASK_SLOT_CLOSED  (0)
#define SEMA_BLOCKED_TASK_ID_OFFSET    (1)

/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
static void vd_sema_blockTask(Semaphore* semaphore);
static void vd_sema_unblockTasks(Semaphore* semaphore);


/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_OSsema_init                                        */
/*  Purpose:       Initialize specified semaphore.                       */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsema_init(Semaphore* semaphore)
{
  U1 u1_t_index;
  
  OS_SCH_ENTER_CRITICAL();
  semaphore->sema = (S1)SEMA_SEMAPHORE_STS_INIT;
  
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)SEMA_MAX_NUM_BLOCKED; u1_t_index++)
  {
    semaphore->blockedTasks[u1_t_index] = (U1)SEMA_NO_BLOCKED_TASK;
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
U1 u1_OSsema_wait(Semaphore* semaphore, U4 blockPeriod)
{
  U1 ut_t_returnSts;
  
  ut_t_returnSts = (U1)SEMA_SEMAPHORE_TAKEN;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Check if available */
  if(!(semaphore->sema))
  {  
    if(blockPeriod == (U4)SEMA_NO_BLOCK)
    {  
      OS_SCH_EXIT_CRITICAL();
    }
    else
    {
      vd_sema_blockTask(semaphore);
      vd_sch_setReasonForSleep(semaphore, (U1)SCH_TASK_SLEEP_RESOURCE_SEMA);
      OS_SCH_EXIT_CRITICAL();
      vd_OSsch_taskSleep(blockPeriod);
    }
  }
  else
  {  
    --(semaphore->sema);
    OS_SCH_EXIT_CRITICAL();
    
    ut_t_returnSts = (U1)SEMA_SEMAPHORE_SUCCESS;
  }
  
  return (ut_t_returnSts);
}

/*************************************************************************/
/*  Function Name: u1_OSsema_check                                       */
/*  Purpose:       Check status of semaphore.                            */
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*  Return:        U1 SEMA_SEMAPHORE_TAKEN     OR                        */
/*                    SEMA_SEMAPHORE_SUCCESS                             */
/*************************************************************************/
U1 u1_OSsema_check(Semaphore* semaphore)
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
U1 u1_OSsema_post(Semaphore* semaphore)
{
  OS_SCH_ENTER_CRITICAL();
  
  ++(semaphore->sema);
  vd_sema_unblockTasks(semaphore);
  OS_SCH_EXIT_CRITICAL();
  
  return ((U1)SEMA_SEMAPHORE_SUCCESS);
}

/*************************************************************************/
/*  Function Name: vd_sema_blockedTimeout                                */
/*  Purpose:       API for scheduler to call when sleeping task times out*/
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_sema_blockedTimeout(Semaphore* semaphore, U1 taskID)
{
  U1 u1_t_index;
  U1 u1_t_storedTask;
  
  OS_SCH_ENTER_CRITICAL();
  
  u1_t_storedTask = taskID + (U1)SEMA_BLOCKED_TASK_ID_OFFSET;
  
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)SEMA_MAX_NUM_BLOCKED; ++u1_t_index)
  {
    if((semaphore->blockedTasks[u1_t_index]) == u1_t_storedTask)
    {
      semaphore->blockedTasks[u1_t_index] = (U1)SEMA_NO_BLOCKED_TASK;
      break;
    }
  }
  
  OS_SCH_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_sema_blockTask                                     */
/*  Purpose:       Add task to blocked list of semaphore.                */
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_sema_blockTask(Semaphore* semaphore)
{
  U1 u1_t_index;
  U1 u1_t_slotFlag;
  
  u1_t_index    = (U1)ZERO;
  u1_t_slotFlag = (U1)SEMA_BLOCKED_TASK_SLOT_OPEN;
  
  /* Find open slot on list */
  while((semaphore->blockedTasks[u1_t_index])) 
  {    
    if(u1_t_index == (U1)SEMA_MAX_NUM_BLOCKED)
    {
      u1_t_slotFlag = (U1)SEMA_BLOCKED_TASK_SLOT_CLOSED;
      break;
    }
    ++u1_t_index;
  }
  
  if(u1_t_slotFlag == (U1)SEMA_BLOCKED_TASK_SLOT_OPEN)
  {
    (semaphore->blockedTasks[u1_t_index]) = (u1_OSsch_getCurrentTask() + (U1)SEMA_BLOCKED_TASK_ID_OFFSET);
  }
}

/*************************************************************************/
/*  Function Name: vd_sema_unblockTasks                                  */
/*  Purpose:       Wake up tasks blocked on semaphore. Called in         */
/*                 critical section.                                     */
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_sema_unblockTasks(Semaphore* semaphore)
{
  U1 u1_t_index;
  
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)SEMA_MAX_NUM_BLOCKED; ++u1_t_index)
  {    
    if(semaphore->blockedTasks[u1_t_index] != (U1)SEMA_NO_BLOCKED_TASK)
    {                                                                                                  /* compensate for offset by 1 */
      vd_sch_setReasonForWakeup((U1)SCH_TASK_WAKEUP_SEMA_READY, semaphore->blockedTasks[u1_t_index] - (U1)SEMA_BLOCKED_TASK_ID_OFFSET);
      semaphore->blockedTasks[u1_t_index] = (U1)SEMA_NO_BLOCKED_TASK;
    }
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
