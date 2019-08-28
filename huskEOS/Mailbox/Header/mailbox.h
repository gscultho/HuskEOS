/*************************************************************************/
/*  File Name:  mailbox.h                                                */
/*  Purpose:    Header file for mailbox module.                          */
/*  Created by: Garrett Sculthorpe on 3/3/19                             */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

#ifndef mailbox_h 
#if(RTOS_CFG_OS_MAILBOX_ENABLED == RTOS_CONFIG_TRUE)
#define mailbox_h

#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define MBOX_MAX_NUM_MAILBOX            (RTOS_CFG_NUM_MAILBOX)         
#define MBOX_SUCCESS                    (1)
#define MBOX_FAILURE                    (0)

/* API error codes */
#define MBOX_NO_ERROR                   (0)
#define MBOX_ERR_MAILBOX_OUT_OF_RANGE   (255)
#define MBOX_ERR_MAILBOX_FULL           (1)
#define MBOX_ERR_MAILBOX_EMPTY          (1)
#define MBOX_ERR_MAILBOX_IN_USE         (2)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct Mailbox OSMailbos; /* Forward declaration */

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: mail_OSmbox_getMail                                   */
/*  Purpose:       Get data from specified mailbox.                      */
/*  Arguments:     U1  mailbox:                                          */
/*                      Mailbox identifier.                              */
/*                 U4  blockPeriod:                                      */
/*                      Number of time units for task to sleep if blocked*/
/*                 U1* errorCode:                                        */
/*                      Address to write error to.                       */
/*  Return:        MAIL   MBOX_FAILURE     OR                            */
/*                        data held in mailbox.                          */
/*************************************************************************/
MAIL mail_OSmbox_getMail(U1 mailbox, U4 blockPeriod, U1* errorCode);

/*************************************************************************/
/*  Function Name: mail_OSmbox_checkMail                                 */
/*  Purpose:       Check for data but do not clear it.                   */
/*  Arguments:     U1  mailbox:                                          */
/*                       Mailbox identifier.                             */
/*                 U1* errorCode:                                        */
/*                       Address to write error to.                      */
/*  Return:        MAIL: MBOX_FAILURE if invalid or empty     OR         */
/*                       data held in mailbox.                           */
/*************************************************************************/
MAIL mail_OSmbox_checkMail(U1 mailbox, U1* errorCode);

/*************************************************************************/
/*  Function Name: u1_OSmbox_sendMail                                    */
/*  Purpose:       Send data to mailbox.                                 */
/*  Arguments:     U1  mailbox:                                          */
/*                     Mailbox identifier.                               */
/*                 U4  blockPeriod:                                      */
/*                     Number of time units for task to sleep if blocked.*/
/*                 MAIL  data:                                           */
/*                       Data to be dropped.                             */
/*                 U1* errorCode:                                        */
/*                      Address to write error to.                       */
/*                                                                       */
/*  Return:        U1 MBOX_SUCCESS  OR                                   */
/*                    MBOX_FAILURE                                       */
/*************************************************************************/
U1 u1_OSmbox_sendMail(U1 mailbox, U4 blockPeriod, MAIL data, U1* errorCode);

/*************************************************************************/
/*  Function Name: vd_OSmbox_clearMailbox                                */
/*  Purpose:       Clear data and wake task if one is blocked.           */
/*  Arguments:     U1 mailbox:                                           */
/*                     Mailbox identifier.                               */
/*                                                                       */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSmbox_clearMailbox(U1 mailbox);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/

#else
#warning "MAILBOX MODULE NOT ENABLED"

#endif /* Conditional compile */
#endif 
