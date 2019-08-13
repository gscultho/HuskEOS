/*************************************************************************/
/*  File Name:  rtos_cfg.h                                               */
/*  Purpose:    Configuration for RTOS.                                  */
/*  Created by: Garrett Sculthorpe on 2/28/2019                          */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
/*************************************************************************/

#ifndef rtos_cfg_h 
#define rtos_cfg_h

#include "cpu_defs.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define RTOS_CONFIG_TRUE                            (1)
#define RTOS_CONFIG_FALSE                           (0)

/* Application */
#define RTOS_CONFIG_TASK_STACK_SIZE                 (200)                /* Does not need to be used, each task can have its own stack size */
#define RTOS_CONFIG_BG_TASK_STACK_SIZE              (50)                 /* Stack size for background task if enabled */
#define RTOS_CONFIG_CALC_TASK_CPU_LOAD              (RTOS_CONFIG_TRUE)   /* Can only be enabled if RTOS_CONFIG_BG_TASK and RTOS_CONFIG_ENABLE_BACKGROUND_IDLE_SLEEP enabled */


/* Scheduling */
#define RTOS_CONFIG_MAX_NUM_TASKS                   (6)                  /* This number of TCBs will be allocated at compile-time, plus any others used by OS */
                                                                         /* Available priorities are 0 - 0xEF with 0 being highest priority. */
#define RTOS_CONFIG_ENABLE_BACKGROUND_IDLE_SLEEP    (RTOS_CONFIG_TRUE)   /* Can only be used if RTOS_CONFIG_BG_TASK is enabled */
#define RTOS_CONFIG_ENABLE_STACK_OVERFLOW_DETECT    (RTOS_CONFIG_TRUE)   /* Can only be used if RTOS_CONFIG_BG_TASK is enabled */
#define RTOS_CONFIG_BG_TASK                         (RTOS_CONFIG_TRUE)   /* OS will handle background activities and then put CPU to sleep when idle */
#define RTOS_CONFIG_PRESLEEP_FUNC                   (RTOS_CONFIG_FALSE)  /* If enabled, hook function app_OSPreSleepFcn() can be defined in application. */
#define RTOS_CONFIG_POSTSLEEP_FUNC                  (RTOS_CONFIG_FALSE)  /* If enabled, hook function app_OSPostSleepFcn() can be defined in application. */

/* Mailbox */
#define RTOS_CFG_OS_MAILBOX_ENABLED                 (RTOS_CONFIG_TRUE)
#define RTOS_CFG_NUM_MAILBOX                        (3)
#define RTOS_CFG_MBOX_DATA                          U4                    /* Data type for mailbox */

/* Message Queues */
#define RTOS_CFG_OS_QUEUE_ENABLED                   (RTOS_CONFIG_TRUE)
#define RTOS_CFG_NUM_FIFO                           (3)
#define RTOS_CFG_BUFFER_LENGTH                      (10)
#define RTOS_CFG_MAX_NUM_BLOCKED_TASKS_FIFO         (3)
#define RTOS_CFG_BUFFER_DATA                        U4

/* Semaphores */
#define RTOS_CFG_OS_SEMAPHORE_ENABLED               (RTOS_CONFIG_TRUE)
#define RTOS_CFG_NUM_SEMAPHORES                     (2)
#define RTOS_CFG_NUM_BLOCKED_TASKS_SEMA             (3)

/* Flags */
#define RTOS_CFG_OS_FLAGS_ENABLED                   (RTOS_CONFIG_TRUE)
#define RTOS_CFG_NUM_FLAG_OBJECTS                   (2)
#define RTOS_CFG_MAX_NUM_TASKS_PEND_FLAGS           (2)                   /* Maximum number of tasks that can pend on flags object. */

/* I/O */
#define PART_TM4C123GH6PM 1

  
/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
/* Internal - Do not modify */
typedef RTOS_CFG_MBOX_DATA   MAIL;
typedef RTOS_CFG_BUFFER_DATA Q_MEM;

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
