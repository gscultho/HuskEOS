/*************************************************************************/
/*  File Name: queue.c                                                   */
/*  Purpose: FIFO services for application layer tasks.                  */
/*  Created by: Garrett Sculthorpe on 3/20/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

/* Each queue can have up to four blocked tasks given a scheduler index size
   of one byte. If a fifth task tries to access the fifo and is blocked, it
   replaces the most recently blocked task on the list and does not wake up
   the removed task. The removed task sleeps until it times out. */
   
/* If multiple dequeue tasks are blocked, when data is put into the queue 
   the task with the highest priority shall receive the data. The others
   will simply be unblocked. */ 
   
/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "queue.h"
#include "sch.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define QUEUE_SIZE_ONE_BYTE                   (8)
#define QUEUE_MAX_NUM_BLOCKED_TASKS           (4)
#define QUEUE_TASK_ID_SAVE_OFFSET             (1)
#define QUEUE_BLOCKED_TASK_LIST_CLEAR         (0)
#define QUEUE_BLOCKED_TASK_LIST_PARSE_MASK    (0x000000FF)
#define QUEUE_GET_PTR_START_INDEX             (0)
#define QUEUE_PUT_PTR_START_INDEX             (1)
#define QUEUE_BLOCK_PERIOD_NO_BLOCK           (0)
#define QUEUE_SEMA_NO_BLOCK                   (0)
#define QUEUE_NUM_VALID                       (0)

 
/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/
static Queue queue_queueList[FIFO_MAX_NUM_QUEUES];


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
static void vd_queue_addTaskToBlocked(U1 queueNum);
static void vd_queue_unblockWaitingTasks(U1 queueNum);
static U1   u1_queue_checkValidFIFO(U1 queueNum);


/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_queue_init                                         */
/*  Purpose:       Initialize FIFO module.                               */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_queue_init(void)
{
  U1 u1_t_queueIndex;
  U1 u1_t_dataIndex;
  
  for(u1_t_queueIndex = 0; u1_t_queueIndex < FIFO_MAX_NUM_QUEUES; u1_t_queueIndex++)
  {
    queue_queueList[u1_t_queueIndex].getPtr          = &queue_queueList[u1_t_queueIndex].data[QUEUE_GET_PTR_START_INDEX]; 
    queue_queueList[u1_t_queueIndex].putPtr          = &queue_queueList[u1_t_queueIndex].data[QUEUE_PUT_PTR_START_INDEX];    
    queue_queueList[u1_t_queueIndex].blockedTaskList = QUEUE_BLOCKED_TASK_LIST_CLEAR;  
    
    vd_OSsema_init(&queue_queueList[u1_t_queueIndex].bufferSema); /* Init semaphore to enabled. */
    
    for(u1_t_dataIndex = (U1)ZERO; u1_t_dataIndex < FIFO_QUEUE_LENGTH_WORDS; u1_t_dataIndex++)
    {
      queue_queueList[u1_t_queueIndex].data[u1_t_dataIndex] = (Q_MEM)ZERO;
    }
  }
}

/*************************************************************************/
/*  Function Name: u1_OSqueue_flushFifo                                  */
/*  Purpose:       Clear all values in a queue.                          */
/*  Arguments:     U1  u1_queueNum:                                      */
/*                     Index for queue to be flushed.                    */
/*                 U1* error:                                            */
/*                     Address to write error to.                        */
/*  Return:        FIFO_SUCCESS                    OR                    */
/*                 FIFO_FAILURE                                          */
/*************************************************************************/
U1 u1_OSqueue_flushFifo(U1 queueNum, U1* error)
{
  U1 u1_t_dataIndex;
  U1 u1_t_semaSts;
  
  *error = u1_queue_checkValidFIFO(queueNum);
  
  if(*error)    
  {   
    return ((U1)FIFO_FAILURE);  
  }
  
  u1_t_semaSts = u1_OSsema_wait(&queue_queueList[queueNum].bufferSema, (U4)QUEUE_SEMA_NO_BLOCK); 
  
  if(!u1_t_semaSts)
  {
    *error = (U1)FIFO_ERR_QUEUE_IN_USE;
    return ((U1)FIFO_FAILURE);
  }
  
  queue_queueList[queueNum].getPtr = &queue_queueList[queueNum].data[QUEUE_GET_PTR_START_INDEX]; 
  queue_queueList[queueNum].putPtr = &queue_queueList[queueNum].data[QUEUE_PUT_PTR_START_INDEX];  
    
  for(u1_t_dataIndex = (U1)ZERO; u1_t_dataIndex < (U1)FIFO_QUEUE_LENGTH_WORDS; u1_t_dataIndex++)
  {
    queue_queueList[queueNum].data[u1_t_dataIndex] = (Q_MEM)ZERO;
  }
  
  vd_queue_unblockWaitingTasks(queueNum);
  
  u1_OSsema_post(&queue_queueList[queueNum].bufferSema);
  
  return ((U1)FIFO_SUCCESS);
}

