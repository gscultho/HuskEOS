/*************************************************************************/
/*  File Name:  mutex.h                                                  */
/*  Purpose:    Header file for mutex module.                            */
/*  Created by: Garrett Sculthorpe on 3/3/19                             */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
/*************************************************************************/

#ifndef mutex_h 
#if(RTOS_CFG_OS_MUTEX_ENABLED == RTOS_CONFIG_TRUE)
#define mutex_h


/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define MUTEX_SUCCESS              (1)
#define MUTEX_TAKEN                (0)  
#define MUTEX_NO_OBJECTS_AVAILABLE (0)
#define MUTEX_ALREADY_RELEASED     (0)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct Mutex OSMutex; /* Forward declaration */

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: u1_OSmutex_init                                       */
/*  Purpose:       Initialize specified mutex.                           */
/*  Arguments:     OSMutex** mutex:                                      */
/*                            Address of mutex object.                   */
/*                 U1 initValue:                                         */
/*                            Initial value for mutex.                   */
/*  Return:        U1: MUTEX_SUCCESS   OR                                */
/*                     MUTEX_NO_OBJECTS_AVAILABLE                        */
/*************************************************************************/
U1 u1_OSmutex_init(OSMutex** mutex, U1 initValue); 

/*************************************************************************/
/*  Function Name: u1_OSmutex_lock                                       */
/*  Purpose:       Claim mutex referenced by pointer.                    */
/*  Arguments:     OSMutex* mutex:                                       */
/*                     Pointer to mutex.                                 */
/*                 U4 blockPeriod:                                       */
/*                    Number of time units for task to sleep if blocked. */
/*                                                                       */
/*  Return:        U1 MUTEX_TAKEN      OR                                */
/*                    MUTEX_SUCCESS                                      */
/*************************************************************************/
U1 u1_OSmutex_lock(OSMutex* mutex, U4 blockPeriod); 

/*************************************************************************/
/*  Function Name: u1_OSmutex_check                                      */
/*  Purpose:       Check status of mutex.                                */
/*  Arguments:     OSMutex* mutex:                                       */
/*                     Pointer to mutex.                                 */
/*  Return:        U1 MUTEX_TAKEN     OR                                 */
/*                    MUTEX_SUCCESS                                      */
/*************************************************************************/
U1 u1_OSmutex_check(OSMutex* mutex);

/*************************************************************************/
/*  Function Name: vd_OSmutex_blockedTimeout                             */
/*  Purpose:       API for scheduler to call when sleeping task times out*/
/*  Arguments:     Mutex* mutex:                                         */
/*                     Pointer to mutex.                                 */
/*                 Sch_Task* taskTCB:                                    */
/*                     Pointer to TCB of blocked task.                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
U1 u1_OSmutex_unlock(OSMutex* mutex);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/

#else
#warning "MUTEX MODULE NOT ENABLED"

#endif /* Conditional compile */
#endif
