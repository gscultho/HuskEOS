/*************************************************************************/
/*  File Name: mutex.c                                                   */
/*  Purpose: Mutex services for application layer tasks.                 */
/*  Created by: Garrett Sculthorpe on 8/14/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
/*************************************************************************/

#include "rtos_cfg.h"

#if(RTOS_CFG_OS_MUTEX_ENABLED == RTOS_CONFIG_TRUE)

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "listMgr_internal.h"
#include "mutex_internal_IF.h"
#include "mutex.h"
#include "sch_internal_IF.h"
#include "sch.h"

/*************************************************************************/
/*  External References                                                  */
/*************************************************************************/
extern void OSTaskFault(void);

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define MUTEX_NO_BLOCK                  (0)
#define MUTEX_NULL_PTR                  ((void*)0)
#define MUTEX_NUM_MUTEXES               (RTOS_CFG_MAX_NUM_MUTEX)
#define MUTEX_DEFAULT_PRIO              (0xFF)

/*************************************************************************/
/*  Static Global Variables, Constants                                   */
/*************************************************************************/
static Mutex mutex_s_mutexList[MUTEX_NUM_MUTEXES];
  
/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
static void vd_OSmutex_blockTask(struct Mutex* mutex);
static void vd_OSmutex_unblockTask(struct Mutex* mutex);


/*************************************************************************/

/*************************************************************************/
/*  Function Name: u1_OSmutex_init                                       */
/*  Purpose:       Initialize specified mutex.                           */
/*  Arguments:     OSMutex** mutex:                                      */
/*                            Address of mutex object.                   */
/*                 U1 initValue:                                         */
/*                            Initial value for mutex.                   */
/*  Return:        U1: MUTEX_SUCCESS   OR                                */
/*                     MUTEX_NO_OBJECTS_AVAILABLE                        */
/*************************************************************************/
U1 u1_OSmutex_init(OSMutex** mutex, U1 initValue)
{
         U1 u1_t_index;
         U1 u1_t_returnSts; 
  static U1 u1_s_numMutexAllocated = (U1)ZERO;
  
  u1_t_returnSts = (U1)MUTEX_NO_OBJECTS_AVAILABLE;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Have mutex pointer point to available object */
  if(u1_s_numMutexAllocated < (U1)MUTEX_NUM_MUTEXES)
  {  
    (*mutex) = &mutex_s_mutexList[u1_s_numMutexAllocated];
    
    ++u1_s_numMutexAllocated; 
    
    (*mutex)->lock                            = initValue%TWO;
    (*mutex)->blockedTaskList.blockedListHead = MUTEX_NULL_PTR;
    (*mutex)->priority.taskInheritedPrio      = (U1)MUTEX_DEFAULT_PRIO;
    (*mutex)->priority.taskRealPrio           = (U1)MUTEX_DEFAULT_PRIO;
    
    for(u1_t_index = (U1)ZERO; u1_t_index < (U1)MUTEX_MAX_NUM_BLOCKED; u1_t_index++)
    {
      (*mutex)->blockedTaskList.blockedTasks[u1_t_index].nextNode     = MUTEX_NULL_PTR;
      (*mutex)->blockedTaskList.blockedTasks[u1_t_index].previousNode = MUTEX_NULL_PTR;
      (*mutex)->blockedTaskList.blockedTasks[u1_t_index].TCB          = MUTEX_NULL_PTR;
    }
    
    u1_t_returnSts = (U1)MUTEX_SUCCESS;    
  }
  else
  {
    
  }
  
  OS_SCH_EXIT_CRITICAL();
  
  return (u1_t_returnSts);
}

