/*************************************************************************/
/*  File Name:  queue.h                                                  */
/*  Purpose:    Main header file for queue module.                       */
/*  Created by: Garrett Sculthorpe on 3/16/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

#ifndef queue_h 
#if(RTOS_CFG_OS_QUEUE_ENABLED == RTOS_CONFIG_TRUE)
#define queue_h

#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define FIFO_MAX_NUM_QUEUES                  (RTOS_CFG_NUM_FIFO)
#define FIFO_QUEUE_PUT_SUCCESS               (1)
#define FIFO_STS_QUEUE_EMPTY                 (2)
#define FIFO_STS_QUEUE_FULL                  (3)
#define FIFO_STS_QUEUE_READY                 (1)
#define FIFO_FAILURE                         (0)
#define FIFO_SUCCESS                         (1)

/* API error codes */
#define FIFO_ERR_NO_ERROR                    (0)
#define FIFO_ERR_QUEUE_OUT_OF_RANGE          (255)
#define FIFO_ERR_QUEUE_FULL                  (1)
#define FIFO_ERR_QUEUE_EMPTY                 (2)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct Queue OSQueue;

/*************************************************************************/
/*  Public Functions                                                     */
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
U1 u1_OSqueue_init(Q_MEM* queueStart, U4 queueLength);

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
U1 u1_OSqueue_flushFifo(U1 queueNum, U1* error);

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
/*  Return:        Q_MEM FIFO_FAILURE            OR                      */
/*                       data at head of queue.                          */
/*************************************************************************/
Q_MEM data_OSqueue_get(U1 queueNum, U4 blockPeriod, U1* error);

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
U1 u1_OSqueue_getSts(U1 queueNum, U1* error);

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
U1 u1_OSqueue_put(U1 queueNum, U4 blockPeriod, Q_MEM message, U1* error);

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
U4 u4_OSqueue_getNumInFIFO(U1 queueNum, U1* error);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/

#else
#warning "QUEUE MODULE NOT ENABLED"

#endif /* Conditional compile */
#endif 
