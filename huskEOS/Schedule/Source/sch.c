/*************************************************************************/
/*  File Name: sch.c                                                     */
/*  Purpose: Init and routines for scheduler module and task handling.   */
/*  Created by: Garrett Sculthorpe on 2/29/19.                           */
/*************************************************************************/

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "sch.h"

#if(RTOS_CFG_OS_MAILBOX_ENABLED == RTOS_CONFIG_TRUE)
#include "mbox_internal_IF.h"     
#endif

#if(RTOS_CFG_OS_QUEUE_ENABLED == RTOS_CONFIG_TRUE)
#include "queue_internal_IF.h"
#endif

#if(RTOS_CFG_OS_SEMA_ENABLED == RTOS_CONFIG_TRUE)
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
                                                  
#define SCH_TRUE                                 (1)
#define SCH_FALSE                                (0)
#define SCH_TICK_FLAG_TRUE                       (SCH_TRUE)            
#define SCH_TICK_FLAG_FALSE                      (SCH_FALSE)
#define SCH_TCB_PTR_INIT                         (NULL)
#define SCH_TASK_FLAG_STS_SLEEP                  (0x10)
#define SCH_TASK_FLAG_STS_SUSPENDED              (0x20) 
#define SCH_TASK_FLAG_STS_YIELD                  (0x40)
#define SCH_TASK_FLAG_SLEEP_MBOX                 (SCH_TASK_WAKEUP_MBOX_READY)
#define SCH_TASK_FLAG_SLEEP_QUEUE                (SCH_TASK_WAKEUP_QUEUE_READY)
#define SCH_TASK_FLAG_SLEEP_SEMA                 (SCH_TASK_WAKEUP_SEMA_READY)
#define SCH_TASK_FLAG_SLEEP_FLAGS                (SCH_TASK_WAKEUP_FLAGS_EVENT)
#define SCH_TASK_YIELD                           (0)
#define SCH_TASK_FLAG_STS_CHECK                  (SCH_TASK_FLAG_STS_SLEEP | SCH_TASK_FLAG_STS_SUSPENDED | SCH_TASK_FLAG_STS_YIELD) 
#define SCH_TASK_RESOURCE_SLEEP_CHECK_MASK       (SCH_TASK_FLAG_SLEEP_MBOX | SCH_TASK_FLAG_SLEEP_QUEUE | SCH_TASK_FLAG_SLEEP_SEMA | SCH_TASK_FLAG_SLEEP_FLAGS)
#define SCH_YIELD_TASK_NOT_SET                   (0xFF)
#define SCH_TOP_OF_STACK_MARK                    (0xF0F0F0F0)
  
/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/
static U1 u1_s_numTasks;
static U1 u1_s_tickFlg; /* Protected by scheduler critical section */
static U1 u1_s_taskTCBIndex;
static U4 u4_s_tickCntr;

#if (RTOS_CONFIG_BG_TASK == RTOS_CONFIG_TRUE)
static OS_STACK u4_backgroundStack[SCH_BG_TASK_STACK_SIZE];
#endif
static Sch_Task SchTask_s_as_taskList[SCH_MAX_NUM_TASKS];

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

static void vd_sch_main(void);

