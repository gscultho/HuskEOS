/*************************************************************************/
/*  File Name: mailbox.c                                                 */
/*  Purpose: Mailbox services for application layer tasks.               */
/*  Created by: Garrett Sculthorpe on 3/3/19.                            */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

/* Each created mailbox is to be used between only two tasks 
   due to the fact that each mailbox structure only has allocated space 
   for storing one blocked task at a time. Queues should be used for multiple
   senders/receivers. */
   
#include "rtos_cfg.h"

#if(RTOS_CFG_OS_MAILBOX_ENABLED == RTOS_CONFIG_TRUE)

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "mbox_internal_IF.h"
#include "mailbox.h"
#include "listMgr_internal.h"
#include "sch_internal_IF.h"
#include "sch.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/ 
#define MBOX_BLOCK_PERIOD_NO_BLOCK     (0)
#define MBOX_MAILBOX_EMPTY             (0)
#define MBOX_MAILBOX_NUM_VALID         (0)
#define MBOX_NO_BLOCKED_TASK           (0)

/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/
static Mailbox Mbox_MailboxList[MBOX_MAX_NUM_MAILBOX];


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
static void vd_OSmbox_unblockWaitingTask(U1 mailboxID);
static void vd_OSmbox_blockHandler(U4 blockPeriod, U1 mailboxID);
static U1   u1_OSmbox_checkValidMailbox(U1 mailbox);