/*************************************************************************/
/*  Function Name: u1_OSmutex_lock                                       */
/*  Purpose:       Claim mutex referenced by pointer.                    */
/*  Arguments:     OSMutex* mutex:                                       */
/*                     Pointer to mutex.                                 */
/*                 U4 blockPeriod:                                       */
/*                    Number of time units for task to sleep if blocked. */
/*                                                                       */
/*  Return:        U1 MUTEX_TAKEN      OR                                */
/*                    MUTEX_SUCCESS                                      */
/*************************************************************************/
U1 u1_OSmutex_lock(OSMutex* mutex, U4 blockPeriod)
{
  U1 u1_t_returnSts;
  
  u1_t_returnSts = (U1)MUTEX_TAKEN;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Check if available */
  if(mutex->lock == (U1)ZERO)
  {  
    /* If non-blocking function call, exit critical section and return immediately */
    if(blockPeriod == (U4)MUTEX_NO_BLOCK)
    {  
      OS_SCH_EXIT_CRITICAL();
    }
    /* Else block task */
    else
    {
      /* Per scheduler v2 requirement: No two tasks in active state can share the same priority. */
      vd_OSsch_setReasonForSleep(mutex, (U1)SCH_TASK_SLEEP_RESOURCE_MUTEX); /* Tell scheduler the reason for task block state */
      vd_OSsch_taskSleep(blockPeriod); /* Set sleep timer and change task state */  
      vd_OSmutex_blockTask(mutex); /* Add task to resource blocked list */
      
      OS_SCH_EXIT_CRITICAL();
      
      /* Check again after task wakes up */
      OS_SCH_ENTER_CRITICAL();
      
      if(mutex->lock) /* If available */
      {
        --(mutex->lock);      
        mutex->priority.mutexHolder = SCH_CURRENT_TCB_ADDR;    
        u1_t_returnSts              = (U1)MUTEX_SUCCESS;        
      }
      
      OS_SCH_EXIT_CRITICAL();
    }
  }
  else /* mutex is available */
  {  
    --(mutex->lock);
    mutex->priority.mutexHolder = SCH_CURRENT_TCB_ADDR;
    OS_SCH_EXIT_CRITICAL();
    
    u1_t_returnSts = (U1)MUTEX_SUCCESS;
  }
  
  return (u1_t_returnSts);
}

/*************************************************************************/
/*  Function Name: u1_OSmutex_check                                      */
/*  Purpose:       Check status of mutex.                                */
/*  Arguments:     OSMutex* mutex:                                       */
/*                     Pointer to mutex.                                 */
/*  Return:        U1 MUTEX_TAKEN     OR                                 */
/*                    MUTEX_SUCCESS                                      */
/*************************************************************************/
U1 u1_OSmutex_check(OSMutex* mutex)
{
  U1 u1_t_sts;
  
  OS_SCH_ENTER_CRITICAL();
  
  switch (mutex->lock)
  {  
    case (U1)MUTEX_TAKEN:
      u1_t_sts = (U1)MUTEX_TAKEN;
      break;
    
    case (U1)MUTEX_SUCCESS:
      u1_t_sts = (U1)MUTEX_SUCCESS;
      break;
    
    default:
      OSTaskFault(); /* This point is only reached if there is a software bug. */
      break;
  }
  
  OS_SCH_EXIT_CRITICAL();
  
  return (u1_t_sts);
}

/*************************************************************************/
/*  Function Name: u1_OSmutex_unlock                                     */
/*  Purpose:       Release mutex and manage priority inheritance.        */
/*  Arguments:     OSMutex* mutex:                                       */
/*                     Pointer to mutex.                                 */
/*  Return:        U1: MUTEX_SUCCESS          OR                         */
/*                     MUTEX_ALREADY_RELEASED                            */
/*************************************************************************/
U1 u1_OSmutex_unlock(OSMutex* mutex)
{
  U1 u1_t_return;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Only task that holds mutex can release it. */
  if(mutex->priority.mutexHolder == SCH_CURRENT_TCB_ADDR)
  {
    ++(mutex->lock);
    
    /* Unblock highest priority task on wait list and restore priority to normal value. */
    if(mutex->blockedTaskList.blockedListHead != MUTEX_NULL_PTR)
    {
      vd_OSmutex_unblockTask(mutex);
    }
    else
    {
      /* No blocked task. */ 
    }
    
    u1_t_return = (U1)MUTEX_SUCCESS;
  }
  else
  {
    u1_t_return = (U1)MUTEX_ALREADY_RELEASED;
  }
  
  OS_SCH_EXIT_CRITICAL();
  
  return (u1_t_return);
}

