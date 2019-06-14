/*************************************************************************/
/*  File Name:  mailbox.h                                                */
/*  Purpose:    Header file for mailbox module.                          */
/*  Created by: Garrett Sculthorpe on 3/3/19                             */
/*************************************************************************/

#ifndef mailbox_h /* Protection from declaring more than once */
#if(RTOS_CFG_OS_MAILBOX_ENABLED == RTOS_CONFIG_TRUE)
#define mailbox_h

#include "mbox_internal_IF.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define MBOX_MAX_NUM_MAILBOX            (RTOS_CFG_NUM_MAILBOX)         
#define MBOX_SUCCESS                    (1)
#define MBOX_FAILURE                    (0)

/* API error codes */
#define MBOX_NO_MAILBOXES_AVAILABLE     (0)
#define MBOX_ERR_MAILBOX_OUT_OF_RANGE   (255)
#define MBOX_ERR_MAILBOX_FULL           (1)
#define MBOX_ERR_MAILBOX_EMPTY          (1)
#define MBOX_ERR_MAILBOX_IN_USE         (2)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
MAIL u4_OSmbox_getMail(U1 mailbox, U4 blockPeriod, U1* errorCode);
U1   u1_OSmbox_sendMail(U1 mailbox, U4 blockPeriod, MAIL data, U1* errorCode);
MAIL mail_OSmbox_checkMail(U1 mailbox, U1* errorCode);
void vd_OSmbox_clearMailbox(U1 mailbox);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/

#else
#warning "MAILBOX MODULE NOT ENABLED"

#endif /* Conditional compile */
#endif /* End conditional declaration for mailbox_h */
