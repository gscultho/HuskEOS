/*************************************************************************/
/*  File Name:  queue_internal_IF.h                                      */
/*  Purpose:    Kernel access definitions and routines for FIFO.         */
/*  Created by: Garrett Sculthorpe on 5/25/19                            */
/*************************************************************************/

#ifndef queue_internal_IF_h /* Protection from declaring more than once */
#define queue_internal_IF_h

#include "cpu_defs.h"
#include "rtos_cfg.h"
#include "semaphore.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define FIFO_QUEUE_LENGTH_WORDS              (RTOS_CFG_BUFFER_LENGTH) 
#define Q_MEM                                 RTOS_CFG_BUFFER_DATA

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct
{
  Q_MEM*    putPtr;
  Q_MEM*    getPtr;
  Q_MEM     data[FIFO_QUEUE_LENGTH_WORDS];
  U4        blockedTaskList;
  Semaphore bufferSema;
}
Queue;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_queue_init(void);
void vd_queue_blockedTaskTimeout(void* queueAddr, U1 taskID);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif /* End conditional declaration for queue_internal_IF_h */
