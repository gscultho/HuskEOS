/*************************************************************************/
/*  File Name: queue.c                                                   */
/*  Purpose: FIFO services for application layer tasks.                  */
/*  Created by: Garrett Sculthorpe on 3/20/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/
   
/* If multiple dequeue tasks are blocked, when data is put into the queue 
   the task with the highest priority shall receive the data. */ 
   
#include "rtos_cfg.h"

#if(RTOS_CFG_OS_QUEUE_ENABLED == RTOS_CONFIG_TRUE)

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "listMgr_internal.h"
#include "queue_internal_IF.h"
#include "queue.h"
#include "sch_internal_IF.h"
#include "sch.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define QUEUE_MAX_NUM_BLOCKED_TASKS           (RTOS_CFG_MAX_NUM_BLOCKED_TASKS_FIFO)
#define QUEUE_GET_PTR_START_INDEX             (0)
#define QUEUE_PUT_PTR_START_INDEX             (1)
#define QUEUE_BLOCK_PERIOD_NO_BLOCK           (0)
#define QUEUE_NULL_PTR                        ((void*)0)
 
/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/
static Queue queue_queueList[FIFO_MAX_NUM_QUEUES];


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
static void vd_OSqueue_addTaskToBlocked(U1 queueNum);
static void vd_queue_unblockWaitingTasks(U1 queueNum);
static U1   u1_queue_checkValidFIFO(U1 queueNum);


/*************************************************************************/

/*************************************************************************/
/*  Function Name: u1_OSqueue_init                                       */
/*  Purpose:       Initialize FIFO and provide ID number.                */
/*  Arguments:     Q_MEM* queueStart:                                    */
/*                        Pointer to first address allocated for queue.  */
/*                 U4 queueLength:                                       */
/*                        Length of queue. Number of Q_MEM entries.      */
/*  Return:        U1: FIFO_FAILURE   OR                                 */
/*                     queue ID number.                                  */
/*************************************************************************/
U1 u1_OSqueue_init(Q_MEM* queueStart, U4 queueLength)
{
         U1 u1_t_index;
         U1 u1_t_return;
  static U1 u1_s_numQueuesAllocated = (U1)ZERO;
  
  /* Check that there is available overhead for new queue. */
  if(u1_s_numQueuesAllocated < (U1)FIFO_MAX_NUM_QUEUES)
  {
    /* Return queue ID number. */
    u1_t_return = u1_s_numQueuesAllocated;
    
    OS_SCH_ENTER_CRITICAL();
    
    queue_queueList[u1_s_numQueuesAllocated].startPtr                        = queueStart;
    queue_queueList[u1_s_numQueuesAllocated].endPtr                          = queueStart + queueLength - (U1)ONE;
    queue_queueList[u1_s_numQueuesAllocated].getPtr                          = &queue_queueList[u1_s_numQueuesAllocated].startPtr[QUEUE_GET_PTR_START_INDEX]; /* Offset 0 from start. */
    queue_queueList[u1_s_numQueuesAllocated].putPtr                          = &queue_queueList[u1_s_numQueuesAllocated].startPtr[QUEUE_PUT_PTR_START_INDEX]; /* Offset 1 from start. */
    queue_queueList[u1_s_numQueuesAllocated].blockedTaskList.blockedListHead = QUEUE_NULL_PTR;

    for(u1_t_index = (U1)ZERO; u1_t_index < (U1)RTOS_CFG_MAX_NUM_BLOCKED_TASKS_FIFO; u1_t_index++)
    {
      queue_queueList[u1_s_numQueuesAllocated].blockedTaskList.blockedTasks[u1_t_index].nextNode     = QUEUE_NULL_PTR;
      queue_queueList[u1_s_numQueuesAllocated].blockedTaskList.blockedTasks[u1_t_index].previousNode = QUEUE_NULL_PTR;
      queue_queueList[u1_s_numQueuesAllocated].blockedTaskList.blockedTasks[u1_t_index].TCB          = QUEUE_NULL_PTR;
    }
    
    ++u1_s_numQueuesAllocated;
    
    OS_SCH_EXIT_CRITICAL();
  }
  else
  {
    u1_t_return = (U1)FIFO_FAILURE;
  }
  
  return (u1_t_return);
}

