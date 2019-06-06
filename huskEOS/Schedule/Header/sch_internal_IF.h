/*************************************************************************/
/*  File Name:  sch_internal_IF.h                                        */
/*  Purpose:    Kernel access definitions and routines for scheduler.    */
/*  Created by: Garrett Sculthorpe on 5/20/2019                          */
/*************************************************************************/

#ifndef sch_internal_IF_h /* Protection from declaring more than once */
#define sch_internal_IF_h

#include "cpu_defs.h"
#include "rtos_cfg.h"
#include "cpu_os_interface.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define SCH_TASK_SLEEP_RESOURCE_MBOX        (SCH_TASK_WAKEUP_MBOX_READY)
#define SCH_TASK_SLEEP_RESOURCE_QUEUE       (SCH_TASK_WAKEUP_QUEUE_READY)
#define SCH_TASK_SLEEP_RESOURCE_SEMA        (SCH_TASK_WAKEUP_SEMA_READY)
#define SCH_TASK_SLEEP_RESOURCE_FLAGS       (SCH_TASK_WAKEUP_FLAGS_EVENT)
  
/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct 
{
  void*      stackPtr; /* Task stack pointer must be first entry in struct. */
  U1         flags;
  U4         sleepCntr;
  void*      resource;
  U1         wakeReason;
}
Sch_Task;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_sch_setReasonForWakeup(U1 reason, U1 wakeupTaskID);
void vd_sch_setReasonForSleep(void* taskSleepResource, U1 resourceType);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif /* End conditional declaration for sch_internal_IF_h */