/*************************************************************************/
/*  File Name: sch.c                                                     */
/*  Purpose: Init and routines for scheduler module and task handling.   */
/*  Created by: Garrett Sculthorpe on 2/29/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/

//testing

#include <stdint.h>
#include <stdbool.h>
#include "sysctl.h"
#include "gpio.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "hw_types.h"
#include "pin_map.h"
#include "uart.h"
#include "uartstdio.h"





#include "cpu_defs.h"
#include "rtos_cfg.h"
#include "sch_internal_IF.h"
#include "listMgr_internal.h"
#include "sch.h"

#if(RTOS_CFG_OS_MAILBOX_ENABLED == RTOS_CONFIG_TRUE)
#include "mbox_internal_IF.h"     
#endif

#if(RTOS_CFG_OS_QUEUE_ENABLED == RTOS_CONFIG_TRUE)
#include "queue_internal_IF.h"
#endif

#if(RTOS_CFG_OS_SEMAPHORE_ENABLED == RTOS_CONFIG_TRUE)
#include "semaphore_internal_IF.h"
#endif

#if(RTOS_CFG_OS_FLAGS_ENABLED == RTOS_CONFIG_TRUE)
#include "flags_internal_IF.h"
#endif

/*************************************************************************/
/*  External References                                                  */
/*************************************************************************/
extern void WaitForInterrupt(void);
extern void OSTaskFault(void);

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define RTOS_RESOURCES_CONFIGURED                (RTOS_CFG_OS_MAILBOX_ENABLED   | \
                                                  RTOS_CFG_OS_QUEUE_ENABLED     | \
                                                  RTOS_CFG_OS_SEMAPHORE_ENABLED | \
                                                  RTOS_CFG_OS_FLAGS_ENABLED)
                                                  
#define SCH_NUM_TASKS_ZERO                       (0)
#define SCH_TRUE                                 (1)
#define SCH_FALSE                                (0)
#define SCH_TCB_PTR_INIT                         (NULL)
#define SCH_TASK_FLAG_STS_SLEEP                  (0x10)
#define SCH_TASK_FLAG_STS_SUSPENDED              (0x20) 
#define SCH_TASK_FLAG_SLEEP_MBOX                 (SCH_TASK_WAKEUP_MBOX_READY)
#define SCH_TASK_FLAG_SLEEP_QUEUE                (SCH_TASK_WAKEUP_QUEUE_READY)
#define SCH_TASK_FLAG_SLEEP_SEMA                 (SCH_TASK_WAKEUP_SEMA_READY)
#define SCH_TASK_FLAG_SLEEP_FLAGS                (SCH_TASK_WAKEUP_FLAGS_EVENT)
#define SCH_TASK_FLAG_STS_CHECK                  (SCH_TASK_FLAG_STS_SLEEP | SCH_TASK_FLAG_STS_SUSPENDED) 
#define SCH_TASK_RESOURCE_SLEEP_CHECK_MASK       (SCH_TASK_FLAG_SLEEP_MBOX | SCH_TASK_FLAG_SLEEP_QUEUE | SCH_TASK_FLAG_SLEEP_SEMA | SCH_TASK_FLAG_SLEEP_FLAGS)
#define SCH_TOP_OF_STACK_MARK                    (0xF0F0F0F0)
#define SCH_ONE_HUNDRED_PERCENT                  (100)
#define SCH_HUNDRED_TICKS                        (100)
#define SCH_CPU_NOT_SLEEPING                     (SCH_FALSE)
#define SCH_CPU_SLEEPING                         (SCH_TRUE)
#define SCH_FIRST_TCB_NODE                       (ZERO)
#define SCH_TASK_PRIORITY_UNDEFINED              (0xFF)
#define SCH_TASK_LOWEST_PRIORITY                 (0xF0)
#define SCH_INVALID_TASK_ID                      (0xFF)
#define SCH_BG_TASK_ID                           (0x00)


/*************************************************************************/
/*  Static Global Variables, Constants                                   */
/*************************************************************************/
static U1 u1_s_numTasks;
static U4 u4_s_tickCntr;
static U1 u1_s_sleepState;
static ListNode* node_s_p_headOfWaitList;
static ListNode* node_s_p_headOfReadyList;

#if (RTOS_CONFIG_BG_TASK == RTOS_CONFIG_TRUE)
static OS_STACK u4_backgroundStack[SCH_BG_TASK_STACK_SIZE];
#endif

