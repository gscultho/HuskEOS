/*************************************************************************/
/*  File Name: listMgr_internal.c                                        */
/*  Purpose: Routines for managing ready, global task lists in scheduler */
/*           and wait lists for other RTOS modules.                      */
/*  Created by: Garrett Sculthorpe on 7/17/2019.                         */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "cpu_defs.h"
#include "listMgr_internal.h"
#include "sch_internal_IF.h"


/*************************************************************************/
/*  External References                                                  */
/*************************************************************************/


/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define LIST_NULL_PTR                   ((void*)ZERO)


/*************************************************************************/
/*  Data Structures                                                      */
/*************************************************************************/


/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


/*************************************************************************/


/*************************************************************************/
/*  Function Name: vd_list_addNodeToEnd                                  */
/*  Purpose:       Add new node to end of specified linked list.         */
/*  Arguments:     ListNode* listHead, newNode:                          */
/*                     Pointers to head node and new node.               */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_list_addNodeToEnd(struct ListNode** listHead, struct ListNode* newNode)
{
  ListNode* node_t_ptr;
  
  /* List is empty */
  if(*listHead == LIST_NULL_PTR)
  {
    newNode->nextNode     = LIST_NULL_PTR;  
    newNode->previousNode = LIST_NULL_PTR;
    *listHead             = newNode;
  }
  /* Add new node to end of list */
  else
  {
    node_t_ptr = *listHead;
  
    while(node_t_ptr->nextNode != LIST_NULL_PTR)
    {
      node_t_ptr = node_t_ptr->nextNode;
    }
  
    node_t_ptr->nextNode  = newNode;
    newNode->nextNode     = LIST_NULL_PTR;
    newNode->previousNode = node_t_ptr;
  }
}

/*************************************************************************/
/*  Function Name: vd_list_swapNodes                                     */
/*  Purpose:       Swap data of two nodes.                               */
/*  Arguments:     ListNode* firstNode, secondNode:                      */
/*                     Nodes to be swapped.                              */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_list_swapNodes(struct ListNode* firstNode, struct ListNode* secondNode)
{
  Sch_Task* tcb_t_tempPtr; 
  
  tcb_t_tempPtr   = firstNode->TCB;
  firstNode->TCB  = secondNode->TCB;
  secondNode->TCB = tcb_t_tempPtr;
}

/*************************************************************************/
/*  Function Name: vd_list_addNodeAfter                                 */
/*  Purpose:                                     */
/*  Arguments:     ListNode* placeNode:                            */
/*                     Node that new node will be inserted after.        */
/*                 ListNode* nodeToAdd:                            */
/*                     New node to be added.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_list_addNodeAfter(struct ListNode* placeNode, struct ListNode* nodeToAdd)
{
  ListNode* node_t_tempPtr;
  
  node_t_tempPtr               = placeNode->nextNode;
  placeNode->nextNode          = nodeToAdd;
  nodeToAdd->nextNode          = node_t_tempPtr;
  nodeToAdd->previousNode      = placeNode;
  node_t_tempPtr->previousNode = nodeToAdd;
}

/*************************************************************************/
/*  Function Name: vd_list_addTaskByPrio                                 */
/*  Purpose:       Add task to a queue by order of priority.             */
/*  Arguments:     ListNode* listHead, newNode:                          */
/*                     Pointers to head node and new node.               */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_list_addTaskByPrio(struct ListNode** listHead, struct ListNode* newNode)
{
  ListNode* node_t_tempPtr;
  
  /* List is empty */
  if(*listHead == LIST_NULL_PTR)
  {
    newNode->nextNode     = LIST_NULL_PTR;  
    newNode->previousNode = LIST_NULL_PTR;
    *listHead             = newNode;
  }
  /* If new node has higher priority than current highest priority task */
  else if(newNode->TCB->taskInfo.priority <= (*listHead)->TCB->taskInfo.priority) 
  {
    /* Insert new node as the new head */
    newNode->nextNode         = *listHead;
    newNode->previousNode     = LIST_NULL_PTR;
    (*listHead)->previousNode = newNode;
    
    /* Update head pointer of list */
    *listHead = newNode;
  }
  else
  {
    node_t_tempPtr = (*listHead);
    
    /* Find insertion point */
    while(newNode->TCB->taskInfo.priority >= node_t_tempPtr->TCB->taskInfo.priority)
    {
      node_t_tempPtr = node_t_tempPtr->nextNode;
    }
    
    /* Set new node's next node equal to this node */
    newNode->nextNode = node_t_tempPtr;
    
    /* Move back one node */
    node_t_tempPtr = node_t_tempPtr->previousNode;
    
    /* Set next node's previous pointer to the new node */
    (newNode->nextNode)->previousNode = newNode;
    
    /*Set new node's previous pointer equal to the node before it */
    newNode->previousNode = node_t_tempPtr;
    
    /* Set previous node's next pointer equal to the new node */
    node_t_tempPtr->nextNode = newNode;
  }
}