/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_OS_init                                            */
/*  Purpose:       Initialize scheduler module and configured RTOS       */
/*                 modukes.                                              */
/*  Arguments:     U4 numMsPeriod:                                       */
/*                    Sets scheduler tick rate in milliseconds.          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OS_init(U4 numMsPeriod)
{
  U1 u1_t_index;
  
  u1_s_taskTCBIndex  = (U1)ZERO;
  u1_s_numTasks      = (U1)SCH_NUM_TASKS_ZERO;
  u1_s_tickFlg       = (U1)SCH_TICK_FLAG_FALSE;
  u4_s_tickCntr      = (U1)ZERO;

  /* Initialize task list to default values */
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)SCH_MAX_NUM_TASKS; u1_t_index++)
  {
    SchTask_s_as_taskList[u1_t_index].stackPtr   = (OS_STACK*)NULL;
    SchTask_s_as_taskList[u1_t_index].flags      = (U1)ZERO;
    SchTask_s_as_taskList[u1_t_index].sleepCntr  = (U4)ZERO;
    SchTask_s_as_taskList[u1_t_index].resource   = (void*)NULL;
    SchTask_s_as_taskList[u1_t_index].wakeReason = (U1)ZERO;
  }
  
  tcb_g_p_currentTaskBlock =  (void*)SCH_TCB_PTR_INIT;
  
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
/*                                                                       */
/*  Return:        SCH_TASK_CREATE_SUCCESS   OR                          */
/*                 SCH_TASK_CREATE_DENIED                                */
/*************************************************************************/
U1 u1_OSsch_createTask(void (*newTaskFcn)(void), void* sp, U4 sizeOfStack)
{
  if(u1_s_numTasks >= (U1)SCH_MAX_NUM_TASKS)
  {
    return ((U1)SCH_TASK_CREATE_DENIED);
  }
  
  /* Set new task stack pointers */
#if(STACK_GROWTH == STACK_DESCENDING)
  SchTask_s_as_taskList[u1_s_numTasks].topOfStack = ((OS_STACK*)sp - sizeOfStack + ONE);
#elif(STACK_GROWTH == STACK_ASCENDING)
  SchTask_s_as_taskList[u1_s_numTasks].topOfStack = ((OS_STACK*)sp + sizeOfStack - ONE);
#else 
  #error "STACK DIRECTION NOT PROPERLY DEFINED"
#endif
  
  SchTask_s_as_taskList[u1_s_numTasks].stackPtr    =  sp_cpu_taskStackInit(newTaskFcn, sp);
  *SchTask_s_as_taskList[u1_s_numTasks].topOfStack = (OS_STACK)SCH_TOP_OF_STACK_MARK;
  /* Increment number of tasks */
  ++u1_s_numTasks;

  return ((U1)SCH_TASK_CREATE_SUCCESS);
}

