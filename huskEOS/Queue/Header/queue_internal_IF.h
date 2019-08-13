/*************************************************************************/
/*  File Name:  queue_internal_IF.h                                      */
/*  Purpose:    Kernel access definitions and routines for FIFO.         */
/*  Created by: Garrett Sculthorpe on 5/25/19                            */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

#ifndef queue_internal_IF_h 
#define queue_internal_IF_h

#include "cpu_defs.h"
#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define FIFO_QUEUE_LENGTH_WORDS              (RTOS_CFG_BUFFER_LENGTH) 
#define Q_MEM                                 RTOS_CFG_BUFFER_DATA

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
struct listNode; /* Forward declaration. Defined in "listMgr_internal.h" */

typedef struct blockedList
{
  struct ListNode  blockedTasks[RTOS_CFG_MAX_NUM_BLOCKED_TASKS_FIFO];
  struct ListNode* blockedListHead;
}
blockedList;

typedef struct Queue
{
  Q_MEM*      startPtr;          /* First memory address of FIFO. */
  Q_MEM*      endPtr;            /* Last memory address of FIFO. */
  Q_MEM*      putPtr;            /* Next data sent will be put here. */
  Q_MEM*      getPtr;            /* Next get() call will take data from here. */
  blockedList blockedTaskList;   /* Structure to track blocked tasks. */
}
Queue;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_OSqueue_blockedTaskTimeout(void* queueAddr, struct Sch_Task* taskTCB);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
