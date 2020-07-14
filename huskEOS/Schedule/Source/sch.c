/*************************************************************************/
/*  File Name: sch.c                                                     */
/*  Purpose: Init and routines for scheduler module and task handling.   */
/*  Created by: Garrett Sculthorpe on 2/29/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
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

#if(RTOS_CFG_OS_MUTEX_ENABLED == RTOS_CONFIG_TRUE)
#include "mutex_internal_IF.h"
#endif

#if(RTOS_CFG_OS_MEM_ENABLED == RTOS_CONFIG_TRUE)
#include "memory_internal_IF.h"
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
#define SCH_TASK_FLAG_SLEEP_MUTEX                (SCH_TASK_WAKEUP_MUTEX_READY)
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
#define SCH_BG_TASK_ID                           (SCH_MAX_NUM_TASKS - 1)
#define SCH_NULL_PTR                             ((void*)ZERO)
#define SCH_MAX_NUM_TICK                         (4294967200U)

/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/
ListNode* Node_s_ap_mapTaskIDToTCB[SCH_MAX_NUM_TASKS];

/* Note: These global variables are modified by asm routine */
Sch_Task* tcb_g_p_currentTaskBlock;
Sch_Task* tcb_g_p_nextTaskBlock;

/*************************************************************************/
/*  Static Global Variables, Constants                                   */
/*************************************************************************/
static U1 u1_s_numTasks;
static U4 u4_s_tickCntr;
#if(RTOS_CONFIG_PRESLEEP_FUNC == RTOS_CONFIG_TRUE || RTOS_CONFIG_POSTSLEEP_FUNC == RTOS_CONFIG_TRUE)
static U1 u1_s_sleepState;
#endif
static ListNode* node_s_p_headOfWaitList;
static ListNode* node_s_p_headOfReadyList;
static OS_STACK  u4_backgroundStack[SCH_BG_TASK_STACK_SIZE];

/* Allocate memory for data structures used for TCBs and scheduling queues */
static ListNode   Node_s_as_listAllTasks[SCH_MAX_NUM_TASKS];     
static Sch_Task   SchTask_s_as_taskList[SCH_MAX_NUM_TASKS];

#if (RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
static OS_RunTimeStats OS_s_cpuData;
#endif


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
static void vd_OSsch_background(void);

#if(RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT == RTOS_CONFIG_TRUE)
static U1 u1_sch_checkStack(U1 taskIndex);
#endif

static void vd_OSsch_setNextReadyTaskToRun(void);
static void vd_OSsch_taskSleepTimeoutHandler(Sch_Task* taskTCB);
static void vd_OSsch_periodicScheduler(void);

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
#if(RTOS_CONFIG_PRESLEEP_FUNC == RTOS_CONFIG_TRUE || RTOS_CONFIG_POSTSLEEP_FUNC == RTOS_CONFIG_TRUE)
  u1_s_sleepState    = (U1)SCH_CPU_NOT_SLEEPING;
#endif
  
  /* Initialize task and queue data to default values */
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)SCH_MAX_NUM_TASKS; u1_t_index++)
  {
    SchTask_s_as_taskList[u1_t_index].stackPtr          = (OS_STACK*)NULL;
    SchTask_s_as_taskList[u1_t_index].flags             = (U1)ZERO;
    SchTask_s_as_taskList[u1_t_index].sleepCntr         = (U4)ZERO;
    SchTask_s_as_taskList[u1_t_index].resource          = (void*)NULL;
    SchTask_s_as_taskList[u1_t_index].wakeReason        = (U1)ZERO;
    SchTask_s_as_taskList[u1_t_index].priority          = (U1)SCH_TASK_PRIORITY_UNDEFINED;
    SchTask_s_as_taskList[u1_t_index].taskID            = (U1)SCH_INVALID_TASK_ID;
    
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
  u1_OSsch_createTask(&vd_OSsch_background, 
                      &u4_backgroundStack[SCH_BG_TASK_STACK_SIZE - ONE], 
                      (U4)RTOS_CONFIG_BG_TASK_STACK_SIZE, 
                      (U1)SCH_TASK_LOWEST_PRIORITY, 
                      (U1)SCH_BG_TASK_ID);
  
  /* Mask interrupts until RTOS enters normal operation */
  vd_cpu_disableInterruptsOSStart();
  vd_cpu_init(numMsPeriod);
 
#if(RTOS_CFG_OS_MAILBOX_ENABLED == RTOS_CONFIG_TRUE)  
  vd_OSmbox_init();
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
/*                 U4 sizeOfStack:                                       */
/*                       Size of task stack.                             */
/*                 U1 priority:                                          */
/*                       Unique priority level for task. 0 = highest.    */
/*                 U1 taskID:                                            */
/*                       Task ID to refer to task when using APIs (cannot*/
/*                       be changed). Value must be between 0 and the    */
/*                       total number of application tasks - 1.          */
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
    SchTask_s_as_taskList[u1_s_numTasks].priority = priority;
    SchTask_s_as_taskList[u1_s_numTasks].taskID   = taskID;
    
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
  
  OS_CPU_TRIGGER_DISPATCHER();
  vd_cpu_enableInterruptsOSStart();
}

