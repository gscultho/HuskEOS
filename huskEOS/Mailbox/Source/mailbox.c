/*************************************************************************/
/*  File Name: mailbox.c                                                 */
/*  Purpose: Mailbox services for application layer tasks.               */
/*  Created by: Garrett Sculthorpe on 3/3/19.                            */
/*************************************************************************/

/* Each created mailbox is to be used by only one sender and one receiver 
   due to the fact that each mailbox structure only has allocated space 
   for storing one blocked task at a time. Queues should be used for multiple
   senders/receivers. */
   
/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "mailbox.h"
#include "sch.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/ 
#define MBOX_SCHED_INTERFACE_OFFSET    (1)
#define MBOX_BLOCK_PERIOD_NO_BLOCK     (0)
#define MBOX_SEMA_NO_BLOCK             (0)
#define MBOX_MAILBOX_EMPTY             (0)
#define MBOX_MAILBOX_NUM_VALID         (0)

/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/
static Mailbox Mbox_MailboxList[MBOX_MAX_NUM_MAILBOX];


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
static void vd_mbox_addTaskToBlocked(Mailbox* mailbox, U1 taskID);
static void vd_mbox_unblockWaitingTask(U1 mailboxID);
static void vd_mbox_blockHandler(U4 blockPeriod, U1 mailboxID);
static U1   u1_mbox_checkValidMailbox(U1 mailbox);