/*************************************************************************/
/*  Function Name: u1_OSqueue_flushFifo                                  */
/*  Purpose:       Clear all values in a queue.                          */
/*  Arguments:     U1  u1_queueNum:                                      */
/*                     ID of queue to be flushed.                        */
/*                 U1* error:                                            */
/*                     Address to write error to.                        */
/*  Return:        FIFO_SUCCESS                    OR                    */
/*                 FIFO_FAILURE                                          */
/*************************************************************************/
U1 u1_OSqueue_flushFifo(U1 queueNum, U1* error)
{
  Q_MEM* data_t_p_dataPtr;
  U1     u1_t_return;
  
  *error = u1_queue_checkValidFIFO(queueNum);
  
  if(*error)    
  {   
    u1_t_return = (U1)FIFO_FAILURE;  
  }
  else
  {
    OS_SCH_ENTER_CRITICAL();
    
    queue_queueList[queueNum].getPtr = queue_queueList[queueNum].startPtr; 
    queue_queueList[queueNum].putPtr = queue_queueList[queueNum].startPtr;  
    data_t_p_dataPtr                 = queue_queueList[queueNum].startPtr;  
      
    while(data_t_p_dataPtr != (Q_MEM*)queue_queueList[queueNum].endPtr)
    {
      *data_t_p_dataPtr = (Q_MEM)ZERO;
      ++data_t_p_dataPtr;
    }
    
    /* Wake all blocked tasks. */
    while(queue_queueList[queueNum].blockedTaskList.blockedListHead != QUEUE_NULL_PTR)
    {
      vd_queue_unblockWaitingTasks(queueNum);
    }
    
    OS_SCH_EXIT_CRITICAL();
    
    u1_t_return = (U1)FIFO_SUCCESS;
  }
  
  return (u1_t_return);
}

/*************************************************************************/
/*  Function Name: data_OSqueue_get                                      */
/*  Purpose:       Get data from queue. If higher priority task is       */
/*                 waiting to send data, it will preempt this task.      */
/*  Arguments:     U1  queueNum:                                         */
/*                     Queue index being referenced.                     */
/*                 U4  blockPeriod:                                      */
/*                     Sleep timeout period if task is blocked.          */
/*                 U1* error:                                            */
/*                     Address to write error to.                        */
/*  Return:        Q_MEM FIFO_FAILURE         OR                         */
/*                       Q_MEM_t_data                                    */
/*************************************************************************/
Q_MEM data_OSqueue_get(U1 queueNum, U4 blockPeriod, U1* error)
{
  Q_MEM* data_t_p_nextGetPtr;
  Q_MEM  data_t_return;
  
  *error = u1_queue_checkValidFIFO(queueNum);
  
  if(*error)    
  {   
    data_t_return = (Q_MEM)FIFO_FAILURE;  
  }
  else
  {   
    OS_SCH_ENTER_CRITICAL();
    
    /* Check space ahead of getPtr */
    if(queue_queueList[queueNum].getPtr == queue_queueList[queueNum].endPtr)
    {
      data_t_p_nextGetPtr = queue_queueList[queueNum].startPtr;
    }
    else
    {
      data_t_p_nextGetPtr = queue_queueList[queueNum].getPtr + (U1)ONE;
    }
    
    /* If queue is empty */
    if(data_t_p_nextGetPtr == queue_queueList[queueNum].putPtr)
    {    
      *error = (U1)FIFO_ERR_QUEUE_EMPTY;
      
      /* Block task if blocking enabled */
      if(blockPeriod != (U4)QUEUE_BLOCK_PERIOD_NO_BLOCK)
      {
        vd_OSqueue_addTaskToBlocked(queueNum);
        vd_OSsch_setReasonForSleep(&queue_queueList[queueNum], (U1)SCH_TASK_SLEEP_RESOURCE_QUEUE);
        vd_OSsch_taskSleep(blockPeriod); 
        
        /* Let task enter sleep state. */
        OS_SCH_EXIT_CRITICAL();
        
        /* When task wakes back up, check again. */
        OS_SCH_ENTER_CRITICAL(); 

        /* If queue is no longer empty. */
        if(data_t_p_nextGetPtr != queue_queueList[queueNum].putPtr)
        {  
          *error = (Q_MEM)FIFO_ERR_NO_ERROR;
          
          /* Move pointer */
          queue_queueList[queueNum].getPtr = data_t_p_nextGetPtr;
          
          /* Dequeue and clear entry */
          data_t_return                       = *(queue_queueList[queueNum].getPtr);
          *(queue_queueList[queueNum].getPtr) = (U1)ZERO;
          
          /* Unblock highest priority task that is blocked. */
          if(queue_queueList[queueNum].blockedTaskList.blockedListHead != QUEUE_NULL_PTR)
          {
            vd_queue_unblockWaitingTasks(queueNum);
          }
          else
          {
            
          }
        }
        else
        {
          /* Don't block since task was already blocked. */ 
          data_t_return = (Q_MEM)FIFO_FAILURE; 
        }   
      }
      else
      {
        /* Non-blocking. Return to task */
        data_t_return = (Q_MEM)FIFO_FAILURE; 
      }/* blockPeriod != (U4)QUEUE_BLOCK_PERIOD_NO_BLOCK */    
    }
    else
    {        
      /* Else move pointer */
      queue_queueList[queueNum].getPtr = data_t_p_nextGetPtr;
      
      /* Dequeue and clear entry */
      data_t_return                       = *(queue_queueList[queueNum].getPtr);
      *(queue_queueList[queueNum].getPtr) = (U1)ZERO;
      
      /* Unblock highest priority task that is blocked. */
      if(queue_queueList[queueNum].blockedTaskList.blockedListHead != QUEUE_NULL_PTR)
      {
        vd_queue_unblockWaitingTasks(queueNum);
      }
      else
      {
        
      }
    }/* data_t_p_nextGetPtr == queue_queueList[queueNum].putPtr */
    
    OS_SCH_EXIT_CRITICAL();
    
  }/* if(*error)  */
  
  return (data_t_return);
}

