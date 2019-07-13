/*************************************************************************/
/*  File Name:  mbox_internal_IF.h                                       */
/*  Purpose:    Kernel access definitions and routines for mailbox.      */
/*  Created by: Garrett Sculthorpe on 3/3/19                             */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

#ifndef mbox_internal_IF_h /* Protection from declaring more than once */
#define mbox_internal_IF_h

#include "cpu_defs.h"
#include "rtos_cfg.h"
#include "semaphore.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define MAIL               RTOS_CFG_MBOX_DATA


/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct 
{
  MAIL      mail;
  U1        blockedTaskID;   
  Semaphore mboxSema;
}
Mailbox;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_mbox_init(void);
void vd_mbox_blockedTaskTimeout(void* mbox);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif /* End conditional declaration for mbox_internal_IF_h */