/*************************************************************************/
/*  Function Name: vd_OSmutex_blockedTimeout                             */
/*  Purpose:       API for scheduler to call when sleeping task times out*/
/*  Arguments:     Mutex* mutex:                                         */
/*                     Pointer to mutex.                                 */
/*                 Sch_Task* taskTCB:                                    */
/*                     Pointer to TCB of blocked task.                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSmutex_blockedTimeout(struct Mutex* mutex, struct Sch_Task* taskTCB)
{
  ListNode* node_t_tempPtr;
  U1        u1_t_newMutexHolderPrio;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Remove node from block list. */
  node_t_tempPtr = node_list_removeNodeByTCB(&(mutex->blockedTaskList.blockedListHead), taskTCB);
  
  /* If task with mutex has inherited this task's priority. */
  if(node_t_tempPtr->TCB->priority == mutex->priority.taskInheritedPrio)
  {
    /* If there is no other higher priority task then reset to original priority. */
    if(mutex->blockedTaskList.blockedListHead == MUTEX_NULL_PTR)
    {
      u1_t_newMutexHolderPrio           = mutex->priority.taskRealPrio;
      
      /* Reset internal data. */
      mutex->priority.taskRealPrio      = (U1)MUTEX_DEFAULT_PRIO;
      mutex->priority.taskInheritedPrio = (U1)MUTEX_DEFAULT_PRIO;
    }
    else
    {
      /* If the new highest prio blocked task priority is greater (numerically lower) than the current inherited priority, update the inherited priority. */
      if(mutex->blockedTaskList.blockedListHead->TCB->priority < mutex->priority.taskInheritedPrio)
      {    
        /* Update inherited priority. */
        u1_t_newMutexHolderPrio = mutex->blockedTaskList.blockedListHead->TCB->priority;
        
        /* Change internal record as well. */
        mutex->priority.taskInheritedPrio = mutex->blockedTaskList.blockedListHead->TCB->priority;
      }
      else /* No higher priority to inherit. */
      {
        u1_t_newMutexHolderPrio           = mutex->priority.taskRealPrio;
      
        /* Reset internal data. */
        mutex->priority.taskRealPrio      = (U1)MUTEX_DEFAULT_PRIO;
        mutex->priority.taskInheritedPrio = (U1)MUTEX_DEFAULT_PRIO;
      }
    }
    
    /* Notify scheduler of change. */
    (void)u1_OSsch_setNewPriority(mutex->priority.mutexHolder, u1_t_newMutexHolderPrio);   
  }
  else
  {
    /* No change to mutex holder's priority. */
  }/* node_t_tempPtr->TCB->priority == mutex->priority.taskInheritedPrio */
  
  node_t_tempPtr->TCB = MUTEX_NULL_PTR;
  
  OS_SCH_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_OSmutex_blockTask                                  */
