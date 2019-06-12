/*************************************************************************/
/*  File Name:  sch.h                                                    */
/*  Purpose:    Header file for scheduler module.                        */
/*  Created by: Garrett Sculthorpe on 2/9/19.                            */
/*************************************************************************/

#ifndef __SCH_H__ 
#define __SCH_H__

#include "sch_internal_IF.h"


/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#if(RTOS_CONFIG_IDLE_BG_TASK == RTOS_CONFIG_TRUE) 
  #define SCH_MAX_NUM_TASKS                      (RTOS_CONFIG_MAX_NUM_TASKS + 1)
#else
  #define SCH_MAX_NUM_TASKS                      (RTOS_CONFIG_MAX_NUM_TASKS)
#endif

#define SCH_NUM_TASKS_ZERO                       (0)
#define SCH_TASK_CREATE_SUCCESS                  (1)
#define SCH_TASK_CREATE_DENIED                   (0)
#define SCH_TASK_FLAG_FALSE                      (0)
#define SCH_TASK_FLAG_TRUE                       (1)
#define SCH_BG_TASK_STACK_SIZE                   (RTOS_CONFIG_BG_TASK_STACK_SIZE) 

/* Task wakeup reasons  */
#define SCH_TASK_WAKEUP_SLEEP_TIMEOUT            (0x00)
#define SCH_TASK_NO_WAKEUP_SINCE_LAST_CHECK      (0xFF)
#define SCH_TASK_WAKEUP_MBOX_READY               (0x01)
#define SCH_TASK_WAKEUP_QUEUE_READY              (0x02)
#define SCH_TASK_WAKEUP_SEMA_READY               (0x03)
#define SCH_TASK_WAKEUP_FLAGS_EVENT              (0x04)


#define OS_SCH_ENTER_CRITICAL(void)            (OS_CPU_ENTER_CRITICAL(void))
#define OS_SCH_EXIT_CRITICAL(void)             (OS_CPU_EXIT_CRITICAL(void)) 
#define u1_OSsch_maskInterrupts(c)             (OS_CPU_MASK_SCHEDULER_TICK(c))
#define vd_OSsch_unmaskInterrupts(c)           (OS_CPU_UNMASK_SCHEDULER_TICK(c))

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_OS_init(U4 numMsPeriod);
U1 u1_OSsch_createTask(void (*newTaskFcn)(void), void* sp, U4 sizeOfStack);
void vd_OSsch_start(void);
U1   u1_OSsch_g_numTasks(void);
U4   u4_OSsch_getCurrentTickPeriodMs(void);
U1   u1_OSsch_getReasonForWakeup(void);
U4   u4_OSsch_getTicks(void);
U1   u1_OSsch_getCurrentTask(void);
void vd_OSsch_setNewTickPeriod(U4 numMsReload);
void vd_OSsch_taskYield(void);
void vd_OSsch_taskSleep(U4 period);
U4   u4_OSsch_taskSleepSetFreq(U4 nextWakeTime);
void vd_OSsch_taskWake(U1 taskIndex); 
void vd_OSsch_taskSuspend(U1 taskIndex);  
void vd_OSsch_suspendScheduler(void);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/

#endif /* End conditional declaration for sch.h */
