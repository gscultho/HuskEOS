/*************************************************************************/
/*  File Name:  rtos_cfg.h                                               */
/*  Purpose:    Configuration for RTOS.                                  */
/*  Created by: Garrett Sculthorpe on 2/28/2019                          */
/*************************************************************************/

#ifndef rtos_cfg_h /* Protection from declaring more than once */
#define rtos_cfg_h

#include "cpu_defs.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define RTOS_CONFIG_TRUE                            (1)
#define RTOS_CONFIG_FALSE                           (0)

/* Application */
#define RTOS_CONFIG_TASK_STACK_SIZE                 (100)                /* Does not need to be used, each task can have its own stack size */
#define RTOS_CONFIG_BG_TASK_STACK_SIZE              (50)


/* Scheduling */
#define RTOS_CONFIG_ENABLE_BACKGROUND_IDLE_SLEEP    (RTOS_CONFIG_TRUE)
#define RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT    (RTOS_CONFIG_TRUE)
#define RTOS_CONFIG_BG_TASK                         (RTOS_CONFIG_TRUE)
#define RTOS_CONFIG_MAX_NUM_TASKS                   (6)
#define RTOS_CONFIG_IDLE_SLEEP                      (RTOS_CONFIG_TRUE)
#define RTOS_CONFIG_PRESLEEP_FUNC                   (RTOS_CONFIG_TRUE)
#define RTOS_CONFIG_POSTSLEEP_FUNC                  (RTOS_CONFIG_TRUE)
#define RTOS_CONFIG_APP_IDLE_TASK                   (RTOS_CONFIG_TRUE)

/* Mailbox */
#define RTOS_CFG_OS_MAILBOX_ENABLED                 (RTOS_CONFIG_TRUE)
#define RTOS_CFG_NUM_MAILBOX                        (3)
#define RTOS_CFG_MBOX_DATA                          U4                     /* Data type for mailbox */

/* Message Queues */
#define RTOS_CFG_OS_QUEUE_ENABLED                   (RTOS_CONFIG_TRUE)
#define RTOS_CFG_NUM_FIFO                           (3)
#define RTOS_CFG_BUFFER_LENGTH                      (10)
#define RTOS_CFG_BUFFER_DATA                        U4                     /* Data type for queue */

/* Semaphores */
#define RTOS_CFG_OS_SEMAPHORE_ENABLED               (RTOS_CONFIG_TRUE)
#define RTOS_CFG_NUM_BLOCKED_TASKS_SEMA             (3)

/* Flags */
#define RTOS_CFG_OS_FLAGS_ENABLED                   (RTOS_CONFIG_TRUE)

/* I/O */
#define PART_TM4C123GH6PM 1

  
/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif /* End conditional declaration for rtos_cfg_h */