/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_mbox_init                                          */
/*  Purpose:       Initialize mailbox module.                            */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_mbox_init(void)
{
  U1 u1_t_index;
  
  for(u1_t_index = (U1)ZERO; u1_t_index < MBOX_MAX_NUM_MAILBOX; u1_t_index++)
  {
    Mbox_MailboxList[u1_t_index].mail          = (MAIL)MBOX_MAILBOX_EMPTY;
    Mbox_MailboxList[u1_t_index].blockedTaskID = (U1)MBOX_MAILBOX_EMPTY;
    
    vd_OSsema_init(&Mbox_MailboxList[u1_t_index].mboxSema); 
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
  Mailbox* mbox_t_p_mboxPtr;
  
  *errorCode = u1_mbox_checkValidMailbox(mailbox);

  if(*errorCode)    
  {    
    return ((U1)MBOX_FAILURE);  
  }
  
  mbox_t_p_mboxPtr = &Mbox_MailboxList[mailbox];
  
  /* If semaphore taken by other task */
  if(!(u1_OSsema_wait(&(mbox_t_p_mboxPtr->mboxSema), (U4)MBOX_SEMA_NO_BLOCK)))
  {    
    *errorCode = (U1)MBOX_ERR_MAILBOX_IN_USE;
    vd_mbox_blockHandler(blockPeriod, mailbox);
    
    return ((U1)MBOX_FAILURE);
  }     
  
  /* Semaphore is now taken by this task */ 
  
  /* Semaphore claimed, get the data */
  mail_t_data = mbox_t_p_mboxPtr->mail;
  
  /* Check if mailbox has data ready */
  if(mail_t_data != (MAIL)MBOX_MAILBOX_EMPTY)
  {
    if(mbox_t_p_mboxPtr->blockedTaskID != (U1)MBOX_MAILBOX_EMPTY)
    {  
      vd_mbox_unblockWaitingTask(mailbox);
    }
    
    /* Reset mailbox */
    mbox_t_p_mboxPtr->mail = (MAIL)MBOX_MAILBOX_EMPTY;
    
    /* Release semaphore */
    u1_OSsema_post(&(mbox_t_p_mboxPtr->mboxSema));
    
    return (mail_t_data);
  }
  /* If not, block */
  else
  {
    *errorCode = (U1)MBOX_ERR_MAILBOX_EMPTY;
    u1_OSsema_post(&(mbox_t_p_mboxPtr->mboxSema));
    vd_mbox_blockHandler(blockPeriod, mailbox);
    
    return ((U1)MBOX_FAILURE);
  }
}

/*************************************************************************/
/*  Function Name: mail_OSmbox_checkMail                                 */
/*  Purpose:       Check for data but do not clear it.                   */
/*  Arguments:     U1  mailbox:                                          */
/*                      Mailbox identifier.                              */
/*                 U1* errorCode:                                        */
/*                      Address to write error to.                       */
/*  Return:        MAIL 0 if empty              OR                       */
/*                      MBOX_FAILURE if invalid OR                       */
/*                      data held in mailbox.                            */
/*************************************************************************/
MAIL mail_OSmbox_checkMail(U1 mailbox, U1* errorCode)
{
  MAIL mail_t_data;
  Mailbox* mbox_t_p_mboxPtr;
  
  *errorCode = u1_mbox_checkValidMailbox(mailbox);

  if(*errorCode)    
  {    
    return ((U1)MBOX_FAILURE);  
  }
  
  mbox_t_p_mboxPtr = &Mbox_MailboxList[mailbox];
  mail_t_data        = (MAIL)MBOX_MAILBOX_EMPTY;
  
  OS_CPU_ENTER_CRITICAL();
  
  if(mbox_t_p_mboxPtr->mail != MBOX_MAILBOX_EMPTY)
  {
    mail_t_data = mbox_t_p_mboxPtr->mail;
  }
  
  OS_CPU_EXIT_CRITICAL();
  
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
  *errorCode = u1_mbox_checkValidMailbox(mailbox);

  if(*errorCode)    
  {    
    return ((U1)MBOX_FAILURE);  
  }
  
  /* If semaphore taken by other task */
  if(!(u1_OSsema_wait(&(Mbox_MailboxList[mailbox].mboxSema), (U4)MBOX_SEMA_NO_BLOCK)))
  {
    *errorCode = (U1)MBOX_ERR_MAILBOX_IN_USE;
    vd_mbox_blockHandler(blockPeriod, mailbox);
    
    return ((U1)MBOX_FAILURE);
  }  
  
  /* Semaphore is now claimed by this task */
  
  /* Check if mailbox has no data in it */
  if((Mbox_MailboxList[mailbox].mail) == (U4)MBOX_MAILBOX_EMPTY)
  {
    Mbox_MailboxList[mailbox].mail = data;
    
    /* Check if blocked tasks need to be unblocked */
    if((Mbox_MailboxList[mailbox].blockedTaskID) != (U1)MBOX_MAILBOX_EMPTY)
    {
      vd_mbox_unblockWaitingTask(mailbox);
    }
    
    /* Release semaphore */
    u1_OSsema_post(&(Mbox_MailboxList[mailbox].mboxSema));
    
    return ((U1)MBOX_SUCCESS);
  }
  /* Mailbox full, check if task needs to be blocked */
  else
  {
    *errorCode = (U1)MBOX_ERR_MAILBOX_FULL;
    u1_OSsema_post(&(Mbox_MailboxList[mailbox].mboxSema));
    vd_mbox_blockHandler(blockPeriod, mailbox);
  }
  
  return ((U1)MBOX_FAILURE);
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
  Mailbox* mbox_t_p_mboxPtr;
  
  /* unsigned, check only equal to or above max */
  if(u1_mbox_checkValidMailbox(mailbox))    
  {
    return;
  }
  
  mbox_t_p_mboxPtr = &Mbox_MailboxList[mailbox];
  
  OS_CPU_ENTER_CRITICAL();
  
  mbox_t_p_mboxPtr->mail = (U4)MBOX_MAILBOX_EMPTY;
  
  /* Check if there was a task waiting to send/receive data */
  if((mbox_t_p_mboxPtr->blockedTaskID) != (U1)MBOX_MAILBOX_EMPTY)
  {
    vd_mbox_unblockWaitingTask(mailbox);
  }
  
  OS_CPU_EXIT_CRITICAL();
}

/*************************************************************************/
/*  Function Name: vd_mbox_blockedTaskTimeout                            */
/*  Purpose:       Remove blocked task from specified mailbox.           */
/*  Arguments:     void* mbox:                                           */
/*                     Mailbox address.                                  */
/*                                                                       */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_mbox_blockedTaskTimeout(void* mbox)
{
  ((Mailbox*)(mbox))->blockedTaskID = (U1)MBOX_MAILBOX_EMPTY;
}

/*************************************************************************/
/*  Function Name: u1_mbox_checkValidMailbox                             */
/*  Purpose:       Return if mailbox number is valid or not.             */
/*  Arguments:     U1 mailbox:                                           */
/*                     Mailbox identifier.                               */
/*                                                                       */
/*  Return:        U1 MBOX_ERR_MAILBOX_OUT_OF_RANGE    OR                */
/*                    MBOX_MAILBOX_NUM_VALID                             */
/*************************************************************************/
static U1 u1_mbox_checkValidMailbox(U1 mailbox)
{
  if(mailbox >= (U1)MBOX_MAX_NUM_MAILBOX)    
  {
    return ((U1)MBOX_ERR_MAILBOX_OUT_OF_RANGE);  
  }
  
  return ((U1)MBOX_MAILBOX_NUM_VALID);
}

/*************************************************************************/
/*  Function Name: vd_mbox_blockHandler                                  */
/*  Purpose:       Handle task blocking if mailbox not available.        */
/*  Arguments:     U4 blockPeriod:                                       */
/*                     Period for task to sleep for.                     */
/*                 mailboxID:                                            */
/*                    Mailbox identifier,                                */
/*                                                                       */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_mbox_blockHandler(U4 blockPeriod, U1 mailboxID)
{
  Mailbox* mbox_t_p_mboxPtr;
  U1       u1_t_currentTaskID;
  
  u1_t_currentTaskID = u1_OSsch_getCurrentTask();
  
  mbox_t_p_mboxPtr = &Mbox_MailboxList[mailboxID];
  
  if(blockPeriod != (U4)MBOX_BLOCK_PERIOD_NO_BLOCK)
  {
    if((U1)mbox_t_p_mboxPtr->blockedTaskID == ZERO)
    { 
      vd_mbox_addTaskToBlocked(mbox_t_p_mboxPtr, u1_t_currentTaskID);
    }
    
    vd_sch_setReasonForSleep(&Mbox_MailboxList[mailboxID], (U1)SCH_TASK_SLEEP_RESOURCE_MBOX); 
    vd_OSsch_taskSleep(blockPeriod);  
  }
}

/*************************************************************************/
/*  Function Name: vd_mbox_addTaskToBlocked                              */
/*  Purpose:       Add task number (offset by one) to block list.        */
/*  Arguments:     Mailbox* mailbox:                                     */
/*                          Pointer to specified mailbox                 */
/*                 U1 taskID:                                            */
/*                          Identifier for task to be added to blocked.  */
/*  Return:        void                                                  */
/*************************************************************************/
static void vd_mbox_addTaskToBlocked(Mailbox* mailbox, U1 taskID)
{    
  /* Store task ID offset by one so task 0 isnt stored as 0 */
  mailbox->blockedTaskID |= taskID + (U1)MBOX_SCHED_INTERFACE_OFFSET; 
}

/*************************************************************************/
/*  Function Name: vd_mbox_unblockWaitingTask                            */
/*  Purpose:       Remove task number (offset by one) from list and wake.*/
/*  Arguments:     U1 mailboxID:                                         */
/*                    Mailbox identifier.                                */
/*  Return:        void                                                  */
/*************************************************************************/
static void vd_mbox_unblockWaitingTask(U1 mailboxID)
{
  U1 u1_t_blockedTask;
  
  u1_t_blockedTask = Mbox_MailboxList[mailboxID].blockedTaskID - (U1)MBOX_SCHED_INTERFACE_OFFSET; /* compensate for offset by 1 */
  
  vd_sch_setReasonForWakeup((U1)SCH_TASK_SLEEP_RESOURCE_MBOX, u1_t_blockedTask);
  
  Mbox_MailboxList[mailboxID].blockedTaskID = (U1)MBOX_MAILBOX_EMPTY;
}

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                3/4/19      Mailboxes with blocked task implemented.                     */
/*                                                                                             */
