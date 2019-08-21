/*************************************************************************/
/*  File Name:  sch.h                                                    */
/*  Purpose:    Header file for scheduler module.                        */
/*  Created by: Garrett Sculthorpe on 2/9/19.                            */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*************************************************************************/

#ifndef sch_h 
#define sch_h

#include "cpu_os_interface.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define SCH_TASK_CREATE_SUCCESS                  (1)
#define SCH_TASK_CREATE_DENIED                   (0)
#define SCH_BG_TASK_STACK_SIZE                   (RTOS_CONFIG_BG_TASK_STACK_SIZE) 

/* Task wakeup reasons  */
#define SCH_TASK_WAKEUP_SLEEP_TIMEOUT            (0x00)
#define SCH_TASK_NO_WAKEUP_SINCE_LAST_CHECK      (0xFF)
#define SCH_TASK_WAKEUP_MBOX_READY               (0x01)
#define SCH_TASK_WAKEUP_QUEUE_READY              (0x02)
#define SCH_TASK_WAKEUP_SEMA_READY               (0x03)
#define SCH_TASK_WAKEUP_FLAGS_EVENT              (0x04)
#define SCH_TASK_WAKEUP_MUTEX_READY              (0x05)

#define OS_SCH_ENTER_CRITICAL(void)              (OS_CPU_ENTER_CRITICAL(void))
#define OS_SCH_EXIT_CRITICAL(void)               (OS_CPU_EXIT_CRITICAL(void)) 
#define u1_OSsch_maskInterrupts(c)               (OS_CPU_MASK_SCHEDULER_TICK(c))
#define vd_OSsch_unmaskInterrupts(c)             (OS_CPU_UNMASK_SCHEDULER_TICK(c))

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_OS_init                                            */
/*  Purpose:       Initialize scheduler module and configured RTOS       */
/*                 modules.                                              */
/*  Arguments:     U4 numMsPeriod:                                       */
/*                    Sets scheduler tick rate in milliseconds.          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OS_init(U4 numMsPeriod);

/*************************************************************************/
/*  Function Name: u1_OSsch_createTask                                   */
/*  Purpose:       Create new task in list.                              */
/*  Arguments:     void* newTaskFcn:                                     */
/*                       Function pointer to task routine.               */
/*                 void* sp:                                             */
/*                       Pointer to bottom of task stack (highest mem.   */
/*                       address).                                       */
/*                 U1 priority:                                          */
/*                       Unique priority level for task. 0 = highest.    */
/*                 U1 taskID:                                            */
/*                       Task ID to refer to task when using APIs (cannot*/
/*                       be changed).                                    */
/*                                                                       */
/*  Return:        SCH_TASK_CREATE_SUCCESS   OR                          */
/*                 SCH_TASK_CREATE_DENIED                                */
/*************************************************************************/
U1 u1_OSsch_createTask(void (*newTaskFcn)(void), void* sp, U4 sizeOfStack, U1 priority, U1 taskID);

/*************************************************************************/
/*  Function Name: vd_OSsch_start                                        */
/*  Purpose:       Give control to operating system. Starts with highest */
/*                 priority task.                                        */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_start(void);

/*************************************************************************/
/*  Function Name: vd_OSsch_interruptEnter                               */
/*  Purpose:       Must be called by ISRs external to OS at entry.       */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_interruptEnter(void);
  
/*************************************************************************/
/*  Function Name: u1_OSsch_g_numTasks                                   */
/*  Purpose:       Return current number of tasks seen by OS.            */
/*  Arguments:     N/A                                                   */
/*  Return:        Number of tasks: 0 - SCH_MAX_NUM_TASKS                */
/*************************************************************************/
U1 u1_OSsch_g_numTasks(void);

/*************************************************************************/
/*  Function Name: u4_OSsch_getCurrentTickPeriodMs                       */
/*  Purpose:       Get current period in ms for scheduler tick.          */
/*  Arguments:     N/A                                                   */
/*  Return:        Tick rate in milliseconds.                            */
/*************************************************************************/
U4 u4_OSsch_getCurrentTickPeriodMs(void);

/*************************************************************************/
/*  Function Name: u1_OSsch_getReasonForWakeup                           */
/*  Purpose:       Get most recent reason that current task has woken up.*/
/*  Arguments:     N/A                                                   */
/*  Return:        SCH_TASK_WAKEUP_SLEEP_TIMEOUT          OR             */
/*                 SCH_TASK_NO_WAKEUP_SINCE_LAST_CHECK    OR             */
/*                 SCH_TASK_WAKEUP_MBOX_READY             OR             */
/*                 SCH_TASK_WAKEUP_QUEUE_READY            OR             */
/*                 SCH_TASK_WAKEUP_SEMA_READY             OR             */
/*                 SCH_TASK_WAKEUP_FLAGS_EVENT            OR             */
/*                 OS flags event that triggered wakeup                  */
/*************************************************************************/
U1 u1_OSsch_getReasonForWakeup(void);