/*************************************************************************/
/*  Function Name: vd_list_addNodeToFront                                */
/*  Purpose:       Add node to front of linked list.                     */
/*  Arguments:     ListNode* listHead, newNode:                          */
/*                     Pointers to head node and new node.               */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_list_addNodeToFront(struct ListNode** listHead, struct ListNode* newNode)
{  
  /* List is empty */
  if(*listHead == LIST_NULL_PTR)
  {
    newNode->nextNode     = LIST_NULL_PTR;  
    newNode->previousNode = LIST_NULL_PTR;
    *listHead             = newNode;
  }
  else
  {
    newNode->previousNode = LIST_NULL_PTR;
    
    /* Connect new node with old head node */
    newNode->nextNode = (*listHead); 
    
    /* Point back to new first node */
    newNode->nextNode->previousNode = newNode;
  
    /* Set head pointer to new first node */
    *listHead = newNode;
  }
}

/*************************************************************************/
/*  Function Name: vd_list_removeNode                                    */
/*  Purpose:       Remove a node from linked list.                       */
/*  Arguments:     ListNode* removeNode:                                 */
/*                     Pointer to node to remove.                        */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_list_removeNode(struct ListNode* removeNode)
{
  /* Change links */
  if(removeNode->previousNode != LIST_NULL_PTR)
  {
    removeNode->previousNode->nextNode = removeNode->nextNode;
  }
  if(removeNode->nextNode != LIST_NULL_PTR)
  {
    removeNode->nextNode->previousNode = removeNode->previousNode;
  }
  
  /* Reset node pointers */
  removeNode->nextNode     = LIST_NULL_PTR;
  removeNode->previousNode = LIST_NULL_PTR;
}

/*************************************************************************/
/*  Function Name: node_list_removeFirstNode                             */
/*  Purpose:       Remove a node from linked list.                       */
/*  Arguments:     ListNode* listHead:                                   */
/*                     Pointer to head node.                             */
/*  Return:        ListNode*:                                            */
/*                     Pointer to removed node.                          */
/*************************************************************************/
ListNode* node_list_removeFirstNode(struct ListNode** listHead)
{
  ListNode* node_t_tempPtr;
  ListNode* node_t_deletedNodePtr;
  
  node_t_deletedNodePtr = LIST_NULL_PTR;
  
  if(*listHead != LIST_NULL_PTR)
  {
    /* Get address of first node in list */
    node_t_deletedNodePtr = *listHead;
  
    /* Move to new head of list */
    node_t_tempPtr = (*listHead)->nextNode;
  
    node_t_tempPtr->previousNode = LIST_NULL_PTR;
  
    /* Set new head pointer */
    *listHead = node_t_tempPtr;
    
    /* Reset deleted node pointers */
    node_t_deletedNodePtr->nextNode     = LIST_NULL_PTR;
    node_t_deletedNodePtr->previousNode = LIST_NULL_PTR;
  }
  
  return (node_t_deletedNodePtr);
}


/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                7/17/19     Added routines to support task queue management for scheduler*/
/*                                version 2.x. Still in work.                                  */
