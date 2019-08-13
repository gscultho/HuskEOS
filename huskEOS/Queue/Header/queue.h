/*************************************************************************/
/*  File Name:  queue.h                                                  */
/*  Purpose:    Main header file for queue module.                       */
/*  Created by: Garrett Sculthorpe on 3/16/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

#ifndef queue_h 
#if(RTOS_CFG_OS_QUEUE_ENABLED == RTOS_CONFIG_TRUE)
#define queue_h

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
#define FIFO_ERR_NO_QUEUE_AVAILABLE          (3)
#define FIFO_ERR_QUEUE_OUT_OF_RANGE          (255)
#define FIFO_ERR_QUEUE_FULL                  (1)
#define FIFO_ERR_QUEUE_EMPTY                 (1)
#define FIFO_ERR_QUEUE_IN_USE                (2)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct Queue OSQueue;

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
U1    u1_OSqueue_init(Q_MEM* queueStart, U4 queueLength);
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
#endif 
