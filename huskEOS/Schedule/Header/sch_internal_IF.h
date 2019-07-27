/*************************************************************************/
/*  File Name:  sch_internal_IF.h                                        */
/*  Purpose:    Kernel access definitions and routines for scheduler.    */
/*  Created by: Garrett Sculthorpe on 5/20/2019                          */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
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
#define SCH_TASK_SLEEP_RESOURCE_SEMA        (SCH_TASK_WAKEUP_SEMA_READY)      //how are these defined? just use the ones in sch.h
#define SCH_TASK_SLEEP_RESOURCE_FLAGS       (SCH_TASK_WAKEUP_FLAGS_EVENT)
  

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
struct ListNode; /* Forward declaration. Defined in "listMgr_internal.h" */

typedef struct TaskInfo //probs not needed
{
  U1 priority;
  U1 taskID;
}
TaskInfo;

typedef struct Sch_Task
{
  OS_STACK*  stackPtr; /* Task stack pointer must be first entry in struct. */
  TaskInfo   taskInfo;
  U1         flags;
  U4         sleepCntr;
  void*      resource;
  U1         wakeReason;
#if(RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT == RTOS_CONFIG_TRUE)
  OS_STACK*  topOfStack;
#endif
}
Sch_Task;

#if (RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
typedef struct
{
  U1 CPU_idleAvg;
  U4 CPU_idleRunning;
  U1 CPU_idlePrevTimestamp;
}
CPU_IdleCalc;

typedef struct
{
  CPU_IdleCalc  CPUIdlePercent;
}
OS_RunTimeStats;
#endif

/*************************************************************************/
/*  External References                                                  */
/*************************************************************************/  
extern struct ListNode* Node_s_ap_mapTaskIDToTCB[SCH_MAX_NUM_TASKS];

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: vd_sch_setReasonForWakeup                             */
/*  Purpose:       Set reason for wakeup to resource available. Called   */
/*                 internal to RTOS by other RTOS modules.               */
/*  Arguments:     U1 reason:                                            */
/*                    Identifier code for wakeup reason.                 */
/*                 Sch_Task* wakeupTaskTCB:                              */
/*                    Pointer to task TCB that is being woken up which   */
/*                    was stored on resource blocked list.               */
/*  Return:        void                                                  */
/*************************************************************************/
void vd_sch_setReasonForWakeup(U1 reason, Sch_Task* wakeupTaskTCB);

/*************************************************************************/
/*  Function Name: vd_sch_setReasonForSleep                              */
/*  Purpose:       Set reason for task sleep according to mask.          */
/*  Arguments:     void* taskSleepResource:                              */
/*                       Address of resource task is blocked on.         */
/*  Return:        void                                                  */
/*************************************************************************/
void vd_sch_setReasonForSleep(void* taskSleepResource, U1 resourceType);

/*************************************************************************/
/*  Function Name: tcb_OSsch_getCurrentTCB                               */
/*  Purpose:       Returns pointer to current TCB for block list storage.*/
/*  Arguments:     N/A                                                   */
/*  Return:        Sch_Task*: Current TCB address.                       */
/*************************************************************************/
Sch_Task* tcb_OSsch_getCurrentTCB(void);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