/*************************************************************************/
/*  Function Name: data_OSqueue_get                                      */
/*  Purpose:       Get data from queue.                                  */
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
  Q_MEM  Q_MEM_t_data;
  Q_MEM* Q_MEM_t_p_nextGetPtr;
  
  *error = u1_queue_checkValidFIFO(queueNum);
  
  if(*error)    
  {   
    return ((U1)FIFO_FAILURE);  
  }
  
  /* If semaphore taken by other task */
  if(!(u1_OSsema_wait(&queue_queueList[queueNum].bufferSema, (U4)QUEUE_SEMA_NO_BLOCK)))
  {
    *error = (U1)FIFO_ERR_QUEUE_IN_USE;
    
    if(blockPeriod != (U4)QUEUE_BLOCK_PERIOD_NO_BLOCK)
    {
      vd_queue_addTaskToBlocked(queueNum);
      vd_sch_setReasonForSleep(&queue_queueList[queueNum], (U1)SCH_TASK_SLEEP_RESOURCE_QUEUE);
      vd_OSsch_taskSleep(blockPeriod); 
    }
    
    return ((U4)FIFO_FAILURE);
  }    
  
  /* Check space ahead of getPtr */
  if(queue_queueList[queueNum].getPtr == &queue_queueList[queueNum].data[FIFO_QUEUE_LENGTH_WORDS - 1])
  {
    Q_MEM_t_p_nextGetPtr = &queue_queueList[queueNum].data[ZERO];
  }
  else
  {
    Q_MEM_t_p_nextGetPtr = queue_queueList[queueNum].getPtr + 1;
  }
  
  /* If queue is empty */
  if(Q_MEM_t_p_nextGetPtr == queue_queueList[queueNum].putPtr)
  {    
    *error = (U1)FIFO_ERR_QUEUE_EMPTY;
    
    u1_OSsema_post(&queue_queueList[queueNum].bufferSema);
    
    /* Block task if blocking enabled */
    if(blockPeriod != (U4)QUEUE_BLOCK_PERIOD_NO_BLOCK)
    {
      vd_queue_addTaskToBlocked(queueNum);
      vd_sch_setReasonForSleep(&queue_queueList[queueNum], (U1)SCH_TASK_SLEEP_RESOURCE_QUEUE);
      vd_OSsch_taskSleep(blockPeriod);   
    }
    
    return ((U4)FIFO_FAILURE);    
  }
  
  /* Else move pointer */
  queue_queueList[queueNum].getPtr = Q_MEM_t_p_nextGetPtr;
  
  /* Dequeue and clear entry */
  Q_MEM_t_data = *(queue_queueList[queueNum].getPtr);
  *(queue_queueList[queueNum].getPtr) = (U1)ZERO;
  
  if(queue_queueList[queueNum].blockedTaskList != (U4)ZERO)
  {
    vd_queue_unblockWaitingTasks(queueNum);
  }
  
  u1_OSsema_post(&queue_queueList[queueNum].bufferSema);
  
  return (Q_MEM_t_data);
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
    return ((U1)FIFO_FAILURE);  
  }
  
  u1_t_sts = (U1)FIFO_STS_QUEUE_READY;
  
  OS_CPU_ENTER_CRITICAL();
  
  /* Check space ahead of getPtr */
  if(queue_queueList[queueNum].getPtr == &queue_queueList[queueNum].data[FIFO_QUEUE_LENGTH_WORDS - 1])
  {
    Q_MEM_t_p_nextGetPtr = &queue_queueList[queueNum].data[(U1)ZERO];
  }
  else
  {
    Q_MEM_t_p_nextGetPtr = queue_queueList[queueNum].getPtr + 1;
  }
  
  if(Q_MEM_t_p_nextGetPtr == queue_queueList[queueNum].putPtr)
  {
    u1_t_sts = (U1)FIFO_STS_QUEUE_EMPTY;    
  }
  
  if(queue_queueList[queueNum].putPtr == (queue_queueList[queueNum].getPtr))
  {
    u1_t_sts = (U1)FIFO_STS_QUEUE_FULL;    
  }
  
  OS_CPU_EXIT_CRITICAL();
  
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
  *error = u1_queue_checkValidFIFO(queueNum);
  
  if(*error)    
  {   
    return ((U1)FIFO_FAILURE);  
  }
  
  /* If semaphore taken by other task */
  if(!(u1_OSsema_wait(&queue_queueList[queueNum].bufferSema, (U4)QUEUE_SEMA_NO_BLOCK)))
  {
    *error = (U1)FIFO_ERR_QUEUE_IN_USE;
    
    /* Block if blocking is enabled */
     if(blockPeriod != (U4)QUEUE_BLOCK_PERIOD_NO_BLOCK)
     {
       vd_queue_addTaskToBlocked(queueNum);
       vd_sch_setReasonForSleep(&queue_queueList[queueNum], (U1)SCH_TASK_SLEEP_RESOURCE_QUEUE);
       vd_OSsch_taskSleep(blockPeriod); 
     }
     /* Else return immediately */
     return ((U1)FIFO_FAILURE);
  }
  
  /* If queue is full */
  if(queue_queueList[queueNum].putPtr == (queue_queueList[queueNum].getPtr))
  {
    *error = (U1)FIFO_ERR_QUEUE_FULL;
    
    u1_OSsema_post(&queue_queueList[queueNum].bufferSema);
    
    /* Block if blocking is enabled */
    if(blockPeriod != (U4)QUEUE_BLOCK_PERIOD_NO_BLOCK)
    {
      vd_queue_addTaskToBlocked(queueNum);
      vd_sch_setReasonForSleep(&queue_queueList[queueNum], (U1)SCH_TASK_SLEEP_RESOURCE_QUEUE);
      vd_OSsch_taskSleep(blockPeriod);
    }
    /* Else return immediately */
    return ((U1)FIFO_FAILURE);
  }
  
  /* Puts data in queue */
  *(queue_queueList[queueNum].putPtr) = message;
  
  /* Shift pointers */
  if(queue_queueList[queueNum].putPtr == &queue_queueList[queueNum].data[FIFO_QUEUE_LENGTH_WORDS - 1])
  {
    queue_queueList[queueNum].putPtr = &queue_queueList[queueNum].data[ZERO];
  }
  else
  {
    ++queue_queueList[queueNum].putPtr;
  }
  
  /* Check if tasks need to be woken */
  if(queue_queueList[queueNum].blockedTaskList != (U4)ZERO)
  {
    vd_queue_unblockWaitingTasks(queueNum);
  }
  
  u1_OSsema_post(&queue_queueList[queueNum].bufferSema);
  
  return ((U1)FIFO_SUCCESS);
}

