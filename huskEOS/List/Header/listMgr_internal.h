/*************************************************************************/
/*  File Name:  listMgr_internal.h                                       */
/*  Purpose:    Routines for managing task lists in scheduler.           */
/*  Created by: Garrett Sculthorpe on 7/17/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

#ifndef TaskList_Node_h 
#define TaskList_Node_h

#include "cpu_defs.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
  

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
struct Sch_Task; /* Forward declaration. See definition in sch_internal_IF.h */

typedef struct ListNode
{
  struct ListNode* nextNode;
  struct ListNode* previousNode;
  struct Sch_Task* TCB;
} 
ListNode;

/*typedef struct BlockedTasks
{
  
}
BlockedTasks;*/

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void      vd_list_initLlistHead(struct ListNode* listHead);
void      vd_list_addTaskByPrio(struct ListNode** listHead, struct ListNode* newNode);
void      vd_list_addNodeToEnd(struct ListNode** listHead, struct ListNode* newNode);
void      vd_list_swapNodes(struct ListNode* firstNode, struct ListNode* secondNode);
void      vd_list_addNodeAfter(struct ListNode* placeNode, struct ListNode* nodeToAdd); //needed?
void      vd_list_addNodeToFront(struct ListNode** listHead, struct ListNode* newNode);
void      vd_list_removeNode(struct ListNode* removeNode);
ListNode* node_list_removeFirstNode(struct ListNode** listHead);
void      vd_list_removeNodeByID(struct ListNode** listHead, U1 taskID);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
