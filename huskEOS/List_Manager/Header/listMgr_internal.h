/*************************************************************************/
/*  File Name:  listMgr_internal.h                                       */
/*  Purpose:    Routines for managing task lists in scheduler.           */
/*  Created by: Garrett Sculthorpe on 7/17/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
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


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: vd_list_addNodeToEnd                                  */
/*  Purpose:       Add new node to end of specified linked list.         */
/*  Arguments:     ListNode** listHead:                                  */
/*                     Pointers of head node.                            */
/*                 ListNode* newNode:                                    */
/*                     Node to be added to end of list.                  */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_list_addNodeToEnd(struct ListNode** listHead, struct ListNode* newNode);

/*************************************************************************/
/*  Function Name: vd_list_addTaskByPrio                                 */
/*  Purpose:       Add task to a queue by order of priority.             */
/*  Arguments:     ListNode** listHead, newNode:                         */
/*                     Pointers to head node and new node.               */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_list_addTaskByPrio(struct ListNode** listHead, struct ListNode* newNode);

/*************************************************************************/
/*  Function Name: vd_list_addNodeToFront                                */
/*  Purpose:       Add node to front of linked list.                     */
/*  Arguments:     ListNode** listHead:                                  */
/*                     Pointers to head node and new node.               */
/*                 ListNode* newNode:                                    */
/*                     Pointer to node to be added to front.             */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_list_addNodeToFront(struct ListNode** listHead, struct ListNode* newNode);

/*************************************************************************/
/*  Function Name: vd_list_removeNode                                    */
/*  Purpose:       Remove a node from linked list.                       */
/*  Arguments:     ListNode** listHead:                                  */
/*                     Pointer of head node.                             */
/*                 ListNode* removeNode:                                 */
/*                     Pointer to node to remove.                        */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_list_removeNode(struct ListNode** listHead, struct ListNode* removeNode);

/*************************************************************************/
/*  Function Name: node_list_removeFirstNode                             */
/*  Purpose:       Remove first node from linked list.                   */
/*  Arguments:     ListNode** listHead:                                  */
/*                     Pointer of head node.                             */
/*  Return:        ListNode*:                                            */
/*                     Pointer to removed node.                          */
/*************************************************************************/
ListNode* node_list_removeFirstNode(struct ListNode** listHead);

/*************************************************************************/
/*  Function Name: node_list_removeNodeByTCB                             */
/*  Purpose:       Remove node from linked list that holds specified TCB.*/
/*  Arguments:     ListNode** listHead:                                  */
/*                     Pointer to head node.                             */
/*                 Sch_Task* taskTCB:                                    */
/*                     Pointer to TCB to be searched for.                */
/*  Return:        ListNode*:                                            */
/*                     Pointer to removed node.                          */
/*************************************************************************/
ListNode* node_list_removeNodeByTCB(struct ListNode** listHead, struct Sch_Task* taskTCB);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