/*************************************************************************/
/*  Function Name: vd_OSqueue_getNumInFIFO                               */
/*  Purpose:       Return number of items in buffer.                     */
/*  Arguments:     U1  queueNum:                                         */
/*                     Queue index being referenced.                     */
/*                 U1* error:                                            */
/*                     Address to write error to.                        */
/*  Return:        U1  u1_t_count:                                       */
/*                     Number of entries in queue                        */
/*************************************************************************/
U1 vd_OSqueue_getNumInFIFO(U1 queueNum, U1* error)
{
  U1 u1_t_count;
  S1 s1_t_getPutDiff;
  
  *error = u1_queue_checkValidFIFO(queueNum);
  
  if(*error)    
  {   
    return ((U1)FIFO_FAILURE);  
  }
  
  s1_t_getPutDiff = queue_queueList[queueNum].getPtr - queue_queueList[queueNum].putPtr;
  
  /* Queue is not empty */
  if(s1_t_getPutDiff < (U1)ZERO)
  {
    u1_t_count = (S1)FIFO_QUEUE_LENGTH_WORDS - s1_t_getPutDiff;
  }
  else if(s1_t_getPutDiff > (U1)ZERO)
  {
    u1_t_count = s1_t_getPutDiff;
  }
  /* Queue is empty */
  else
  {
    u1_t_count = (U1)FIFO_STS_QUEUE_EMPTY;
  }
  
  return (u1_t_count);
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
  
  return((U1)QUEUE_NUM_VALID);
}

