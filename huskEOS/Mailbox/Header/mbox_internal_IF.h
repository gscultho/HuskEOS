/*************************************************************************/
/*  File Name:  mbox_internal_IF.h                                       */
/*  Purpose:    Kernel access definitions and routines for mailbox.      */
/*  Created by: Garrett Sculthorpe on 3/3/19                             */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

#ifndef mbox_internal_IF_h 
#define mbox_internal_IF_h

#include "cpu_defs.h"
#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/


/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct Mailbox
{
  MAIL mail;             /* Holds data. */
  U1   blockedTaskID;    /* If a task is blocked on mailbox, its ID is stored here. */
}
Mailbox;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: vd_OSmbox_init                                        */
/*  Purpose:       Initialize mailbox module.                            */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSmbox_init(void);

/*************************************************************************/
/*  Function Name: vd_OSmbox_blockedTaskTimeout                          */
/*  Purpose:       Remove blocked task from specified mailbox.           */
/*  Arguments:     void* mbox:                                           */
/*                     Mailbox address.                                  */
/*                                                                       */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSmbox_blockedTaskTimeout(void* mbox);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