/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_OSmbox_init                                        */
/*  Purpose:       Initialize mailbox module.                            */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSmbox_init(void)
{
  U1 u1_t_index;
  
  for(u1_t_index = (U1)ZERO; u1_t_index < (U1)MBOX_MAX_NUM_MAILBOX; u1_t_index++)
  {
    Mbox_MailboxList[u1_t_index].mail          = (MAIL)MBOX_MAILBOX_EMPTY;
    Mbox_MailboxList[u1_t_index].blockedTaskID = (U1)MBOX_NO_BLOCKED_TASK;
  }
}

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
MAIL mail_OSmbox_getMail(U1 mailbox, U4 blockPeriod, U1* errorCode)
{
  MAIL     mail_t_data;
  
  *errorCode = u1_OSmbox_checkValidMailbox(mailbox);

  if(*errorCode)    
  {    
    mail_t_data = (U1)MBOX_FAILURE;  
  }
  else
  {
    OS_SCH_ENTER_CRITICAL();
    
    mail_t_data = Mbox_MailboxList[mailbox].mail;
  
    /* Check if mailbox has data ready */
    if(mail_t_data != (MAIL)MBOX_MAILBOX_EMPTY)
    {
      if(Mbox_MailboxList[mailbox].blockedTaskID != (U1)MBOX_NO_BLOCKED_TASK)
      {  
        vd_OSmbox_unblockWaitingTask(mailbox);
      }
      else
      {
        
      }
      
      /* Reset mailbox */
      Mbox_MailboxList[mailbox].mail = (MAIL)MBOX_MAILBOX_EMPTY;
    }
    /* If not, block */
    else
    {
      *errorCode = (U1)MBOX_ERR_MAILBOX_EMPTY;

      /* If block period is non-zero, then block task */
      if(blockPeriod != (U1)MBOX_BLOCK_PERIOD_NO_BLOCK)
      {
        vd_OSmbox_blockHandler(blockPeriod, mailbox);
        
        /* Let task go to sleep. */
        OS_SCH_EXIT_CRITICAL();
        
        /* When it wakes, check again for mail. There is no blocked task since this was the blocked task. */ 
        OS_SCH_ENTER_CRITICAL();
        
        mail_t_data = Mbox_MailboxList[mailbox].mail;
      
        /* Check if mailbox has data ready */
        if(mail_t_data != (MAIL)MBOX_MAILBOX_EMPTY)
        {
          /* Reset mailbox */
          Mbox_MailboxList[mailbox].mail = (MAIL)MBOX_MAILBOX_EMPTY;
          *errorCode                     = (U1)MBOX_NO_ERROR;
        }
        else
        {
          /* Non-blocking call, return immediately. */
        }
      }
    }/* mail_t_data != (MAIL)MBOX_MAILBOX_EMPTY */
    
    OS_SCH_EXIT_CRITICAL();
    
  }/* if *errorCode */
  
  return (mail_t_data);
}

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
MAIL mail_OSmbox_checkMail(U1 mailbox, U1* errorCode)
{
  MAIL mail_t_data;
  
  *errorCode = u1_OSmbox_checkValidMailbox(mailbox);

  if(*errorCode)    
  {    
    mail_t_data = (U1)MBOX_FAILURE;  
  }
  else
  {
    OS_CPU_ENTER_CRITICAL();
  
    if(Mbox_MailboxList[mailbox].mail != (U1)MBOX_MAILBOX_EMPTY)
    {
      mail_t_data = Mbox_MailboxList[mailbox].mail;
    }
    else
    {
     
    }
    
    OS_CPU_EXIT_CRITICAL();
  }
  
  return (mail_t_data);
}

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
U1 u1_OSmbox_sendMail(U1 mailbox, U4 blockPeriod, MAIL data, U1* errorCode)
{ 
  U1 u1_t_return;
  
  *errorCode = u1_OSmbox_checkValidMailbox(mailbox);

  if(*errorCode)    
  {    
    u1_t_return = (U1)MBOX_FAILURE;  
  }
  else
  {
    OS_CPU_ENTER_CRITICAL();
    
    /* Check if mailbox has no data in it */
    if((Mbox_MailboxList[mailbox].mail) == (U4)MBOX_MAILBOX_EMPTY)
    {
      Mbox_MailboxList[mailbox].mail = data;
      
      /* Check if blocked tasks need to be unblocked */
      if((Mbox_MailboxList[mailbox].blockedTaskID) != (U1)MBOX_NO_BLOCKED_TASK)
      {
        vd_OSmbox_unblockWaitingTask(mailbox);
      }
      else
      {
        /* No task waiting. */
      }
      
      u1_t_return = (U1)MBOX_SUCCESS;
    }
    /* Mailbox full, check if task needs to be blocked */
    else
    {
      *errorCode = (U1)MBOX_ERR_MAILBOX_FULL;
      
      /* If block period is non-zero, then block task */
      if(blockPeriod != (U1)MBOX_BLOCK_PERIOD_NO_BLOCK)
      {        
        vd_OSmbox_blockHandler(blockPeriod, mailbox);
        
        /* Let task sleep. */
        OS_CPU_EXIT_CRITICAL(); 
        
        /* Task wakes up here. Check again if mailbox is available. */
        OS_CPU_ENTER_CRITICAL();
        
        if((Mbox_MailboxList[mailbox].mail) == (U4)MBOX_MAILBOX_EMPTY)
        {
          Mbox_MailboxList[mailbox].mail = data;
          *errorCode                     = (U1)MBOX_NO_ERROR;
          u1_t_return                    = (U1)MBOX_SUCCESS;
        }
        else
        {
          /* Don't block task since task woke from block timeout. */
          u1_t_return = (U1)MBOX_FAILURE; 
        }
      }
      else
      {
        /* Not a blocking call. */
        u1_t_return = (U1)MBOX_FAILURE;
      }/* blockPeriod != (U1)MBOX_BLOCK_PERIOD_NO_BLOCK */ 
    }/* (Mbox_MailboxList[mailbox].mail) == (U4)MBOX_MAILBOX_EMPTY */
    
    OS_CPU_EXIT_CRITICAL(); 
    
  }/* if(*errorCode) */
  
  return ((U1)u1_t_return);
}

/*************************************************************************/
/*  Function Name: vd_OSmbox_clearMailbox                                */
/*  Purpose:       Clear data and wake task if one is blocked.           */
/*  Arguments:     U1 mailbox:                                           */
/*                     Mailbox identifier.                               */
/*                                                                       */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSmbox_clearMailbox(U1 mailbox)
{
  if(u1_OSmbox_checkValidMailbox(mailbox))    
  {

  }
  else
  {  
    OS_CPU_ENTER_CRITICAL();
    
    Mbox_MailboxList[mailbox].mail = (U4)MBOX_MAILBOX_EMPTY;
    
    /* Check if there was a task waiting to send/receive data */
    if((Mbox_MailboxList[mailbox].blockedTaskID) != (U1)MBOX_NO_BLOCKED_TASK)
    {
      vd_OSmbox_unblockWaitingTask(mailbox);
    }
    
    OS_CPU_EXIT_CRITICAL();
  }
}

