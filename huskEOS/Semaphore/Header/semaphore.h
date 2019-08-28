/*************************************************************************/
/*  File Name:  semaphore.h                                              */
/*  Purpose:    Header file for semaphore module.                        */
/*  Created by: Garrett Sculthorpe on 3/3/19                             */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

#ifndef semaphore_h 
#if(RTOS_CFG_OS_SEMAPHORE_ENABLED == RTOS_CONFIG_TRUE)
#define semaphore_h

#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define SEMA_SEMAPHORE_SUCCESS         (1)
#define SEMA_SEMAPHORE_TAKEN           (0)  
#define SEMA_NO_SEMA_OBJECTS_AVAILABLE (0)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct Semaphore OSSemaphore; /* Forward declaration */

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: u1_OSsema_init                                        */
/*  Purpose:       Initialize specified semaphore.                       */
/*  Arguments:     OSSemaphore** semaphore:                              */
/*                            Address of semaphore object.               */
/*                 S1 initValue:                                         */
/*                            Initial value for semsphore.               */
/*  Return:        U1: SEMA_SEMAPHORE_SUCCESS   OR                       */
/*                     SEMA_NO_SEMA_OBJECTS_AVAILABLE                    */
/*************************************************************************/
U1 u1_OSsema_init(OSSemaphore** semaphore, S1 initValue); 

/*************************************************************************/
/*  Function Name: u1_OSsema_wait                                        */
/*  Purpose:       Claim semaphore referenced by pointer.                */
/*  Arguments:     OSSemaphore* semaphore:                               */
/*                     Pointer to semaphore.                             */
/*                 U4 blockPeriod:                                       */
/*                    Number of time units for task to sleep if blocked. */
/*                                                                       */
/*  Return:        U1 SEMA_SEMAPHORE_TAKEN      OR                       */
/*                    SEMA_SEMAPHORE_SUCCESS                             */
/*************************************************************************/
U1 u1_OSsema_wait(OSSemaphore* semaphore, U4 blockPeriod); 

/*************************************************************************/
/*  Function Name: u1_OSsema_check                                       */
/*  Purpose:       Check status of semaphore.                            */
/*  Arguments:     OSSemaphore* semaphore:                               */
/*                     Pointer to semaphore.                             */
/*  Return:        U1 SEMA_SEMAPHORE_TAKEN     OR                        */
/*                    SEMA_SEMAPHORE_SUCCESS                             */
/*************************************************************************/
U1 u1_OSsema_check(OSSemaphore* semaphore);

/*************************************************************************/
/*  Function Name: vd_OSsema_post                                        */
/*  Purpose:       Release semaphore                                     */
/*  Arguments:     OSSemaphore* semaphore:                               */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsema_post(OSSemaphore* semaphore);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/

#else
#warning "SEMAPHORE MODULE NOT ENABLED"

#endif /* Conditional compile */
#endif