/*************************************************************************/
/*  Function Name: vd_OSsch_interruptEnter                               */
/*  Purpose:       Must be called by ISRs external to OS at entry.       */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
U1 u1_OSsch_interruptEnter(void)
{
#if(RTOS_CONFIG_PRESLEEP_FUNC == RTOS_CONFIG_TRUE)
  if(u1_s_sleepState == (U1)SCH_CPU_SLEEPING)
  {  
    /* Post-sleep hook function defined by application */
    app_OSPostSleepFcn();
    
    u1_s_sleepState = (U1)SCH_CPU_NOT_SLEEPING;
  }
#endif
  
  return (u1_OSsch_maskInterrupts());
}  
  
/*************************************************************************/
/*  Function Name: vd_OSsch_interruptExit                                */
/*  Purpose:       Must be called by ISRs external to OS at exit.        */
/*  Arguments:     U1 prioMaskReset:                                     */
/*                    Priority mask returned by u1_OSsch_interruptEnter()*/
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_interruptExit(U1 prioMaskReset)
{
  vd_OSsch_unmaskInterrupts(prioMaskReset);
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
/*                 SCH_TASK_WAKEUP_FLAGS_EVENT            OR             */
/*                 SCH_TASK_WAKEUP_MUTEX_READY            OR             */
/*                 OS flags event that triggered wakeup                  */
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
/*  Return:        Number of scheduler ticks.                            */
/*************************************************************************/
U4 u4_OSsch_getTicks(void)
{
  return(u4_s_tickCntr);
}

/*************************************************************************/
/*  Function Name: u1_OSsch_getCurrentTaskID                             */
/*  Purpose:       Returns current task ID.                              */
/*  Arguments:     N/A                                                   */
/*  Return:        U1: Current task ID number.                           */
/*************************************************************************/
U1 u1_OSsch_getCurrentTaskID(void)
{
  return(tcb_g_p_currentTaskBlock->taskID);
}

/*************************************************************************/
/*  Function Name: u1_OSsch_getCurrentTaskPrio                           */
/*  Purpose:       Returns current task priority.                        */
/*  Arguments:     N/A                                                   */
/*  Return:        U1: Current task priority.                            */
/*************************************************************************/
U1 u1_OSsch_getCurrentTaskPrio(void)
{
  return(tcb_g_p_currentTaskBlock->priority);
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
/*  Function Name: vd_OSsch_setReasonForWakeup                           */
/*  Purpose:       Set reason for wakeup to resource available. Called   */
/*                 internal to RTOS by other RTOS modules. It is expected*/
/*                 that OS internal modules will call taskWake() *after* */
/*                 this function call and maintain their own block lists.*/
/*  Arguments:     U1 reason:                                            */
/*                    Identifier code for wakeup reason.                 */
/*                 Sch_Task* wakeupTaskTCB:                              */
/*                    Pointer to task TCB that is being woken up which   */
/*                    was stored on resource blocked list.               */
/*  Return:        void                                                  */
/*************************************************************************/
void vd_OSsch_setReasonForWakeup(U1 reason, struct Sch_Task* wakeupTaskTCB)
{
  OS_CPU_ENTER_CRITICAL();
  
  /* Clear OS resource pointer. */
  wakeupTaskTCB->resource = (void*)NULL;
  
  /* Remove sleep reason from flags entry in TCB */
  wakeupTaskTCB->flags &= ~((U1)reason);
  
  /* Set the wakeup reason for application to read. */
  wakeupTaskTCB->wakeReason = reason;
  
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_OSsch_setReasonForSleep                            */
/*  Purpose:       Set reason for task sleep according to mask and set   */
/*                 task to sleep state.                                  */
/*  Arguments:     void* taskSleepResource:                              */
/*                       Address of resource task is blocked on.         */
/*                 U1 resourceType:                                      */
/*                       Code for resource that task is sleeping on.     */
/*                 U4 period:                                            */
/*                       Period to sleep for.                            */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_setReasonForSleep(void* taskSleepResource, U1 resourceType, U4 period)
{
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  OS_CPU_ENTER_CRITICAL();
  
  tcb_g_p_currentTaskBlock->resource = taskSleepResource;
  tcb_g_p_currentTaskBlock->flags   |= (U1)resourceType;
	
	tcb_g_p_currentTaskBlock->sleepCntr = period; 
  tcb_g_p_currentTaskBlock->flags    |= (U1)SCH_TASK_FLAG_STS_SLEEP;
  
  /* Switch to an active task */
  vd_OSsch_setNextReadyTaskToRun();
  OS_CPU_TRIGGER_DISPATCHER();
  
  /* Resume tick interrupts and enable context switch interrupt. */
  OS_CPU_EXIT_CRITICAL();
}

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
U1 u1_OSsch_setNewPriority(struct Sch_Task* tcb, U1 newPriority)
{
  U1 u1_t_prevPrio;
  
  OS_CPU_ENTER_CRITICAL();
  
  /* If task is suspended or sleeping then simply change priority. */
  if(tcb->flags & (U1)SCH_TASK_FLAG_STS_CHECK)
  {
    u1_t_prevPrio = tcb->priority;
    tcb->priority = newPriority;
  }
  else
  {
    /* Remove node from current position. */
    vd_list_removeNode(&node_s_p_headOfReadyList, Node_s_ap_mapTaskIDToTCB[tcb->taskID]);
    
    /* Change priority. */
    u1_t_prevPrio = tcb->priority;
    tcb->priority = newPriority;
    
    /* Add back into list in order. */
    vd_list_addTaskByPrio(&node_s_p_headOfReadyList, Node_s_ap_mapTaskIDToTCB[tcb->taskID]);
    
    /* Is new priority higher priority than current task ? */
    if(node_s_p_headOfReadyList->TCB != tcb_g_p_currentTaskBlock)
    { 
      /* Set global task pointer to new task control block */
      tcb_g_p_nextTaskBlock = node_s_p_headOfReadyList->TCB;
      
      OS_CPU_TRIGGER_DISPATCHER();
    }
    else
    {
      
    }
  }
  
  OS_CPU_EXIT_CRITICAL();
  
  return (u1_t_prevPrio);
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
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  OS_CPU_ENTER_CRITICAL();
  
  tcb_g_p_currentTaskBlock->sleepCntr = period; 
  tcb_g_p_currentTaskBlock->flags    |= (U1)SCH_TASK_FLAG_STS_SLEEP;
  
  /* Switch to an active task */
  vd_OSsch_setNextReadyTaskToRun();
  OS_CPU_TRIGGER_DISPATCHER();

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

  /* Handle tick roll-over. */
  if(nextWakeTime > u4_s_tickCntr)
  {
    tcb_g_p_currentTaskBlock->sleepCntr = nextWakeTime - u4_s_tickCntr; 
  }
  else
  {
    tcb_g_p_currentTaskBlock->sleepCntr = ((U4)SCH_MAX_NUM_TICK - u4_s_tickCntr) + nextWakeTime;
  }
  
  tcb_g_p_currentTaskBlock->flags |= (U1)SCH_TASK_FLAG_STS_SLEEP;
  
  /* Switch to an active task */
  vd_OSsch_setNextReadyTaskToRun();
  OS_CPU_TRIGGER_DISPATCHER();
  
  /* Resume tick interrupts and enable context switch interrupt. */
  OS_CPU_EXIT_CRITICAL();

  return(u4_s_tickCntr);  
}

/*************************************************************************/
/*  Function Name: vd_OSsch_taskWake                                     */
/*  Purpose:       Wake specified task from sleep or suspended state.    */
/*  Arguments:     U1 taskID:                                            */
/*                    Task ID to be woken from sleep or suspend state.   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_taskWake(U1 taskID)
{  
  OS_CPU_ENTER_CRITICAL();
  
  /* Check that task is not already in ready state. */
  if(Node_s_ap_mapTaskIDToTCB[taskID]->TCB->flags & (U1)SCH_TASK_FLAG_STS_CHECK)
  {
    /* If task is blocked on resource, then tell resource that task has timed out. */
    if(Node_s_ap_mapTaskIDToTCB[taskID]->TCB->resource != SCH_NULL_PTR)
    {
      vd_OSsch_taskSleepTimeoutHandler(Node_s_ap_mapTaskIDToTCB[taskID]->TCB);
    }
    else
    {
      /* Task not blocked on resource. */
    }
    
    Node_s_ap_mapTaskIDToTCB[taskID]->TCB->sleepCntr  =   (U4)ZERO; 
    Node_s_ap_mapTaskIDToTCB[taskID]->TCB->flags     &= ~((U1)(SCH_TASK_FLAG_STS_SLEEP|SCH_TASK_FLAG_STS_SUSPENDED));
    
#if(RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
    /* Check if CPU was previously idle, make calculation if so. */
    if(tcb_g_p_currentTaskBlock == (Node_s_ap_mapTaskIDToTCB[SCH_BG_TASK_ID]->TCB))
    {
      OS_s_cpuData.CPUIdlePercent.CPU_idleRunning += (u1_cpu_getPercentOfTick() - OS_s_cpuData.CPUIdlePercent.CPU_idlePrevTimestamp);
    }
    else {}
#endif
    
    /* Remove task from wait list */
    vd_list_removeNode(&node_s_p_headOfWaitList, Node_s_ap_mapTaskIDToTCB[taskID]); 
    
    /* Add woken task to ready queue */
    vd_list_addTaskByPrio(&node_s_p_headOfReadyList, Node_s_ap_mapTaskIDToTCB[taskID]);
    
    /* Is woken up task higher priority than current task ? */
    if(node_s_p_headOfReadyList->TCB != tcb_g_p_currentTaskBlock)
    { 
      /* Set global task pointer to new task control block */
      tcb_g_p_nextTaskBlock = node_s_p_headOfReadyList->TCB;
      
      OS_CPU_TRIGGER_DISPATCHER();
    }
    else
    {
      
    }
  }
  else
  {
    /* Task is not in sleep or suspended state. Do nothing. */
  }/* Node_s_ap_mapTaskIDToTCB[taskID]->TCB->flags & (U1)SCH_TASK_FLAG_STS_CHECK */
  
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_OSsch_taskSuspend                                  */
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
  
  if((node_t_p_suspendTask->TCB->flags & (U1)SCH_TASK_FLAG_STS_CHECK) != (U1)SCH_TASK_FLAG_STS_SUSPENDED)
  {
    Node_s_ap_mapTaskIDToTCB[taskIndex]->TCB->flags |= (U1)SCH_TASK_FLAG_STS_SUSPENDED;
    vd_list_removeNode(&node_s_p_headOfReadyList, node_t_p_suspendTask); 
    vd_list_addNodeToEnd(&node_s_p_headOfWaitList, node_t_p_suspendTask); 
  }
  else{}

  /* Is task suspending itself or another task? */
  if(node_t_p_suspendTask->TCB == tcb_g_p_currentTaskBlock)
  {
    /* Switch to an active task */
    tcb_g_p_nextTaskBlock = node_s_p_headOfReadyList->TCB;
    OS_CPU_TRIGGER_DISPATCHER();
  }
  else{}
  
  /* Resume tick interrupts and enable context switch interrupt. */
  OS_CPU_EXIT_CRITICAL();
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
/*  Function Name: vd_OSsch_systemTick_ISR                               */
/*  Purpose:       Handle system tick operations and run scheduler.      */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
__irq void vd_OSsch_systemTick_ISR(void)
{
  U1 u1_t_prioMask;

  u1_t_prioMask = u1_OSsch_interruptEnter();
  
  /* Increment ticks but cap at max (0xFFFFFFFF rounded down to nearest 100). */
  u4_s_tickCntr = (u4_s_tickCntr + 1) % (U4)SCH_MAX_NUM_TICK;
  
#if(RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
  /* Check if CPU was previously idle, make calculation if so. */
  if(tcb_g_p_currentTaskBlock == (Node_s_ap_mapTaskIDToTCB[SCH_BG_TASK_ID]->TCB))
  {
    OS_s_cpuData.CPUIdlePercent.CPU_idleRunning += ((U1)SCH_ONE_HUNDRED_PERCENT - OS_s_cpuData.CPUIdlePercent.CPU_idlePrevTimestamp);
  }
  else{}
  
  if((u4_s_tickCntr % (U4)SCH_HUNDRED_TICKS) == (U4)ZERO)
  {
    OS_s_cpuData.CPUIdlePercent.CPU_idleAvg     = (U1)(OS_s_cpuData.CPUIdlePercent.CPU_idleRunning/(U4)SCH_HUNDRED_TICKS);
    OS_s_cpuData.CPUIdlePercent.CPU_idleRunning = (U1)ZERO;
  }
  else{}
  
  OS_s_cpuData.CPUIdlePercent.CPU_idlePrevTimestamp = (U1)ZERO;
#endif  
  
  vd_OSsch_periodicScheduler();

  /* Resume tick interrupts and enable context switch interrupt. */
  vd_OSsch_interruptExit(u1_t_prioMask);
}

/*************************************************************************/
/*  Function Name: vd_OSsch_setNextReadyTaskToRun                        */
/*  Purpose:       Select next task to run.                              */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_OSsch_setNextReadyTaskToRun(void)
{
  ListNode* node_t_p_moveToWaitList;

  /* Remove node of task that was previously executing */
  node_t_p_moveToWaitList = node_list_removeFirstNode(&node_s_p_headOfReadyList);
  
  /* Move previous node to wait list */
  vd_list_addNodeToFront(&node_s_p_headOfWaitList, node_t_p_moveToWaitList);
  
  tcb_g_p_nextTaskBlock = node_s_p_headOfReadyList->TCB;
}

/*************************************************************************/
/*  Function Name: vd_OSsch_taskSleepTimeoutHandler                      */
/*  Purpose:       Tell resources that blocked task has timed out.       */
/*  Arguments:     Sch_Task* taskTCB:                                    */
/*                           Pointer to task control block.              */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_OSsch_taskSleepTimeoutHandler(Sch_Task* taskTCB)
{        
  /* Task has timed out. Determine if due to manual sleep or resource */
  switch(taskTCB->flags & (U1)SCH_TASK_RESOURCE_SLEEP_CHECK_MASK)
  {
    /* Remove task from resource's blocked list */
#if(RTOS_CFG_OS_MAILBOX_ENABLED == RTOS_CONFIG_TRUE)
    case (U1)SCH_TASK_FLAG_SLEEP_MBOX:
      vd_OSmbox_blockedTaskTimeout((Mailbox*)taskTCB->resource);
      taskTCB->resource = (void*)NULL;
      break;
#endif          
#if(RTOS_CFG_OS_QUEUE_ENABLED == RTOS_CONFIG_TRUE)            
    case (U1)SCH_TASK_FLAG_SLEEP_QUEUE:
      vd_OSqueue_blockedTaskTimeout((Queue*)taskTCB->resource, taskTCB);
      taskTCB->resource = (void*)NULL;
      break;
#endif   
#if(RTOS_CFG_OS_SEMAPHORE_ENABLED == RTOS_CONFIG_TRUE)            
    case (U1)SCH_TASK_FLAG_SLEEP_SEMA:
      vd_OSsema_blockedTimeout((Semaphore*)taskTCB->resource, taskTCB);
      taskTCB->resource = (void*)NULL;
      break;
#endif         
#if(RTOS_CFG_OS_FLAGS_ENABLED == RTOS_CONFIG_TRUE)            
    case (U1)SCH_TASK_FLAG_SLEEP_FLAGS:
      vd_OSflags_pendTimeout((FlagsObj*)taskTCB->resource, taskTCB);
      taskTCB->resource = (void*)NULL;
      break;
#endif         
#if(RTOS_CFG_OS_MUTEX_ENABLED == RTOS_CONFIG_TRUE)
    case (U1)SCH_TASK_FLAG_SLEEP_MUTEX:
      vd_OSmutex_blockedTimeout((Mutex*)taskTCB->resource, taskTCB);
      taskTCB->resource = (void*)NULL;
      break;
#endif    
    case (U1)ZERO:
      /* Manual sleep time out */
      break;  

    default:
      OSTaskFault(); /* If code execution reaches this point it is a bug */
      break;             
  }          
}

/*************************************************************************/
/*  Function Name: vd_OSsch_periodicScheduler                            */
/*  Purpose:       Scheduler algorithm called to check if task is ready. */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_OSsch_periodicScheduler(void)
{
  ListNode* node_t_p_check;
  ListNode* node_t_p_changeListNode;
  Sch_Task* tcb_t_p_currentTCB;
  
  if(node_s_p_headOfWaitList == (ListNode*)NULL)
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
        vd_OSsch_taskSleepTimeoutHandler(tcb_t_p_currentTCB);
#endif        
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
/*  Function Name: vd_OSsch_background                                   */
/*  Purpose:       Background task when no others are scheduled.         */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_OSsch_background(void)
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
      else{}
    }
#endif
#if(RTOS_CFG_OS_MEM_ENABLED == RTOS_CONFIG_TRUE)
    if(u1_OSMem_maintenance())
    {
      OSTaskFault();
    }
    else{}
#endif    
#if (RTOS_CONFIG_CALC_TASK_CPU_LOAD == RTOS_CONFIG_TRUE)
    OS_s_cpuData.CPUIdlePercent.CPU_idlePrevTimestamp = u1_cpu_getPercentOfTick();
#endif
    
#if(RTOS_CONFIG_ENABLE_BACKGROUND_IDLE_SLEEP == RTOS_CONFIG_TRUE)
  #if(RTOS_CONFIG_PRESLEEP_FUNC == RTOS_CONFIG_TRUE || RTOS_CONFIG_POSTSLEEP_FUNC == RTOS_CONFIG_TRUE)
    u1_s_sleepState = (U1)SCH_CPU_SLEEPING;
  #endif
  #if(RTOS_CONFIG_PRESLEEP_FUNC == RTOS_CONFIG_TRUE)
    /* pre-sleep hook function defined by application */
    app_OSPreSleepFcn();
  #endif
    WaitForInterrupt();
#endif
  }
}

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
/* 2.1                7/29/19     vd_OSsch_taskWake() call removed from                        */
/*                                vd_OSsch_setReasonForWakeup() to save space on call stack.   */
/*                                When vd_OSsch_setReasonForWakeup() is called by an internal  */
/*                                module, it must also now call vd_OSsch_taskWake().           */
/*                                -Note: These two function calls must occur in the same       */
/*                                critical section.                                            */
/*                                                                                             */
/*                                Removed getCurrentTCB() and replaced with #define in         */
/*                                sch_internal_F.h                                             */
/*                                                                                             */
/* 2.2                8/19/19     Fixed minor bugs in CPU load calculation.                    */
/*                                                                                             */
/* 2.3                8/27/19     Integrated memory module.                                    */
/*                                                                                             */
/* 2.4                5/3/20      Changed SCH_BG_TASK_ID from ZERO to (SCH_MAX_NUM_TASKS - 1)  */
/*                                to so that task ID zero is available (to match priorities).  */
/*                                                                                             */
/* 2.5                5/4/20      Changing SCH_BG_TASK_ID caused bug in CPU load calculation.  */
/*                                Bug is now resolved.                                         */
