/*************************************************************************/
/*  File Name:  flags_internal_IF.h                                      */
/*  Purpose:    Kernel access definitions and routines for flags.        */
/*  Created by: Garrett Sculthorpe on 5/20/19                            */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

#ifndef flags_internal_IF_h 
#define flags_internal_IF_h

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
  U1               event;            /* Event that task is pending on. */
  struct Sch_Task* tcb;              /* Pointer to pending task TCB. */
  U1               eventPendType;    /* Type of pend (exact match or just one flag set). */
}
TasksPending;

typedef struct FlagsObj
{
  U1            flags;                                       /* Object containing 8 flags. */
  TasksPending  pendingList[FLAGS_MAX_NUM_TASKS_PENDING];    /* Memory allocated for storing pending task info. */
}
FlagsObj;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: vd_OSflags_pendTimeout                                */
/*  Purpose:       Timeout hander for pending task. Called by scheduler. */
/*  Arguments:     FlagsObj* flags:                                      */
/*                      Pointer to flags object.                         */
/*                 Sch_Task pendingTCB:                                  */
/*                      Pointer to timed out task's TCB.                 */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSflags_pendTimeout(struct FlagsObj* flags, struct Sch_Task* pendingTCB);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