/* Allocate memory for data structures used for TCBs and scheduling queues */
static ListNode   Node_s_as_listAllTasks[SCH_MAX_NUM_TASKS];     
static Sch_Task   SchTask_s_as_taskList[SCH_MAX_NUM_TASKS];
static ListNode*  Node_s_ap_mapTaskIDToTCB[SCH_MAX_NUM_TASKS];

#if (RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
static OS_RunTimeStats OS_s_cpuData;
#endif

/* Note: These global variables are modified by asm routine */
Sch_Task* tcb_g_p_currentTaskBlock;
Sch_Task* tcb_g_p_nextTaskBlock;


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
#if (RTOS_CONFIG_BG_TASK == RTOS_CONFIG_TRUE)
static void vd_sch_background(void);
#endif

#if(RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT == RTOS_CONFIG_TRUE)
static U1 u1_sch_checkStack(U1 taskIndex);
#endif

static void vd_sch_periodicScheduler(void);
static void vd_sch_setNextReadyTaskToRun(void);

/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_OS_init                                            */
/*  Purpose:       Initialize scheduler module and configured RTOS       */
/*                 modules.                                              */
/*  Arguments:     U4 numMsPeriod:                                       */
/*                    Sets scheduler tick rate in milliseconds.          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OS_init(U4 numMsPeriod)
{
  U1 u1_t_index;
  
  u1_s_numTasks      = (U1)SCH_NUM_TASKS_ZERO;
  u4_s_tickCntr      = (U1)ZERO;
  u1_s_sleepState    = (U1)SCH_CPU_NOT_SLEEPING;
  
  /* Initialize task and queue data to default values */
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)SCH_MAX_NUM_TASKS; u1_t_index++)
  {
    SchTask_s_as_taskList[u1_t_index].stackPtr          = (OS_STACK*)NULL;
    SchTask_s_as_taskList[u1_t_index].flags             = (U1)ZERO;
    SchTask_s_as_taskList[u1_t_index].sleepCntr         = (U4)ZERO;
    SchTask_s_as_taskList[u1_t_index].resource          = (void*)NULL;
    SchTask_s_as_taskList[u1_t_index].wakeReason        = (U1)ZERO;
    SchTask_s_as_taskList[u1_t_index].taskInfo.priority = (U1)SCH_TASK_PRIORITY_UNDEFINED;
    SchTask_s_as_taskList[u1_t_index].taskInfo.taskID   = (U1)SCH_INVALID_TASK_ID;
    
    Node_s_ap_mapTaskIDToTCB[u1_t_index]                = (ListNode*)NULL;
    
    Node_s_as_listAllTasks[u1_t_index].nextNode         = (ListNode*)NULL;
    Node_s_as_listAllTasks[u1_t_index].previousNode     = (ListNode*)NULL;
    Node_s_as_listAllTasks[u1_t_index].TCB              = (Sch_Task*)NULL;
  }
  
  /* Initialize linked list head pointers */
  node_s_p_headOfWaitList   = (ListNode*)NULL;
  node_s_p_headOfReadyList  = (ListNode*)NULL;
  
  /* Initialize running task pointer */
  tcb_g_p_currentTaskBlock = (Sch_Task*)SCH_TCB_PTR_INIT;
  
#if (RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
  OS_s_cpuData.CPUIdlePercent.CPU_idleAvg           = (U1)ZERO;
  OS_s_cpuData.CPUIdlePercent.CPU_idlePrevTimestamp = (U1)ZERO;
  OS_s_cpuData.CPUIdlePercent.CPU_idleRunning       = (U4)ZERO;
#endif
  
  /* Create background task */
  u1_OSsch_createTask(&vd_sch_background, 
                      &u4_backgroundStack[SCH_BG_TASK_STACK_SIZE - ONE], 
                      (U4)RTOS_CONFIG_BG_TASK_STACK_SIZE, 
                      (U1)SCH_TASK_LOWEST_PRIORITY, 
                      (U1)SCH_BG_TASK_ID);
  
  /* Mask interrupts until RTOS enters normal operation */
  vd_cpu_disableInterruptsOSStart();
  vd_cpu_init(numMsPeriod);
 
#if(RTOS_CFG_OS_MAILBOX_ENABLED == RTOS_CONFIG_TRUE)  
  vd_mbox_init();
#endif

#if(RTOS_CFG_OS_QUEUE_ENABLED == RTOS_CONFIG_TRUE)  
  vd_queue_init();
#endif 
}

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
U1 u1_OSsch_createTask(void (*newTaskFcn)(void), void* sp, U4 sizeOfStack, U1 priority, U1 taskID)
{
  U1 u1_t_returnSts;
  
  if(u1_s_numTasks >= (U1)SCH_MAX_NUM_TASKS)
  {
    u1_t_returnSts = (U1)SCH_TASK_CREATE_DENIED;
  }
  else if(Node_s_ap_mapTaskIDToTCB[taskID] != (void*)NULL)
  {
    u1_t_returnSts = (U1)SCH_TASK_CREATE_DENIED;
  }
  else
  {
    /* Map user configured task ID to the actual TCB location for later queries by user */
    Node_s_ap_mapTaskIDToTCB[taskID] = &Node_s_as_listAllTasks[u1_s_numTasks];
    
    /* Set new task stack pointers */
#if(RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT == RTOS_CONFIG_TRUE)   
  #if(STACK_GROWTH == STACK_DESCENDING)
    SchTask_s_as_taskList[u1_s_numTasks].topOfStack = ((OS_STACK*)sp - sizeOfStack + ONE);
  #elif(STACK_GROWTH == STACK_ASCENDING)
    SchTask_s_as_taskList[u1_s_numTasks].topOfStack = ((OS_STACK*)sp + sizeOfStack - ONE);
  #else 
    #error "STACK DIRECTION NOT PROPERLY DEFINED"
  #endif /* STACK_GROWTH */
    *SchTask_s_as_taskList[u1_s_numTasks].topOfStack = (OS_STACK)SCH_TOP_OF_STACK_MARK;
#endif /* RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT */     
    SchTask_s_as_taskList[u1_s_numTasks].stackPtr    =  sp_cpu_taskStackInit(newTaskFcn, (OS_STACK*)sp);
    
    /* Set new task priority, ID */
    SchTask_s_as_taskList[u1_s_numTasks].taskInfo.priority = priority;
    SchTask_s_as_taskList[u1_s_numTasks].taskInfo.taskID   = taskID;
    
    /* Set new linked list node content to newly formed TCB */
    Node_s_as_listAllTasks[u1_s_numTasks].TCB = &SchTask_s_as_taskList[u1_s_numTasks];
    
    /* Put new task into ready queue sorted by priority */
    vd_list_addTaskByPrio(&node_s_p_headOfReadyList, &Node_s_as_listAllTasks[u1_s_numTasks]);
    
    /* Increment number of tasks */
    ++u1_s_numTasks;
    
    u1_t_returnSts = (U1)SCH_TASK_CREATE_SUCCESS;
  }
  
  return (u1_t_returnSts);
}

/*************************************************************************/
/*  Function Name: vd_OSsch_start                                        */
/*  Purpose:       Give control to operating system.                     */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_start(void)
{
  /* Start at highest priority task */
  tcb_g_p_nextTaskBlock = node_s_p_headOfReadyList->TCB;
  
  vd_cpu_enableInterruptsOSStart();
  OS_CPU_TRIGGER_DISPATCHER();
}

/*************************************************************************/
/*  Function Name: vd_OSsch_interruptEnter                               */
/*  Purpose:       Must be called by ISRs external to OS at entry.       */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_interruptEnter(void)
{
#if(RTOS_CONFIG_PRESLEEP_FUNC == RTOS_CONFIG_TRUE)
  if(u1_s_sleepState == (U1)SCH_CPU_SLEEPING)
  {  
    /* pre-sleep hook function defined by application */
    app_OSPostSleepFcn();
    
    u1_s_sleepState = (U1)SCH_CPU_NOT_SLEEPING;
  }
#endif
}

/*************************************************************************/
/*  Function Name: u1_OSsch_g_numTasks                                   */
/*  Purpose:       Return current number of scheduled tasks.             */
/*  Arguments:     N/A                                                   */
/*  Return:        Number of tasks: 0 - SCH_MAX_NUM_TASKS                */
/*************************************************************************/
U1 u1_OSsch_g_numTasks(void)
{
  return (u1_s_numTasks);
}