/*  Purpose:       Add task to blocked list of mutex and handle priority */
/*                 inheritance.                                          */
/*  Arguments:     OSMutex* mutex:                                       */
/*                     Pointer to mutex.                                 */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_OSmutex_blockTask(OSMutex* mutex)
{
  U1 u1_t_index;
  U1 u1_t_priorityOriginal;
  
  u1_t_index = (U1)ZERO;
  
  /* Find available node to store data */
  while((mutex->blockedTaskList.blockedTasks[u1_t_index].TCB != MUTEX_NULL_PTR) && (u1_t_index < (U1)MUTEX_MAX_NUM_BLOCKED)) 
  {    
    ++u1_t_index;
  }
  /* If node found, then store TCB pointer and add to blocked list */
  if(u1_t_index < (U1)MUTEX_MAX_NUM_BLOCKED)
  {
    /* Add task to blocked list. */
    (mutex->blockedTaskList.blockedTasks[u1_t_index].TCB) = SCH_CURRENT_TCB_ADDR;
    vd_list_addTaskByPrio(&(mutex->blockedTaskList.blockedListHead), &(mutex->blockedTaskList.blockedTasks[u1_t_index]));
    
    /* If the blocking task's priority is greater (numerically lower) than the mutex holder's current priority, update the inherited priority. */
    if((mutex->blockedTaskList.blockedListHead->TCB->priority != mutex->priority.taskInheritedPrio) && (mutex->blockedTaskList.blockedListHead->TCB->priority < mutex->priority.mutexHolder->priority))
    {
      /* Notify scheduler of change. */
      u1_t_priorityOriginal = u1_OSsch_setNewPriority(mutex->priority.mutexHolder, mutex->blockedTaskList.blockedListHead->TCB->priority);
      
      /* Change internal record as well. */
      mutex->priority.taskInheritedPrio = mutex->blockedTaskList.blockedListHead->TCB->priority;
      
      /* Check if original priority needs to be stored internally. */
      if(mutex->priority.taskRealPrio == (U1)MUTEX_DEFAULT_PRIO)
      {
        mutex->priority.taskRealPrio = u1_t_priorityOriginal;
      }
      else
      {
        /* Original priority is already stored. */
      }
    }
    else
    {
      
    }/* (mutex->blockedTaskList.blockedListHead->TCB->priority < mutex->priority.taskInheritedPrio) */
  }/* (u1_t_index < (U1)MUTEX_MAX_NUM_BLOCKED) */
}

/*************************************************************************/
/*  Function Name: vd_OSmutex_unblockTask                                */
/*  Purpose:       Wake up task blocked on mutex, handle priorities.     */
/*  Arguments:     OSMutex* mutex:                                       */
/*                     Pointer to mutex.                                 */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_OSmutex_unblockTask(OSMutex* mutex)
{
  ListNode* node_t_p_highPrioTask;

  /* Remove highest priority task */    
  node_t_p_highPrioTask = node_list_removeFirstNode(&(mutex->blockedTaskList.blockedListHead));
  
  /* If task holding mutex had inherited priority, restore original priority. */
  if(mutex->priority.taskInheritedPrio != (U1)MUTEX_DEFAULT_PRIO)
  {
    (void)u1_OSsch_setNewPriority(mutex->priority.mutexHolder, mutex->priority.taskRealPrio);
    mutex->priority.taskInheritedPrio = (U1)MUTEX_DEFAULT_PRIO;
  }
  else
  {
    /* Priority was not inherited. */ 
  }
    
  /* Reset internal data. */
  mutex->priority.mutexHolder  = MUTEX_NULL_PTR;
  mutex->priority.taskRealPrio = (U1)MUTEX_DEFAULT_PRIO;
  
  /*  Notify scheduler the reason that task is going to be woken. */    
  vd_OSsch_setReasonForWakeup((U1)SCH_TASK_WAKEUP_MUTEX_READY, node_t_p_highPrioTask->TCB);
  
  /* Notify scheduler to change task state. If woken task is higher priority than running task, context switch will occur after critical section. */
  vd_OSsch_taskWake(node_t_p_highPrioTask->TCB->taskID);
  
  /* Clear TCB pointer. This frees this node for future use. */
  node_t_p_highPrioTask->TCB = MUTEX_NULL_PTR;   
}

#endif /* Conditional compile */

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                8/19/19     Module implemented for first time.                           */
/*                                                                                             */