/*************************************************************************/
/*  Function Name: u1_OSqueue_getSts                                     */
/*  Purpose:       Check if queue is ready, full, or empty.              */
/*  Arguments:     U1  queueNum:                                         */
/*                     Queue index being referenced.                     */
/*                 U1* error:                                            */
/*                     Address to write error to.                        */
/*  Return:        U1 FIFO_STS_QUEUE_FULL     OR                         */
/*                    FIFO_FAILURE            OR                         */
/*                    FIFO_STS_QUEUE_EMPTY    OR                         */
/*                    FIFO_STS_QUEUE_READY                               */
/*************************************************************************/
U1 u1_OSqueue_getSts(U1 queueNum, U1* error)
{
  U1     u1_t_sts;
  Q_MEM* Q_MEM_t_p_nextGetPtr;
  
  *error = u1_queue_checkValidFIFO(queueNum);
  
  if(*error)    
  {   
    u1_t_sts = (U1)FIFO_FAILURE;  
  }
  else
  {    
    OS_CPU_ENTER_CRITICAL();
    
    /* Check space ahead of getPtr */
    if(queue_queueList[queueNum].getPtr == queue_queueList[queueNum].endPtr)
    {
      Q_MEM_t_p_nextGetPtr = queue_queueList[queueNum].startPtr;
    }
    else
    {
      Q_MEM_t_p_nextGetPtr = queue_queueList[queueNum].getPtr + (U1)ONE;
    }
    
    /* Get status */
    if(Q_MEM_t_p_nextGetPtr == queue_queueList[queueNum].putPtr)
    {
      u1_t_sts = (U1)FIFO_STS_QUEUE_EMPTY;    
    } 
    else if(queue_queueList[queueNum].putPtr == (queue_queueList[queueNum].getPtr))
    {
      u1_t_sts = (U1)FIFO_STS_QUEUE_FULL;    
    }
    else
    {
      u1_t_sts = (U1)FIFO_STS_QUEUE_READY;
    }
    
    OS_CPU_EXIT_CRITICAL();
  }
  
  return (u1_t_sts);
}

