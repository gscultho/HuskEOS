/*************************************************************************/
/*  File Name:  flags_internal_IF.h                                      */
/*  Purpose:    Kernel access definitions and routines for flags.        */
/*  Created by: Garrett Sculthorpe on 5/20/19                            */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

#ifndef flags_internal_IF_h 
#define flags_internal_IF_h

#include "cpu_defs.h"
#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define FLAGS_MAX_NUM_TASKS_PENDING       (RTOS_CFG_MAX_NUM_TASKS_PEND_FLAGS)


/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
struct Sch_Task; /* Forward declaration. See definition in sch_internal_IF.h */

typedef struct TasksPending
{
  U1               event;
  struct Sch_Task* tcb;
  U1               eventPendType;
}
TasksPending;

typedef struct FlagsObj
{
  U1            flags;
  TasksPending  pendingList[FLAGS_MAX_NUM_TASKS_PENDING];
}
FlagsObj;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: vd_flags_pendTimeout                                  */
/*  Purpose:       Timeout hander for pending task. Called by scheduler. */
/*  Arguments:     FlagsObj* flags:                                      */
/*                      Pointer to flags object.                         */
/*                 Sch_Task pendingTCB:                                  */
/*                      Pointer to timed out task's TCB.                 */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_flags_pendTimeout(struct FlagsObj* flags, struct Sch_Task* pendingTCB);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
