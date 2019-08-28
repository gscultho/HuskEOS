/*************************************************************************/
/*  File Name:  mutex_internal_IF.h                                      */
/*  Purpose:    Kernel access definitions and routines for mutex.        */
/*  Created by: Garrett Sculthorpe on 5/23/19                            */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

#ifndef mutex_internal_IF_h 
#if(RTOS_CFG_OS_MUTEX_ENABLED == RTOS_CONFIG_TRUE)
#define mutex_internal_IF_h

#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define MUTEX_MAX_NUM_BLOCKED           (RTOS_CFG_MAX_NUM_BLOCKED_TASKS_MUTEX)
  
/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
struct listNode; /* Forward declaration. Defined in "listMgr_internal.h" */
struct Sch_Task; /* Forward declaration. Defined in sch_internal_IF.h" */

/* Handle task blocking on each mutex. */
typedef struct BlockedTasks
{
  struct ListNode  blockedTasks[MUTEX_MAX_NUM_BLOCKED];          /* Memory allocated for storing blocked task data. */
  struct ListNode* blockedListHead;                              /* Pointer to first blocked task in list (highest priority. */
}
BlockedTasks;

/* Priority management. */
typedef struct PrioInheritance
{
  U1               taskRealPrio;             /* Real priority of task holding mutex. */
  U1               taskInheritedPrio;        /* Inherited priority of task holding mutex. */
  struct Sch_Task* mutexHolder;              /* Pointer to task holding the mutex. */
}
PrioInheritance;

typedef struct Mutex
{
  U1              lock;                      /* Binary lock value (1 is available). */
  BlockedTasks    blockedTaskList;           /* Group for handling blocked tasks. */
  PrioInheritance priority;                  /* Group for handling priority inheritance. */
}
Mutex;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: vd_OSmutex_blockedTimeout                             */
/*  Purpose:       API for scheduler to call when sleeping task times out*/
/*  Arguments:     Mutex* mutex:                                         */
/*                     Pointer to mutex.                                 */
/*                 Sch_Task* taskTCB:                                    */
/*                     Pointer to TCB of blocked task.                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSmutex_blockedTimeout(struct Mutex* mutex, struct Sch_Task* taskTCB);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/

#endif /* Conditional compile. */
#endif 
