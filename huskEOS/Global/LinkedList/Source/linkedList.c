/*************************************************************************/
/*  File Name: linkedList.c                                              */
/*  Purpose: Routines for linked lists for OS and application.           */
/*  Created by: Garrett Sculthorpe on 7/13/2019.                         */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "cpu_defs.h"
#include "linkedList.h"


/*************************************************************************/
/*  External References                                                  */
/*************************************************************************/


/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/


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
/*  Function Name: vd_llist_initLlistHead                                */
/*  Purpose:       Initialize the first node in a linked list by setting */
/*                 previous pointer and next pointer to NULL.            */
/*  Arguments:     LL_Single_Node* listHead:                             */
/*                                 Pointer to first node in future list. */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_llist_initLlistHead(struct LL_Single_Node* listHead)
{
  listHead->nextNode    = LL_NULL_PTR;
  listHead->nodeContent = LL_NULL_PTR;
}

/*************************************************************************/
/*  Function Name: vd_llist_addNodeToEnd                                 */
/*  Purpose:       Add new node to end of specified linked list.         */
/*  Arguments:     LL_Single_Node* listHead, newNode:                    */
/*                     Pointers to head node and new node.               */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_llist_addNodeToEnd(struct LL_Single_Node* listHead, struct LL_Single_Node* newNode)
{
  LL_Single_Node* node_t_ptr;
  
  node_t_ptr = listHead;
  
  while(node_t_ptr->nextNode != LL_NULL_PTR)
  {
    node_t_ptr = node_t_ptr->nextNode;
  }
  
  node_t_ptr->nextNode  = newNode;
  newNode->nextNode     = LL_NULL_PTR;
}

/*************************************************************************/
/*  Function Name: vd_llist_swapNodes                                    */
/*  Purpose:       Swap data of two nodes.                               */
/*  Arguments:     LL_Single_Node* firstNode, secondNode:                */
/*                     Nodes to be swapped.                              */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_llist_swapNodes(struct LL_Single_Node* firstNode, struct LL_Single_Node* secondNode)
{
  void* void_t_tempPtr; 
  
  void_t_tempPtr          = firstNode->nodeContent;
  firstNode->nodeContent  = secondNode->nodeContent;
  secondNode->nodeContent = void_t_tempPtr;
}

/*************************************************************************/
/*  Function Name: vd_llist_addNodeAfter                                 */
/*  Purpose:       Swap two nodes in list.                               */
/*  Arguments:     LL_Single_Node* placeNode:                            */
/*                     Node that new node will be inserted after.        */
/*                 LL_Single_Node* nodeToAdd:                            */
/*                     New node to be added.                             */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_llist_addNodeAfter(struct LL_Single_Node* placeNode, struct LL_Single_Node* nodeToAdd)
{
  LL_Single_Node* node_t_tempPtr;
  
  node_t_tempPtr      = placeNode->nextNode;
  placeNode->nextNode = nodeToAdd;
  nodeToAdd->nextNode = node_t_tempPtr;
}

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                7/17/19     Added linked list routines to support dynamic queues in      */
/*                                scheduler.                                                   */