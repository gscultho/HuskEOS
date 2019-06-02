/*************************************************************************/
/*  File Name:  semaphore_internal_IF.h                                  */
/*  Purpose:    Kernel access definitions and routines for semaphore.    */
/*  Created by: Garrett Sculthorpe on 5/23/19                            */
/*************************************************************************/

#ifndef semaphore_internal_IF_h /* Protection from declaring more than once */
#define semaphore_internal_IF_h

#include "cpu_defs.h"
#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define SEMA_MAX_NUM_BLOCKED           (RTOS_CFG_NUM_BLOCKED_TASKS_SEMA)


/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct
{
  S1    sema;
  U1    blockedTasks[SEMA_MAX_NUM_BLOCKED];
}
Semaphore;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_sema_blockedTimeout(Semaphore* sema, U1 taskID);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif /* End conditional declaration for semaphore_internal_IF_h */