/*************************************************************************/
/*  Function Name: u1_OSqueue_put                                        */
/*  Purpose:       Put data in queue if not full.                        */
/*  Arguments:     U1  queueNum:                                         */
/*                     Queue index being referenced.                     */
/*                 U4  blockPeriod:                                      */
/*                     Sleep timeout period if task is blocked.          */
/*                 U4  message:                                          */
/*                     Data to be added to queue.                        */
/*                 U1* error:                                            */
/*                     Address to write error to.                        */
/*  Return:        U1 FIFO_QUEUE_FULL            OR                      */
/*                    FIFO_QUEUE_PUT_SUCCESS                             */
/*************************************************************************/
U1 u1_OSqueue_put(U1 queueNum, U4 blockPeriod, Q_MEM message, U1* error)
{
  U1 u1_t_return;
  
  *error = u1_queue_checkValidFIFO(queueNum);
  
  if(*error)    
  {   
    u1_t_return = (U1)FIFO_FAILURE;  
  }
  else
  {    
    OS_SCH_ENTER_CRITICAL();
    
    /* If queue is full */
    if(queue_queueList[queueNum].putPtr == (queue_queueList[queueNum].getPtr))
    {
      *error = (U1)FIFO_ERR_QUEUE_FULL;

      /* Block if blocking is enabled */
      if(blockPeriod != (U4)QUEUE_BLOCK_PERIOD_NO_BLOCK)
      {
        vd_OSqueue_addTaskToBlocked(queueNum);
        vd_OSsch_setReasonForSleep(&queue_queueList[queueNum], (U1)SCH_TASK_SLEEP_RESOURCE_QUEUE);
        vd_OSsch_taskSleep(blockPeriod);
        
        /* Let task enter sleep state. */
        OS_SCH_EXIT_CRITICAL();
        
        /* Check if queue is full after task wakes up. */
        OS_SCH_ENTER_CRITICAL();
        
        if(queue_queueList[queueNum].putPtr != (queue_queueList[queueNum].getPtr)) /* Spot is available. */
        {
          *error = (Q_MEM)FIFO_ERR_NO_ERROR;
          
          /* Puts data in queue */
          *(queue_queueList[queueNum].putPtr) = message;
          
          /* Shift pointer */
          if(queue_queueList[queueNum].putPtr == queue_queueList[queueNum].endPtr)
          {
            queue_queueList[queueNum].putPtr = queue_queueList[queueNum].startPtr;
          }
          else
          {
            ++queue_queueList[queueNum].putPtr;
          }
          
          /* Check if tasks need to be woken */
          if(queue_queueList[queueNum].blockedTaskList.blockedListHead != QUEUE_NULL_PTR)
          {
            vd_queue_unblockWaitingTasks(queueNum);
          }
          
          u1_t_return = (Q_MEM)FIFO_SUCCESS;           
        }
        else
        {
          /* Don't block again since this is the second check. */
          u1_t_return = (U1)FIFO_FAILURE; 
        }/* queue_queueList[queueNum].putPtr != (queue_queueList[queueNum].getPtr) */
      }
      else 
      {
        /* API call is non-blocking, don't block. */
        u1_t_return = (U1)FIFO_FAILURE; 
      } /* queue_queueList[queueNum].putPtr != (queue_queueList[queueNum].getPtr) */
    }
    else
    {
      u1_t_return = (U1)FIFO_QUEUE_PUT_SUCCESS;
      
      /* Puts data in queue */
      *(queue_queueList[queueNum].putPtr) = message;
      
      /* Shift pointer */
      if(queue_queueList[queueNum].putPtr == queue_queueList[queueNum].endPtr)
      {
        queue_queueList[queueNum].putPtr = queue_queueList[queueNum].startPtr;
      }
      else
      {
        ++queue_queueList[queueNum].putPtr;
      }
      
      /* Check if tasks need to be woken */
      if(queue_queueList[queueNum].blockedTaskList.blockedListHead != QUEUE_NULL_PTR)
      {
        vd_queue_unblockWaitingTasks(queueNum);
      }
    }/* queue_queueList[queueNum].putPtr == (queue_queueList[queueNum].getPtr) */
    
    OS_SCH_EXIT_CRITICAL();
    
  }/* if(*error) */
  
  return (u1_t_return);
}

/*************************************************************************/
/*  Function Name: u4_OSqueue_getNumInFIFO                               */
/*  Purpose:       Return number of items in buffer.                     */
/*  Arguments:     U1  queueNum:                                         */
/*                     Queue index being referenced.                     */
/*                 U1* error:                                            */
/*                     Address to write error to.                        */
/*  Return:        U4  u4_t_count:                                       */
/*                     FIFO_FAILURE         OR                           */
/*                     Number of entries in queue                        */
/*************************************************************************/
U4 u4_OSqueue_getNumInFIFO(U1 queueNum, U1* error)
{
  U4 u4_t_count;
  
  *error = u1_queue_checkValidFIFO(queueNum);
  
  if(*error)    
  {   
    u4_t_count = (U4)FIFO_FAILURE;  
  }
  else
  {    
    OS_SCH_ENTER_CRITICAL();
    
    if(queue_queueList[queueNum].putPtr > queue_queueList[queueNum].getPtr)
    {
      u4_t_count = queue_queueList[queueNum].putPtr - queue_queueList[queueNum].getPtr;
    }
    else
    {
      u4_t_count = (queue_queueList[queueNum].putPtr - queue_queueList[queueNum].startPtr) + (queue_queueList[queueNum].endPtr - queue_queueList[queueNum].getPtr);
    }
    
    OS_SCH_EXIT_CRITICAL();
    
  } 
  
  return (u4_t_count);
}