/*************************************************************************/
/*  Function Name: u4_OSsch_getTicks                                     */
/*  Purpose:       Get number of ticks from scheduler. Overflows to zero.*/
/*  Arguments:     N/A                                                   */
/*  Return:        Number of scheduler ticks.                            */
/*************************************************************************/
U4 u4_OSsch_getTicks(void);

/*************************************************************************/
/*  Function Name: u1_OSsch_getCurrentTaskID                             */
/*  Purpose:       Returns current task ID.                              */
/*  Arguments:     N/A                                                   */
/*  Return:        U1: Current task ID number.                           */
/*************************************************************************/
U1 u1_OSsch_getCurrentTaskID(void);
  
/*************************************************************************/
/*  Function Name: u1_OSsch_getCurrentTaskPrio                           */
/*  Purpose:       Returns current task priority.                        */
/*  Arguments:     N/A                                                   */
/*  Return:        U1: Current task priority.                            */
/*************************************************************************/
U1 u1_OSsch_getCurrentTaskPrio(void);

/*************************************************************************/
/*  Function Name: u1_OSsch_getCurrentTaskID                             */
/*  Purpose:       Returns current task ID number.                       */
/*  Arguments:     N/A                                                   */
/*  Return:        U1: Current task ID.                                  */
/*************************************************************************/
U1 u1_OSsch_getCurrentTaskID(void);

/*************************************************************************/
/*  Function Name: vd_OSsch_setNewTickPeriod                             */
/*  Purpose:       Set new tick period in milliseconds.                  */
/*  Arguments:     U4 numMsReload:                                       */
/*                    Period of interrupts.                              */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_setNewTickPeriod(U4 numMsReload);

/*************************************************************************/
/*  Function Name: vd_OSsch_taskSleep                                    */
/*  Purpose:       Suspend current task for a specified amount of time.  */
/*  Arguments:     U4 u4_period:                                         */
/*                    Time units to suspend for.                         */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_taskSleep(U4 period);

/*************************************************************************/
/*  Function Name: u4_OSsch_taskSleepSetFreq                             */
/*  Purpose:       Used to set task to sleep such that task will run at a*/
/*                 set frequency.                                        */
/*  Arguments:     U4 nextWakeTime:                                      */
/*                    Time to wake up at (in ticks).                     */
/*  Return:        U4 u4_t_wakeTime:                                     */
/*                    Tick value that task was most recently woken at.   */
/*************************************************************************/
U4 u4_OSsch_taskSleepSetFreq(U4 nextWakeTime);

/*************************************************************************/
/*  Function Name: vd_OSsch_taskWake                                     */
/*  Purpose:       Wake specified task from sleep or suspended state.    */
/*  Arguments:     U1 taskID:                                            */
/*                    Task ID to be woken from sleep or suspend state.   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_taskWake(U1 taskID); 

/*************************************************************************/
/*  Function Name: vd_OSsch_taskSuspend                                  */
/*  Purpose:       Suspend current task for a specified amount of time.  */
/*  Arguments:     U1 taskIndex:                                         */
/*                    Task ID to be suspended.                           */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_taskSuspend(U1 taskIndex);  

/*************************************************************************/
/*  Function Name: vd_OSsch_suspendScheduler                             */
/*  Purpose:       Turn off scheduler interrupts and reset ticker.       */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_suspendScheduler(void);

/*************************************************************************/
/*  Function Name: u1_OSsch_getCPULoad                                   */
/*  Purpose:       Returns CPU load averaged over 100 ticks.             */
/*  Arguments:     N/A                                                   */
/*  Return:        U1: CPU load as a percentage.                         */
/*************************************************************************/
#if (RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
U1 u1_OSsch_getCPULoad(void);
#endif

/*************************************************************************/
/*  Function Name: app_OSPreSleepFcn                                     */
/*  Purpose:       Hook function. Will run before CPU put to sleep.      */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
#if(RTOS_CONFIG_PRESLEEP_FUNC == RTOS_CONFIG_TRUE)
void app_OSPreSleepFcn(void);
#endif

/*************************************************************************/
/*  Function Name: app_OSPostSleepFcn                                    */
/*  Purpose:       Hook function. Will run after CPU woken from sleep.   */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
#if(RTOS_CONFIG_POSTSLEEP_FUNC == RTOS_CONFIG_TRUE)
void app_OSPostSleepFcn(void);
#endif

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/

#endif 
