/*************************************************************************/
/*  File Name:  sch_internal_IF.h                                        */
/*  Purpose:    Kernel access definitions and routines for scheduler.    */
/*  Created by: Garrett Sculthorpe on 5/20/2019                          */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*************************************************************************/

#ifndef sch_internal_IF_h 
#define sch_internal_IF_h

#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define SCH_MAX_NUM_TASKS                   (RTOS_CONFIG_MAX_NUM_TASKS + ONE)  
#define SCH_TASK_SLEEP_RESOURCE_MBOX        (SCH_TASK_WAKEUP_MBOX_READY)
#define SCH_TASK_SLEEP_RESOURCE_QUEUE       (SCH_TASK_WAKEUP_QUEUE_READY)
#define SCH_TASK_SLEEP_RESOURCE_SEMA        (SCH_TASK_WAKEUP_SEMA_READY)      
#define SCH_TASK_SLEEP_RESOURCE_FLAGS       (SCH_TASK_WAKEUP_FLAGS_EVENT)
#define SCH_TASK_SLEEP_RESOURCE_MUTEX       (SCH_TASK_WAKEUP_MUTEX_READY)
#define SCH_SET_PRIORITY_FAILED             (0)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
struct ListNode; /* Forward declaration. Defined in "listMgr_internal.h" */

typedef struct Sch_Task
{
  OS_STACK*  stackPtr;        /* Task stack pointer must be first entry in struct. */
  U1         priority;        /* Task priority. */
  U1         taskID;          /* Task ID. Task can be referenced via this number. */
  U1         flags;           /* Status flags used for scheduling. */
  U4         sleepCntr;       /* Sleep counter. Unit is scheduler ticks. */
  void*      resource;        /* If task is blocked on a resource, its address is stored here. */
  U1         wakeReason;      /* Stores code for reason task was most recently woken up. */
#if(RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT == RTOS_CONFIG_TRUE)
  OS_STACK*  topOfStack;      /* Pointer to stack watermark. Used to detect stack overflow. */
#endif
}
Sch_Task;

#if (RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
/* Used to calculate CPU load. */
typedef struct CPU_IdleCalc
{
  U1 CPU_idleAvg;
  U4 CPU_idleRunning;
  U1 CPU_idlePrevTimestamp;
}
CPU_IdleCalc;

/* More CPU run-time statistics can be added here. */
typedef struct OS_RunTimeStats
{
  CPU_IdleCalc  CPUIdlePercent;     
}
OS_RunTimeStats;
#endif


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: vd_OSsch_setReasonForWakeup                           */
/*  Purpose:       Set reason for wakeup to resource available. Called   */
/*                 internal to RTOS by other RTOS modules. It is expected*/
/*                 that OS internal modules will call taskWake() *After* */
/*                 this function call and maintain their own block lists.*/
/*  Arguments:     U1 reason:                                            */
/*                    Identifier code for wakeup reason.                 */
/*                 Sch_Task* wakeupTaskTCB:                              */
/*                    Pointer to task TCB that is being woken up which   */
/*                    was stored on resource blocked list.               */
/*  Return:        void                                                  */
/*************************************************************************/
void vd_OSsch_setReasonForWakeup(U1 reason, struct Sch_Task* wakeupTaskTCB);

/*************************************************************************/
/*  Function Name: vd_OSsch_setReasonForSleep                            */
/*  Purpose:       Set reason for task sleep according to mask.          */
/*  Arguments:     void* taskSleepResource:                              */
/*                       Address of resource task is blocked on.         */
/*  Return:        void                                                  */
/*************************************************************************/
void vd_OSsch_setReasonForSleep(void* taskSleepResource, U1 resourceType);

/*************************************************************************/
/*  Function Name: u1_OSsch_setNewPriority                               */
/*  Purpose:       Function to change task priority in support of        */
/*                 priority inheritance. Internal use only. Internal     */
/*                 module must ensure that no two active tasks share     */
/*                 the same priority value at any time.                  */
/*  Arguments:     Sch_Task* tcb:                                        */
/*                           Pointer to TCB to have priority changed.    */
/*                 U1 newPriority:                                       */
/*                                                                       */
/*  Return:        U1: Previous priority value.                          */
/*************************************************************************/
U1 u1_OSsch_setNewPriority(struct Sch_Task* tcb, U1 newPriority);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/
extern Sch_Task*        tcb_g_p_currentTaskBlock;                    /* Lets internal modules quickly dereference current task data. Should be used as read-only. */
extern struct ListNode* Node_s_ap_mapTaskIDToTCB[SCH_MAX_NUM_TASKS]; /* Lets internal modules quickly dereference TCB from task ID. */

/* THESE MACROS MUST BE USED AS READ-ONLY. MADE AVAILABLE FOR BLOCK LIST HANDLING BY RESOURCES. */
#define SCH_CURRENT_TCB_ADDR                (tcb_g_p_currentTaskBlock)                   /* Address of current task TCB.   */
#define SCH_CURRENT_TASK_ID                 ((U1)(tcb_g_p_currentTaskBlock->taskID))     /* ID of current task.            */
#define SCH_ID_TO_TCB(c)                    (Node_s_ap_mapTaskIDToTCB[c]->TCB)           /* Get TCB address from task ID.  */
#define SCH_CURRENT_TASK_PRIO               (tcb_g_p_currentTaskBlock->priority)         /* Get priority from TCB address. */
#define SCH_ID_TO_PRIO(c)                   (Node_s_ap_mapTaskIDToTCB[c]->TCB->priority) /* Get priority from task ID.     */

#endif 