/*************************************************************************/
/*  Function Name: vd_OSqueue_blockedTaskTimeout                         */
/*  Purpose:       Update block list if a task times out on its block.   */
/*                 Called internally by scheduler.                       */
/*  Arguments:     void* queueAddr                                       */
/*                    Address of queue structure.                        */
/*                 Sch_Task* taskTCB:                                    */
/*                    TCB address of blocked task.                       */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSqueue_blockedTaskTimeout(void* queueAddr, struct Sch_Task* taskTCB)
{
  ListNode* node_t_tempPtr;
  
  OS_SCH_ENTER_CRITICAL();
  
  /* Remove node from block list and clear its contents */
  node_t_tempPtr      = node_list_removeNodeByTCB((&((OSQueue*)queueAddr)->blockedTaskList.blockedListHead), taskTCB);
  node_t_tempPtr->TCB = QUEUE_NULL_PTR;
  
  OS_SCH_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: u1_queue_checkValidFIFO                               */
/*  Purpose:       Return if queue index is valid or invalid.            */
/*  Arguments:     U1 queueNum:                                          */
/*                    Queue index being referenced.                      */
/*  Return:        FIFO_ERR_QUEUE_OUT_OF_RANGE          OR               */
/*                 QUEUE_NUM_VALID                                       */
/*************************************************************************/
static U1 u1_queue_checkValidFIFO(U1 queueNum)
{
  /* unsigned, check only equal to or above max */
  if(queueNum >= (U1)FIFO_MAX_NUM_QUEUES)    
  {    
    return ((U1)FIFO_ERR_QUEUE_OUT_OF_RANGE);  
  } 
  
  return((U1)FIFO_ERR_NO_ERROR);
}

/*************************************************************************/
/*  Function Name: vd_OSqueue_addTaskToBlocked                           */
/*  Purpose:       Add task to block list.                               */
/*  Arguments:     U1 queueNum:                                          */
/*                    Queue index being referenced.                      */
/*  Return:        void                                                  */
/*************************************************************************/
static void vd_OSqueue_addTaskToBlocked(U1 queueNum)
{
  U1 u1_t_index;
  
  u1_t_index = (U1)ZERO;
  
  /* Find available node to store data */
  while((queue_queueList[queueNum].blockedTaskList.blockedTasks[u1_t_index].TCB != QUEUE_NULL_PTR) && (u1_t_index < (U1)QUEUE_MAX_NUM_BLOCKED_TASKS)) 
  {    
    ++u1_t_index;
  }
  /* If node found, then store TCB pointer and add to blocked list */
  if(u1_t_index < (U1)QUEUE_MAX_NUM_BLOCKED_TASKS)
  {
    queue_queueList[queueNum].blockedTaskList.blockedTasks[u1_t_index].TCB = SCH_CURRENT_TCB_ADDR;
    vd_list_addTaskByPrio(&(queue_queueList[queueNum].blockedTaskList.blockedListHead), &(queue_queueList[queueNum].blockedTaskList.blockedTasks[u1_t_index]));
  }
}

/*************************************************************************/
/*  Function Name: vd_queue_unblockWaitingTasks                          */
/*  Purpose:       Remove highest priority task from blocked list & wake.*/
/*  Arguments:     U1 queueNum:                                          */
/*                    Queue index being referenced.                      */
/*  Return:        void                                                  */
/*************************************************************************/
static void vd_queue_unblockWaitingTasks(U1 queueNum)
{
  ListNode* node_t_p_highPrioTask;
  
  /* If blocked list is not empty */

  /* Remove highest priority task */    
  node_t_p_highPrioTask = node_list_removeFirstNode(&(queue_queueList[queueNum].blockedTaskList.blockedListHead));
  
  /*  Notify scheduler the reason that task is going to be woken. */    
  vd_OSsch_setReasonForWakeup((U1)SCH_TASK_WAKEUP_QUEUE_READY, node_t_p_highPrioTask->TCB);
  
  /* Notify scheduler to change task state. If woken task is higher priority than running task, context switch will occur after critical section. */
  vd_OSsch_taskWake(node_t_p_highPrioTask->TCB->taskID);
  
  /* Clear TCB pointer. This frees this node for future use. */
  node_t_p_highPrioTask->TCB = QUEUE_NULL_PTR;   
}

#endif /* Conditional compile */

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                3/25/19     First implementation of module.                              */
/*                                                                                             */
/* 1.0                8/5/19      Rewrite to support better flow and blocked task handling.    */
/*                                                                                             */
/* 1.1                8/14/19     Some bug fixes. Data being overwritten.                      */