/*************************************************************************/
/*  Function Name: u4_OSsch_getCurrentTickPeriodMs                       */
/*  Purpose:       Get current period in ms for scheduler tick.          */
/*  Arguments:     N/A                                                   */
/*  Return:        Tick rate in milliseconds.                            */
/*************************************************************************/
U4 u4_OSsch_getCurrentTickPeriodMs(void)
{
  return (u4_cpu_getCurrentMsPeriod());
}

/*************************************************************************/
/*  Function Name: u1_OSsch_getReasonForWakeup                           */
/*  Purpose:       Get most recent reason that current task has woken up.*/
/*  Arguments:     N/A                                                   */
/*  Return:        SCH_TASK_WAKEUP_SLEEP_TIMEOUT          OR             */
/*                 SCH_TASK_NO_WAKEUP_SINCE_LAST_CHECK    OR             */
/*                 SCH_TASK_WAKEUP_MBOX_READY             OR             */
/*                 SCH_TASK_WAKEUP_QUEUE_READY            OR             */
/*                 SCH_TASK_WAKEUP_SEMA_READY             OR             */
/*                 SCH_TASK_WAKEUP_FLAGS_EVENT                           */
/*************************************************************************/
U1 u1_OSsch_getReasonForWakeup(void)
{
  U1 u1_t_reason;
  
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  OS_CPU_ENTER_CRITICAL();
  
  u1_t_reason = tcb_g_p_currentTaskBlock->wakeReason;
  tcb_g_p_currentTaskBlock->wakeReason = (U1)SCH_TASK_NO_WAKEUP_SINCE_LAST_CHECK;
  
  /* Resume tick interrupts and enable context switch interrupt. */
  OS_CPU_EXIT_CRITICAL();
  
  return(u1_t_reason);
}

/*************************************************************************/
/*  Function Name: u4_OSsch_getTicks                                     */
/*  Purpose:       Get number of ticks from scheduler. Overflows to zero.*/
/*  Arguments:     N/A                                                   */
/*  Return:        U4 u4_s_tickCntr                                      */
/*************************************************************************/
U4 u4_OSsch_getTicks(void)
{
  return(u4_s_tickCntr);
}

/*************************************************************************/
/*  Function Name: u1_OSsch_getCurrentTaskPrio                           */
/*  Purpose:       Returns current task priority.                        */
/*  Arguments:     N/A                                                   */
/*  Return:        U1: Current task priority.                            */
/*************************************************************************/
U1 u1_OSsch_getCurrentTaskPrio(void)
{
  return(tcb_g_p_currentTaskBlock->taskInfo.priority);
}

/*************************************************************************/
/*  Function Name: u1_OSsch_getCurrentTaskID                             */
/*  Purpose:       Returns current task ID number.                       */
/*  Arguments:     N/A                                                   */
/*  Return:        U1: Current task ID.                                  */
/*************************************************************************/
U1 u1_OSsch_getCurrentTaskID(void)
{
  return(tcb_g_p_currentTaskBlock->taskInfo.taskID);
}
  
/*************************************************************************/
/*  Function Name: tcb_OSsch_getCurrentTCB                               */
/*  Purpose:       Returns pointer to current TCB for block list storage.*/
/*  Arguments:     N/A                                                   */
/*  Return:        Sch_Task*: Current TCB address.                       */
/*************************************************************************/
Sch_Task* tcb_OSsch_getCurrentTCB(void)
{
  return(tcb_g_p_currentTaskBlock);
}

/*************************************************************************/
/*  Function Name: u1_OSsch_getCPULoad                                   */
/*  Purpose:       Returns CPU load averaged over 100 ticks.             */
/*  Arguments:     N/A                                                   */
/*  Return:        U1: CPU load as a percentage.                         */
/*************************************************************************/
#if (RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
U1 u1_OSsch_getCPULoad(void)
{
  return ((U1)SCH_ONE_HUNDRED_PERCENT - OS_s_cpuData.CPUIdlePercent.CPU_idleAvg);
}
#endif

