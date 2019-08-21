/*************************************************************************/
/*  File Name:  mutex_internal_IF.h                                      */
/*  Purpose:    Kernel access definitions and routines for mutex.        */
/*  Created by: Garrett Sculthorpe on 5/23/19                            */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
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
  struct ListNode  blockedTasks[MUTEX_MAX_NUM_BLOCKED];
  struct ListNode* blockedListHead;
}
BlockedTasks;

/* Priority management. */
typedef struct PrioInheritance
{
  U1               taskRealPrio;
  U1               taskInheritedPrio;
  struct Sch_Task* mutexHolder;
}
PrioInheritance;

typedef struct Mutex
{
  U1              lock;
  BlockedTasks    blockedTaskList;
  PrioInheritance priority;
}
Mutex;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_OSmutex_blockedTimeout(struct Mutex* mutex, struct Sch_Task* taskTCB);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/

#endif /* Conditional compile. */
#endif 