/*************************************************************************/
/*  Function Name: vd_queue_blockedTaskTimeout                           */
/*  Purpose:       Update block list if a task times out on its block.   */
/*  Arguments:     void* queueAddr                                       */
/*                    Address of queue structure.                        */
/*                 U1 taskID:                                            */
/*                    Queue index number.                                */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_queue_blockedTaskTimeout(void* queueAddr, U1 taskID)
{
  U1     u1_t_index;
  U4     u4_t_taskMask;
  Queue* queue;
  
  queue         = (Queue*)queueAddr;  
  u4_t_taskMask = (taskID + (U1)QUEUE_TASK_ID_SAVE_OFFSET);
  
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)QUEUE_MAX_NUM_BLOCKED_TASKS; u1_t_index++)
  {
    /* Shift to spot on list that is about to be checked */
    u4_t_taskMask <<= (((U1)QUEUE_SIZE_ONE_BYTE)*u1_t_index);
    
    /* Check if task already waiting on this mailbox */
    if((queue->blockedTaskList & u4_t_taskMask) == u4_t_taskMask)
    {
      queue->blockedTaskList &= ~u4_t_taskMask;
      break;
    }
  }
}

/*************************************************************************/
/*  Function Name: vd_queue_addTaskToBlocked                             */
/*  Purpose:       Add task ID (offset by one) to block list.            */
/*  Arguments:     U1 queueNum:                                          */
/*                    Queue index being referenced.                      */
/*  Return:        void                                                  */
/*************************************************************************/
static void vd_queue_addTaskToBlocked(U1 queueNum)
{
  U1 u1_t_index;
  U4 u4_t_mask;
  U4 u4_t_addTaskMask;
  
  u4_t_mask = (U4)QUEUE_BLOCKED_TASK_LIST_PARSE_MASK;
  
  u4_t_addTaskMask = u1_OSsch_getCurrentTask() + (U1)QUEUE_TASK_ID_SAVE_OFFSET;
  
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)QUEUE_MAX_NUM_BLOCKED_TASKS; u1_t_index++)
  {
    /* Shift to spot on list that is about to be checked */
    u4_t_addTaskMask <<= (((U1)QUEUE_SIZE_ONE_BYTE)*u1_t_index);
    
    /* Check if task already waiting on this queue */
    if((queue_queueList[queueNum].blockedTaskList & u4_t_addTaskMask) == u4_t_addTaskMask)
    {
      break;
    }
    
    /* Check if this spot on list is occupied */
    if(!(queue_queueList[queueNum].blockedTaskList & u4_t_mask))
    {  
      /* Add current task to this spot on list */
      queue_queueList[queueNum].blockedTaskList |= u4_t_addTaskMask;
      break;
    }
    
    u4_t_mask <<= (U1)QUEUE_SIZE_ONE_BYTE;
  }
}

/*************************************************************************/
/*  Function Name: vd_queue_unblockWaitingTasks                          */
/*  Purpose:       Remove all task IDs (offset by one) from list.        */
/*  Arguments:     U1 queueNum:                                          */
/*                    Queue index being referenced.                      */
/*  Return:        void                                                  */
/*************************************************************************/
static void vd_queue_unblockWaitingTasks(U1 queueNum)
{
  U1 u1_t_index;
  U4 u4_t_mask;
  U4 u4_t_taskMask;
  
  u4_t_mask = (U4)QUEUE_BLOCKED_TASK_LIST_PARSE_MASK;
  
  for(u1_t_index = (U1)ZERO; u1_t_index < QUEUE_MAX_NUM_BLOCKED_TASKS; u1_t_index++)
  {
    u4_t_taskMask = (queue_queueList[queueNum].blockedTaskList & u4_t_mask);
    
    /* Check if task is in spot on blocked list */
    if(u4_t_taskMask == (U1)ZERO)
    {

    }
    else
    {
      /* Remove task from list and wake up task */
      queue_queueList[queueNum].blockedTaskList &= ~u4_t_taskMask;
      vd_sch_setReasonForWakeup((U1)SCH_TASK_WAKEUP_QUEUE_READY, ((U1)(u4_t_taskMask - (U4)QUEUE_TASK_ID_SAVE_OFFSET)));
    }
    
    /* Move to next spot on list */
    u4_t_mask <<= (U1)QUEUE_SIZE_ONE_BYTE;
  }
}

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                3/25/19     First implementation of module.                              */
/*                                                                                             */