/*************************************************************************/
/*  Function Name: vd_OSsch_setNewTickPeriod                             */
/*  Purpose:       Set new tick period in milliseconds.                  */
/*  Arguments:     U4 numMsReload:                                       */
/*                    Period of interrupts.                              */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_setNewTickPeriod(U4 numMsReload)
{
  vd_cpu_setNewSchedPeriod(numMsReload);
}

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
void vd_sch_setReasonForWakeup(U1 reason, Sch_Task* wakeupTaskTCB)
{
  OS_CPU_ENTER_CRITICAL();
  
  wakeupTaskTCB->resource   = (void*)NULL;
  wakeupTaskTCB->flags     &= ~((U1)reason);
  wakeupTaskTCB->wakeReason = reason;
  
  vd_OSsch_taskWake(wakeupTaskTCB->taskInfo.taskID);
  
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_sch_setReasonForSleep                              */
/*  Purpose:       Set reason for task sleep according to mask.          */
/*  Arguments:     void* taskSleepResource:                              */
/*                       Address of resource task is blocked on.         */
/*  Return:        void                                                  */
/*************************************************************************/
void vd_sch_setReasonForSleep(void* taskSleepResource, U1 resourceType)
{
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  OS_CPU_ENTER_CRITICAL();
  
  tcb_g_p_currentTaskBlock->resource = taskSleepResource;
  tcb_g_p_currentTaskBlock->flags   |= (U1)resourceType;
  
  /* Resume tick interrupts and enable context switch interrupt. */
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_OSsch_taskSleep                                    */
/*  Purpose:       Suspend current task for a specified amount of time.  */
/*  Arguments:     U4 u4_period:                                         */
/*                    Time units to suspend for.                         */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_taskSleep(U4 period)
{

  //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  OS_CPU_ENTER_CRITICAL();
  
  tcb_g_p_currentTaskBlock->sleepCntr = period; 
  tcb_g_p_currentTaskBlock->flags    |= (U1)SCH_TASK_FLAG_STS_SLEEP;
  
  /* Switch to an active task */
  vd_sch_setNextReadyTaskToRun();
  OS_CPU_TRIGGER_DISPATCHER();
  //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
  /* Resume tick interrupts and enable context switch interrupt. */
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: u4_OSsch_taskSleepSetFreq                             */
/*  Purpose:       Used to set task to sleep such that task will run at a*/
/*                 set frequency.                                        */
/*  Arguments:     U4 nextWakeTime:                                      */
/*                    Time to wake up at (in ticks).                     */
/*  Return:        U4 u4_t_wakeTime:                                     */
/*                    Tick value that task was most recently woken at.   */
/*************************************************************************/
U4 u4_OSsch_taskSleepSetFreq(U4 nextWakeTime)
{
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  OS_CPU_ENTER_CRITICAL();

  tcb_g_p_currentTaskBlock->sleepCntr = nextWakeTime - u4_s_tickCntr; 
  tcb_g_p_currentTaskBlock->flags    |= (U1)SCH_TASK_FLAG_STS_SLEEP;
  
  /* Switch to an active task */
  vd_sch_setNextReadyTaskToRun();
  OS_CPU_TRIGGER_DISPATCHER();
  
  /* Resume tick interrupts and enable context switch interrupt. */
  OS_CPU_EXIT_CRITICAL();

  return(u4_s_tickCntr);  
}

/*************************************************************************/
/*  Function Name: vd_OSsch_taskWake                                     */
/*  Purpose:       Wake specified task from sleep or suspended state.    */
/*  Arguments:     U1 taskIndex:                                         */
/*                    Task ID to be woken from sleep or suspend state.   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_taskWake(U1 taskIndex)
{  
  OS_CPU_ENTER_CRITICAL();
  
  Node_s_ap_mapTaskIDToTCB[taskIndex]->TCB->sleepCntr  =   (U4)ZERO; 
  Node_s_ap_mapTaskIDToTCB[taskIndex]->TCB->flags     &= ~((U1)(SCH_TASK_FLAG_STS_SLEEP|SCH_TASK_FLAG_STS_SUSPENDED));
  
#if(RTOS_CONFIG_BG_TASK == RTOS_CONFIG_TRUE && RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
  if(tcb_g_p_currentTaskBlock == (Node_s_ap_mapTaskIDToTCB[u1_s_numTasks - ONE].TCBNode->TCB))
  {
    OS_s_cpuData.CPUIdlePercent.CPU_idleRunning += (u1_cpu_getPercentOfTick() - OS_s_cpuData.CPUIdlePercent.CPU_idlePrevTimestamp);
  }
#endif
  
  /* Remove task from wait list */
  vd_list_removeNode(&node_s_p_headOfWaitList, Node_s_ap_mapTaskIDToTCB[taskIndex]); 
  
  /* Add woken task to ready queue */
  vd_list_addTaskByPrio(&node_s_p_headOfReadyList, Node_s_ap_mapTaskIDToTCB[taskIndex]);
  
  /* Is woken up task higher priority than current task ? */
  if(node_s_p_headOfReadyList->TCB != tcb_g_p_currentTaskBlock)
  { 
    /* Set global task pointer to new task control block */
    tcb_g_p_nextTaskBlock = node_s_p_headOfReadyList->TCB;
    
    OS_CPU_TRIGGER_DISPATCHER();
  }
  
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_OSsch_taskSleep                                    */
/*  Purpose:       Suspend current task for a specified amount of time.  */
/*  Arguments:     U1 taskIndex:                                         */
/*                    Task ID to be suspended.                           */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_taskSuspend(U1 taskIndex)
{
  ListNode* node_t_p_suspendTask;
  
  OS_CPU_ENTER_CRITICAL();
  
  node_t_p_suspendTask = Node_s_ap_mapTaskIDToTCB[taskIndex];
  
  if(Node_s_ap_mapTaskIDToTCB[taskIndex]->TCB->flags != (U1)SCH_TASK_FLAG_STS_SUSPENDED)
  {
    Node_s_ap_mapTaskIDToTCB[taskIndex]->TCB->flags |= (U1)SCH_TASK_FLAG_STS_SUSPENDED;
    vd_list_removeNode(&node_s_p_headOfWaitList, node_t_p_suspendTask); 
    vd_list_addNodeToEnd(&node_s_p_headOfWaitList, node_t_p_suspendTask);   //need to pass by reference not pointer 
  }

  /* Is task suspending itself or another task? */
  if(Node_s_ap_mapTaskIDToTCB[taskIndex]->TCB == tcb_g_p_currentTaskBlock)
  {
    /* Switch to an active task */
    vd_sch_setNextReadyTaskToRun();
    OS_CPU_TRIGGER_DISPATCHER();
  }
  /* Resume tick interrupts and enable context switch interrupt. */
  OS_CPU_ENTER_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_OSsch_suspendScheduler                             */
/*  Purpose:       Turn off scheduler interrupts and reset ticker.       */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_suspendScheduler(void)
{
  vd_cpu_suspendScheduler();
}

/*************************************************************************/
/*  Function Name: SysTick_Handler                                       */
/*  Purpose:       Set ticker flag and run the scheduler.                */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
__irq void SysTick_Handler(void)
{
  //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  OS_CPU_ENTER_CRITICAL();

  ++u4_s_tickCntr;
  
#if(RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)

#if(RTOS_CONFIG_BG_TASK == RTOS_CONFIG_TRUE)
  if(tcb_g_p_currentTaskBlock == (Node_s_ap_mapTaskIDToTCB[ZERO].TCBNode->TCB))
  {
    OS_s_cpuData.CPUIdlePercent.CPU_idleRunning += ((U1)SCH_ONE_HUNDRED_PERCENT - OS_s_cpuData.CPUIdlePercent.CPU_idlePrevTimestamp);
  }
  
  if(!(u4_s_tickCntr % SCH_HUNDRED_TICKS))
  {
    OS_s_cpuData.CPUIdlePercent.CPU_idleAvg     = (U1)(OS_s_cpuData.CPUIdlePercent.CPU_idleRunning/(U4)SCH_HUNDRED_TICKS);
    OS_s_cpuData.CPUIdlePercent.CPU_idleRunning = (U1)ZERO;
  }
  
  OS_s_cpuData.CPUIdlePercent.CPU_idlePrevTimestamp = (U1)ZERO;
#endif
  
#endif  
  
  vd_sch_periodicScheduler();
 // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
  /* Resume tick interrupts and enable context switch interrupt. */
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_sch_setNextReadyTaskToRun                          */
/*  Purpose:       Select next task to run.                              */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_sch_setNextReadyTaskToRun(void)
{
  ListNode* node_t_p_moveToWaitList;

  /* Remove node of task that was previously executing */
  node_t_p_moveToWaitList = node_list_removeFirstNode(&node_s_p_headOfReadyList);
  
  /* Move previous node to wait list */
  vd_list_addNodeToFront(&node_s_p_headOfWaitList, node_t_p_moveToWaitList);
  
  tcb_g_p_nextTaskBlock = node_s_p_headOfReadyList->TCB;
}

/*************************************************************************/
/*  Function Name: vd_sch_periodicScheduler                              */
/*  Purpose:       Scheduler algorithm called to check if task is ready. */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_sch_periodicScheduler(void)
{
  ListNode* node_t_p_check;
  ListNode* node_t_p_changeListNode;
  Sch_Task* tcb_t_p_currentTCB;
  
  if(node_s_p_headOfWaitList == NULL)
  {
    /* No tasks to process. Return immediately. */  
  }
  else
  {
    node_t_p_check = node_s_p_headOfWaitList;
    
    /* Check each node that is not suspended. Suspended tasks always held at end of waiting list */
    while(node_t_p_check != (ListNode*)NULL)
    {
      tcb_t_p_currentTCB = node_t_p_check->TCB;
      
      if(tcb_t_p_currentTCB->flags & (U1)SCH_TASK_FLAG_STS_SUSPENDED)
      {
        /* All waiting tasks have been checked */
        break;
      }
      /* Decrement sleep counter and check if zero */
      else if((--(tcb_t_p_currentTCB->sleepCntr) == (U1)ZERO))
      {  
#if(RTOS_RESOURCES_CONFIGURED)          
        /* Task has timed out. Determine if due to manual sleep or resource */
        switch(tcb_t_p_currentTCB->flags & (U1)SCH_TASK_RESOURCE_SLEEP_CHECK_MASK)
        {
          /* Remove task from resource's blocked list */
  #if(RTOS_CFG_OS_MAILBOX_ENABLED == RTOS_CONFIG_TRUE)
          case (U1)SCH_TASK_FLAG_SLEEP_MBOX:
            vd_mbox_blockedTaskTimeout((Mbox*)tcb_t_p_currentTCB->resource);
            tcb_t_p_currentTCB->resource = (void*)NULL;
            break;
  #endif          
  #if(RTOS_CFG_OS_QUEUE_ENABLED == RTOS_CONFIG_TRUE)            
          case (U1)SCH_TASK_FLAG_SLEEP_QUEUE:
            vd_queue_blockedTaskTimeout((Queue*)tcb_t_p_currentTCB->resource, tcb_t_p_currentTCB->taskInfo.taskID);
            tcb_t_p_currentTCB->resource = (void*)NULL;
            break;
  #endif   
  #if(RTOS_CFG_OS_SEMAPHORE_ENABLED == RTOS_CONFIG_TRUE)            
          case (U1)SCH_TASK_FLAG_SLEEP_SEMA:
            vd_sema_blockedTimeout((Semaphore*)tcb_t_p_currentTCB->resource, tcb_t_p_currentTCB);
            tcb_t_p_currentTCB->resource = (void*)NULL;
            break;
  #endif         
  #if(RTOS_CFG_OS_FLAGS_ENABLED == RTOS_CONFIG_TRUE)            
          case (U1)SCH_TASK_FLAG_SLEEP_FLAGS:
            vd_flags_pendTimeout(tcb_t_p_currentTCB->resource);
            tcb_t_p_currentTCB->resource = (void*)NULL;
            break;
  #endif            
          case (U1)ZERO:
            /* Manual sleep time out */
            break;  

          default:
            OSTaskFault(); /* If code execution reaches this point it is a bug */
            break;             
        }          
#endif /* Resources configured */
        
        /* Update flags and wake reason to TIMEOUT */
        tcb_t_p_currentTCB->wakeReason = (U1)SCH_TASK_WAKEUP_SLEEP_TIMEOUT;
        tcb_t_p_currentTCB->flags     &= ~((U1)(SCH_TASK_FLAG_STS_SLEEP | SCH_TASK_RESOURCE_SLEEP_CHECK_MASK));
            
        /* Save node to be moved to ready list */
        node_t_p_changeListNode = node_t_p_check;
        
        /* Move to next node in wait list */
        node_t_p_check = node_t_p_check->nextNode;
        
        /* Remove from waiting list and add to ready queue by priority. */
        vd_list_removeNode(&node_s_p_headOfWaitList, node_t_p_changeListNode);
        vd_list_addTaskByPrio(&node_s_p_headOfReadyList, node_t_p_changeListNode);      
      }
      else
      {      
        /* Move to next node in wait list */
        node_t_p_check = node_t_p_check->nextNode;
      }        
    }
    
    /* Is first task in ready queue the same as before tick? */
    if(node_s_p_headOfReadyList->TCB == tcb_g_p_currentTaskBlock)
    {  
      /* Do nothing, return to current task. */
    }
    else
    {
      /* Set global task pointer to new task control block */
      tcb_g_p_nextTaskBlock = node_s_p_headOfReadyList->TCB;

      /* Set bit for pendSV to run when CPU is ready */
      OS_CPU_TRIGGER_DISPATCHER();
    }
  } /* node_s_p_headOfWaitList == NULL */
}

/*************************************************************************/
/*  Function Name: vd_sch_background                                     */
/*  Purpose:       Background task when no others are scheduled.         */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
#if (RTOS_CONFIG_BG_TASK == RTOS_CONFIG_TRUE)
static void vd_sch_background(void)
{
  U1 u1_t_index;

  for(;;)
  {     
#if(RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT == RTOS_CONFIG_TRUE)
    for(u1_t_index = ZERO; u1_t_index < u1_s_numTasks; u1_t_index++)
    {
      if(u1_sch_checkStack(u1_t_index))
      {
        OSTaskFault();
      }
    }
#endif
    
#if (RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
    OS_s_cpuData.CPUIdlePercent.CPU_idlePrevTimestamp = u1_cpu_getPercentOfTick();
#endif
    
#if(RTOS_CONFIG_ENABLE_BACKGROUND_IDLE_SLEEP == RTOS_CONFIG_TRUE)
    u1_s_sleepState = (U1)SCH_CPU_SLEEPING;
  #if(RTOS_CONFIG_PRESLEEP_FUNC == RTOS_CONFIG_TRUE)
    /* pre-sleep hook function defined by application */
    app_OSPreSleepFcn();
  #endif
    WaitForInterrupt();
#endif
  }
}
#endif

/*************************************************************************/
/*  Function Name: u1_sch_checkStack                                     */
/*  Purpose:       Check watermark on task stacks.                       */
/*  Arguments:     U1 taskIndex:                                         */
/*                    Task ID to be checked.                             */
/*  Return:        SCH_TRUE or SCH_FALSE                                 */
/*************************************************************************/
#if(RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT == RTOS_CONFIG_TRUE)
static U1 u1_sch_checkStack(U1 taskIndex)
{
  return(*SchTask_s_as_taskList[taskIndex].topOfStack != (OS_STACK)SCH_TOP_OF_STACK_MARK);
}
#endif

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                2/9/19      Basic time-sharing functionality implemented. Processes up   */
/*                                to five tasks according to their period and sequence.        */
/*                                                                                             */
/* 0.2                2/12/19     Took timer/interrupt operations and combined into            */
/*                                timeHandler module. Has since been migrated into CPU IF mod. */
/*                                                                                             */
/* 1.0                2/28/19     First implementation of pre-emptive scheduler.               */
/*                                                                                             */
/* 1.1                3/2/19      Changes to scheduling algorithm. Priority-based instead of   */
/*                                time-based, added blocking and yielding APIs for tasks.      */
/*                                                                                             */
/* 1.2                3/25/19     Resolved scheduler bug in which a lower priority task would  */
/*                                not have its sleep timer decremented on a SysTick interrupt  */
/*                                if a higher priority task was ready to run.                  */
/*                                                                                             */
/* 1.3                5/20/19     Added members to TCB to track which resource task is blocked */
/*                                on. Scheduler calls APIs to remove task ID from resource     */
/*                                block list.                                                  */
/*                                                                                             */
/* 1.4                5/21/19     Added TCB member and public API to track task wakeup reason. */
/*                                Application can now determine if task woke up due to timeout,*/
/*                                or a resource became available.                              */
/*                                                                                             */
/* 1.5                6/9/19      Added stack overflow detection.                              */
/*                                                                                             */
/* 1.6                6/9/19      Added u4_OSsch_taskSleepSetFreq to support task execution at */
/*                                set frequencies.                                             */
/*                                                                                             */
/* 1.7                6/22/19     Added API to get CPU load during runtime.                    */
/*                                                                                             */
/* 1.8                7/13/19     Added hook functions for pre-sleep, post-sleep.              */
/*                                                                                             */
/* 2.0                7/20/19     Redesigned scheduler from array-based with static priorities */
/*                                to using linked-lists and dynamic priorities (user can change*/
/*                                during runtime). Uses ready queue, waiting list, and an array*/
/*                                to map task ID (priority) used in APIs --> linked list node. */
/*                                                                                             */
