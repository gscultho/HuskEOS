/*************************************************************************/
/*  File Name:  semaphore_internal_IF.h                                  */
/*  Purpose:    Kernel access definitions and routines for semaphore.    */
/*  Created by: Garrett Sculthorpe on 5/23/19                            */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

#ifndef semaphore_internal_IF_h 
#define semaphore_internal_IF_h

#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define SEMA_MAX_NUM_BLOCKED           (RTOS_CFG_NUM_BLOCKED_TASKS_SEMA)
  
/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
struct listNode; /* Forward declaration. Defined in "listMgr_internal.h" */

typedef struct Semaphore
{
  S1               sema;
  struct ListNode  blockedTasks[SEMA_MAX_NUM_BLOCKED];
  struct ListNode* blockedListHead;  
}
Semaphore;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_OSsema_blockedTimeout                              */
/*  Purpose:       API for scheduler to call when sleeping task times out*/
/*  Arguments:     OSSemaphore* semaphore:                               */
/*                     Pointer to semaphore.                             */
/*                 Sch_Task* taskTCB:                                    */
/*                     Pointer to TCB of blocked task.                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSsema_blockedTimeout(struct Semaphore* semaphore, struct Sch_Task* taskTCB);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
