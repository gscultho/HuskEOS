/*************************************************************************/
/*  File Name:  queue.h                                                  */
/*  Purpose:    Main header file for queue module.                       */
/*  Created by: Garrett Sculthorpe on 3/16/19.                           */
/*************************************************************************/

#ifndef queue_h /* Protection from declaring more than once */
#if(RTOS_CFG_OS_QUEUE_ENABLED == RTOS_CONFIG_TRUE)
#define queue_h

#include "queue_internal_IF.h"

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
#define FIFO_ERR_NO_QUEUE_AVAILABLE          (0)
#define FIFO_ERR_QUEUE_OUT_OF_RANGE          (255)
#define FIFO_ERR_QUEUE_FULL                  (1)
#define FIFO_ERR_QUEUE_EMPTY                 (1)
#define FIFO_ERR_QUEUE_IN_USE                (2)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
Q_MEM data_OSqueue_get(U1 queueNum, U4 blockPeriod, U1* error);
U1    u1_OSqueue_put(U1 queueNum, U4 blockPeriod, Q_MEM message, U1* error);
U1    u1_OSqueue_flushFifo(U1 queueNum, U1* error);
U1    u1_OSqueue_getSts(U1 queueNum, U1* error);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/

#else
#warning "QUEUE MODULE NOT ENABLED"

#endif /* Conditional compile */
#endif /* End conditional declaration for queue_h */