/*************************************************************************/
/*  Function Name: vd_OSmbox_blockedTaskTimeout                          */
/*  Purpose:       Remove blocked task from specified mailbox.           */
/*  Arguments:     void* mbox:                                           */
/*                     Mailbox address.                                  */
/*                                                                       */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSmbox_blockedTaskTimeout(void* mbox)
{
  ((Mailbox*)(mbox))->blockedTaskID = (U1)MBOX_NO_BLOCKED_TASK;
}

/*************************************************************************/
/*  Function Name: u1_OSmbox_checkValidMailbox                           */
/*  Purpose:       Return if mailbox number is valid or not.             */
/*  Arguments:     U1 mailbox:                                           */
/*                     Mailbox identifier.                               */
/*                                                                       */
/*  Return:        U1 MBOX_ERR_MAILBOX_OUT_OF_RANGE    OR                */
/*                    MBOX_MAILBOX_NUM_VALID                             */
/*************************************************************************/
static U1 u1_OSmbox_checkValidMailbox(U1 mailbox)
{
  if(mailbox >= (U1)MBOX_MAX_NUM_MAILBOX)    
  {
    return ((U1)MBOX_ERR_MAILBOX_OUT_OF_RANGE);  
  }
  
  return ((U1)MBOX_MAILBOX_NUM_VALID);
}

/*************************************************************************/
/*  Function Name: vd_OSmbox_blockHandler                                */
/*  Purpose:       Handle task blocking if mailbox not available.        */
/*  Arguments:     U4 blockPeriod:                                       */
/*                     Period for task to sleep for.                     */
/*                 mailboxID:                                            */
/*                    Mailbox identifier,                                */
/*                 CALL IN CRITICAL SECTION                              */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_OSmbox_blockHandler(U4 blockPeriod, U1 mailboxID)
{  
  /* Do not block if a task is already on block list, per mailbox usage requirement of one blocked task per mailbox. */
  if(Mbox_MailboxList[mailboxID].blockedTaskID == (U1)MBOX_NO_BLOCKED_TASK)
  { 
    Mbox_MailboxList[mailboxID].blockedTaskID = SCH_CURRENT_TASK_ID;
  }
  
  /* Set task to sleep state and notify scheduler of reason. */
  vd_OSsch_setReasonForSleep(&Mbox_MailboxList[mailboxID], (U1)SCH_TASK_SLEEP_RESOURCE_MBOX); 
  vd_OSsch_taskSleep(blockPeriod);  
}

/*************************************************************************/
/*  Function Name: vd_OSmbox_unblockWaitingTask                          */
/*  Purpose:       Remove task number (offset by one) from list and wake.*/
/*  Arguments:     U1 mailboxID:                                         */
/*                    Mailbox identifier.                                */
/*  Return:        void                                                  */
/*************************************************************************/
static void vd_OSmbox_unblockWaitingTask(U1 mailboxID)
{
  U1 u1_t_blockedTask;
  
  u1_t_blockedTask = Mbox_MailboxList[mailboxID].blockedTaskID;
  
  /* Notify scheduler of reason for task wakeup. */
  vd_OSsch_setReasonForWakeup((U1)SCH_TASK_SLEEP_RESOURCE_MBOX, SCH_ID_TO_TCB(u1_t_blockedTask));
  
  /* Notify scheduler to change task state. If woken task is higher priority than running task, context switch will occur after critical section. */
  vd_OSsch_taskWake(u1_t_blockedTask);
  
  Mbox_MailboxList[mailboxID].blockedTaskID = (U1)MBOX_NO_BLOCKED_TASK;
}

#endif /* Conditional compile */

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                3/4/19      Mailboxes with blocked task implemented.                     */
/*                                                                                             */
/* 1.0                8/3/19      Updated for better software flow and block handling.         */