/*************************************************************************/
/*  Function Name: vd_OSsch_start                                        */
/*  Purpose:       Give control to operating system.                     */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_start(void)
{
#if (RTOS_CONFIG_BG_TASK == RTOS_CONFIG_TRUE)
  u1_OSsch_createTask(&vd_sch_background, &u4_backgroundStack[SCH_BG_TASK_STACK_SIZE - ONE], (U4)RTOS_CONFIG_BG_TASK_STACK_SIZE);
#endif

  /* Start at highest priority task */
  tcb_g_p_nextTaskBlock = &SchTask_s_as_taskList[ZERO];
  vd_cpu_enableInterruptsOSStart();
  OS_CPU_TRIGGER_DISPATCHER();
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
  U1 u1_t_intMask;
  
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  u1_t_intMask = u1_OSsch_maskInterrupts();
  
  u1_t_reason = SchTask_s_as_taskList[u1_s_taskTCBIndex].wakeReason;
  SchTask_s_as_taskList[u1_s_taskTCBIndex].wakeReason = (U1)SCH_TASK_NO_WAKEUP_SINCE_LAST_CHECK;
  
  /* Resume tick interrupts and enable context switch interrupt. */
  vd_OSsch_unmaskInterrupts(u1_t_intMask);
  
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
/*  Function Name: u1_OSsch_getCurrentTask                               */
/*  Purpose:       Returns current TCB index.                            */
/*  Arguments:     N/A                                                   */
/*  Return:        U1 u1_s_taskTCBIndex (index of current task)          */
/*************************************************************************/
U1 u1_OSsch_getCurrentTask(void)
{
  return(u1_s_taskTCBIndex);
}

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
/*                 U1 wakeupTaskID:                                      */
/*                    Task ID that is being woken up.                    */
/*  Return:        void                                                  */
/*************************************************************************/
void vd_sch_setReasonForWakeup(U1 reason, U1 wakeupTaskID)
{
  U1 u1_t_intMask;
  
  u1_t_intMask = u1_OSsch_maskInterrupts();
  
  SchTask_s_as_taskList[wakeupTaskID].resource   = (void*)NULL;
  SchTask_s_as_taskList[wakeupTaskID].flags     &= ~((U1)reason);
  SchTask_s_as_taskList[wakeupTaskID].wakeReason = reason;
  
  vd_OSsch_taskWake(wakeupTaskID);
  
  vd_OSsch_unmaskInterrupts(u1_t_intMask);
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
  U1 u1_t_intMask;
  
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  u1_t_intMask = u1_OSsch_maskInterrupts();
  SchTask_s_as_taskList[u1_s_taskTCBIndex].resource = taskSleepResource;
  SchTask_s_as_taskList[u1_s_taskTCBIndex].flags   |= (U1)resourceType;
  
  /* Resume tick interrupts and enable context switch interrupt. */
  vd_OSsch_unmaskInterrupts(u1_t_intMask);
}

/*************************************************************************/
/*  Function Name: vd_OSsch_taskYield                                    */
/*  Purpose:       Suspends task to see if higher                        */
/*                 priority task can run before continuing.              */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_taskYield(void)
{
  vd_OSsch_taskSleep((U4)SCH_TASK_YIELD); 
}

/*************************************************************************/
/*  Function Name: vd_OSsch_taskSleep                                    */
/*  Purpose:       Suspend current task for a specified amount of time.  */
/*  Arguments:     U4 u4_period:                                         */
/*                    Time units to suspend for. sch_main handles timing.*/
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsch_taskSleep(U4 period)
{
  U1 u1_t_intMask;
  
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  u1_t_intMask = u1_OSsch_maskInterrupts();
  
  if(period == (U4)SCH_TASK_YIELD)
  {
    /* Yield does not sleep. It only yields to highest priority waiting task, or returns to itself if none ready. */
    SchTask_s_as_taskList[u1_s_taskTCBIndex].flags |= (U1)SCH_TASK_FLAG_STS_YIELD;
  }
  else
  {
    SchTask_s_as_taskList[u1_s_taskTCBIndex].sleepCntr = period; 
    SchTask_s_as_taskList[u1_s_taskTCBIndex].flags    |= (U1)SCH_TASK_FLAG_STS_SLEEP;
  }
  
  /* Switch to an active task */
  vd_sch_main();
  
  /* Resume tick interrupts and enable context switch interrupt. */
  vd_OSsch_unmaskInterrupts(u1_t_intMask);
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
  U1 u1_t_intMask;

  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  u1_t_intMask = u1_OSsch_maskInterrupts();

  SchTask_s_as_taskList[u1_s_taskTCBIndex].sleepCntr = nextWakeTime - u4_s_tickCntr; 
  SchTask_s_as_taskList[u1_s_taskTCBIndex].flags    |= (U1)SCH_TASK_FLAG_STS_SLEEP;
  
  /* Switch to an active task */
  vd_sch_main();
  
  /* Resume tick interrupts and enable context switch interrupt. */
  vd_OSsch_unmaskInterrupts(u1_t_intMask);

  
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
  U1 u1_t_intMask;
  
  u1_t_intMask = u1_OSsch_maskInterrupts();
  
  SchTask_s_as_taskList[taskIndex].sleepCntr =   (U4)ZERO; 
  SchTask_s_as_taskList[taskIndex].flags    &= ~((U1)(SCH_TASK_FLAG_STS_SLEEP|SCH_TASK_FLAG_STS_SUSPENDED));
  
  /* Is woken up task higher priority than current task ? */
  if(taskIndex < u1_s_taskTCBIndex)
  {
    u1_s_taskTCBIndex = taskIndex;
    
    /* Set global task pointer to new task control block */
    tcb_g_p_nextTaskBlock = &SchTask_s_as_taskList[u1_s_taskTCBIndex];
    
    OS_CPU_TRIGGER_DISPATCHER();
  }
  
  vd_OSsch_unmaskInterrupts(u1_t_intMask);
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
  U1 u1_t_intMask;
  
  u1_t_intMask = u1_OSsch_maskInterrupts();
  
  SchTask_s_as_taskList[u1_s_taskTCBIndex].flags |= (U1)SCH_TASK_FLAG_STS_SUSPENDED;
  
  /* Is task suspending itself or another task? */
  if(taskIndex == u1_s_taskTCBIndex)
  {
    /* Switch to an active task. Ticker keeps ticking. */
    vd_sch_main();
  }
  /* Resume tick interrupts and enable context switch interrupt. */
  vd_OSsch_unmaskInterrupts(u1_t_intMask);
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
  U1 u1_t_intMask;
  
  /* Don't let scheduler interrupt itself. Ticker keeps ticking. */
  u1_t_intMask = u1_OSsch_maskInterrupts();
  
  /* Not a yielding or blocking scheduler call */
  u1_s_tickFlg = (U1)SCH_TICK_FLAG_TRUE;
  ++u4_s_tickCntr;
  
  vd_sch_main();

  /* Resume tick interrupts and enable context switch interrupt. */
  vd_OSsch_unmaskInterrupts(u1_t_intMask);
}

/*************************************************************************/
/*  Function Name: vd_sch_main                                           */
/*  Purpose:       Scheduler algorithm called to check if task is ready. */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_sch_main(void)
{
  U1 u1_t_index;
  U1 u1_t_taskReady;
  U1 u1_t_yieldTask;
  
  u1_t_yieldTask = (U1)SCH_YIELD_TASK_NOT_SET;
  
  /* If current task has been set to sleep, reset current TCB number to pick new task */
  if(SchTask_s_as_taskList[u1_s_taskTCBIndex].flags & (U1)SCH_TASK_FLAG_STS_CHECK)
  {
    u1_s_taskTCBIndex = u1_s_numTasks;
  }
  
  /* Scheduler */
  for(u1_t_index = ZERO; u1_t_index < u1_s_numTasks; u1_t_index++)
  {
    /* Default to lowest priority */
    u1_t_taskReady = u1_s_numTasks;
    
    /* Task status handling. No flags set means task is ready. */
    if(SchTask_s_as_taskList[u1_t_index].flags & (U1)SCH_TASK_FLAG_STS_CHECK)
    {  
      if(SchTask_s_as_taskList[u1_t_index].flags & (U1)SCH_TASK_FLAG_STS_SUSPENDED)
      {
        /* Task is suspended. Do nothing. */
      }
      /* If task has just yielded, store in case no other task is ready */
      else if(SchTask_s_as_taskList[u1_t_index].flags & (U1)SCH_TASK_FLAG_STS_YIELD)
      {
        u1_t_yieldTask = u1_t_index;
        SchTask_s_as_taskList[u1_t_index].flags &= ~((U1)SCH_TASK_FLAG_STS_YIELD);
      }
      /* Sleep/wake handling if millisecond flag set to true. If no other flag is set, sleep flag must be set. */
      else if(u1_s_tickFlg == (U1)SCH_TICK_FLAG_TRUE)
      {
        /* Decrement sleep counter and check if zero */
        if(--SchTask_s_as_taskList[u1_t_index].sleepCntr == (U1)ZERO)
        {  
#if(RTOS_RESOURCES_CONFIGURED)          
          /* Task has timed out. Determine if due to manual sleep or resource */
          switch(SchTask_s_as_taskList[u1_t_index].flags & (U1)SCH_TASK_RESOURCE_SLEEP_CHECK_MASK)
          {
            /* Remove task from resource's blocked list */
  #if(RTOS_CFG_OS_MAILBOX_ENABLED == RTOS_CONFIG_TRUE)
            case (U1)SCH_TASK_FLAG_SLEEP_MBOX:
              vd_mbox_blockedTaskTimeout(SchTask_s_as_taskList[u1_t_index].resource);
              SchTask_s_as_taskList[u1_t_index].resource = (void*)NULL;
              break;
  #endif          
  #if(RTOS_CFG_OS_QUEUE_ENABLED == RTOS_CONFIG_TRUE)            
            case (U1)SCH_TASK_FLAG_SLEEP_QUEUE:
              vd_queue_blockedTaskTimeout(SchTask_s_as_taskList[u1_t_index].resource, u1_t_index);
              SchTask_s_as_taskList[u1_t_index].resource = (void*)NULL;
              break;
  #endif   
  #if(RTOS_CFG_OS_SEMAPHORE_ENABLED == RTOS_CONFIG_TRUE)            
            case (U1)SCH_TASK_FLAG_SLEEP_SEMA:
              vd_sema_blockedTimeout(SchTask_s_as_taskList[u1_t_index].resource, u1_t_index);
              SchTask_s_as_taskList[u1_t_index].resource = (void*)NULL;
              break;
  #endif         
  #if(RTOS_CFG_OS_FLAGS_ENABLED == RTOS_CONFIG_TRUE)            
            case (U1)SCH_TASK_FLAG_SLEEP_FLAGS:
              vd_flags_pendTimeout(SchTask_s_as_taskList[u1_t_index].resource);
              SchTask_s_as_taskList[u1_t_index].resource = (void*)NULL;
              break;
  #endif            
            case (U1)ZERO:
              /* Manual sleep time out */
              break;          
          }          
#endif /* Resources configured */
		
          SchTask_s_as_taskList[u1_t_index].wakeReason = (U1)SCH_TASK_WAKEUP_SLEEP_TIMEOUT;
          u1_t_taskReady = u1_t_index;
          SchTask_s_as_taskList[u1_t_index].flags &= ~((U1)(SCH_TASK_FLAG_STS_SLEEP | SCH_TASK_RESOURCE_SLEEP_CHECK_MASK));
        }
        else
        {
          //NOP
        }
        
      }
      else
      {
        //NOP
      }
      
    }  
    else
    {
      u1_t_taskReady = u1_t_index;
    }  

    /* Check if checked task is higher priority (lower index) than current task */
    if(u1_t_taskReady < u1_s_taskTCBIndex)
    {
      u1_s_taskTCBIndex = u1_t_taskReady;
    }
    else
    {
      //NOP
    }    
  }
  
#if (RTOS_CONFIG_BG_TASK == SCH_TRUE)
  /* Scheduler background task does not take priority over a yielding task */
  if((u1_s_taskTCBIndex == (u1_s_numTasks - 1)) && (u1_t_yieldTask != (U1)SCH_YIELD_TASK_NOT_SET))
  {
    u1_s_taskTCBIndex = u1_t_yieldTask;
  }
#endif

  /* Reset tick flag */
  u1_s_tickFlg = (U1)SCH_TICK_FLAG_FALSE;
  
  if(&SchTask_s_as_taskList[u1_s_taskTCBIndex] == tcb_g_p_currentTaskBlock)
  {  
    /* Do nothing, return to current task. */
  }
  else
  {
    /* Set global task pointer to new task control block */
    tcb_g_p_nextTaskBlock = &SchTask_s_as_taskList[u1_s_taskTCBIndex];

    /* Set bit for pendSV to run when CPU is ready */
    OS_CPU_TRIGGER_DISPATCHER();
  }
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
    /* pre-sleep hook function goes here */
    
#if(RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT == RTOS_CONFIG_TRUE)
    for(u1_t_index = ZERO; u1_t_index < u1_s_numTasks; u1_t_index++)
    {
      if(u1_sch_checkStack(u1_t_index))
      {
        OSTaskFault();
      }
    }
#endif
    
#if(RTOS_CONFIG_ENABLE_BACKGROUND_IDLE_SLEEP == RTOS_CONFIG_TRUE)
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
/*                                timeHandler module. (Has since been migrated into CPU IF.    */
/*                                                                                             */
/* 1.0                2/28/19     First implementation of pre-emptive scheduler.               */
/*                                                                                             */
/* 1.1                3/2/19      Changes to scheduling algorithm. Priority-based instead of   */
/*                                time-based, added blocking and yielding APIs for tasks.      */
/*                                                                                             */
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
/* 1.5                5/9/19      Added stack overflow detection.                              */
/*                                                                                             */
/* 1.6                5/9/19      Added u4_OSsch_taskSleepSetFreq to support task execution at */
/*                                set frequencies.                                             */
