/*************************************************************************/
/*  File Name:  semaphore.h                                              */
/*  Purpose:    Header file for semaphore module.                        */
/*  Created by: Garrett Sculthorpe on 3/3/19                             */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

#ifndef semaphore_h 
#define semaphore_h


/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define SEMA_SEMAPHORE_SUCCESS         (1)
#define SEMA_SEMAPHORE_TAKEN           (0)  


/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct Semaphore OSSemaphore; /* Forward declaration */

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: vd_OSsema_init                                        */
/*  Purpose:       Initialize specified semaphore.                       */
/*  Arguments:     Semaphore** semaphore:                                */
/*                            Address of semaphore object.               */
/*                 S1 initValue:                                         */
/*                            Initial value for semsphore.               */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsema_init(struct Semaphore** semaphore, S1 initValue); 

/*************************************************************************/
/*  Function Name: u1_OSsema_wait                                        */
/*  Purpose:       Claim semaphore referenced by pointer.                */
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*                 U4 blockPeriod:                                       */
/*                    Number of time units for task to sleep if blocked. */
/*                                                                       */
/*  Return:        U1 SEMA_SEMAPHORE_TAKEN      OR                       */
/*                    SEMA_SEMAPHORE_SUCCESS                             */
/*************************************************************************/
U1   u1_OSsema_wait(struct Semaphore* semaphore, U4 blockPeriod); 

/*************************************************************************/
/*  Function Name: u1_OSsema_check                                       */
/*  Purpose:       Check status of semaphore.                            */
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*  Return:        U1 SEMA_SEMAPHORE_TAKEN     OR                        */
/*                    SEMA_SEMAPHORE_SUCCESS                             */
/*************************************************************************/
U1   u1_OSsema_check(struct Semaphore* semaphore);

/*************************************************************************/
/*  Function Name: u1_OSsema_post                                        */
/*  Purpose:       Release semaphore                                     */
/*  Arguments:     Semaphore* semaphore:                                 */
/*                     Pointer to semaphore.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
U1   u1_OSsema_post(struct Semaphore* semaphore);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif
