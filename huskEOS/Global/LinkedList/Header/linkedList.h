/*************************************************************************/
/*  File Name:  linkedList.h                                             */
/*  Purpose:    Public interface for linked list library.                */
/*  Created by: Garrett Sculthorpe on 7/13/19.                           */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

#ifndef linkedList_h 
#define linkedList_h

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define LL_NULL_PTR                   ((void*)ZERO)
  

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct LL_Single_Node
{
  struct LL_Single_Node* nextNode;
  void*                  nodeContent;
}
LL_Single_Node;

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_llist_initLlistHead(struct LL_Single_Node* listHead);
void vd_llist_addNodeToEnd(struct LL_Single_Node* listHead, struct LL_Single_Node* newNode);
void vd_llist_swapNodes(struct LL_Single_Node* firstNode, struct LL_Single_Node* secondNode);
void vd_llist_addNodeAfter(struct LL_Single_Node* placeNode, struct LL_Single_Node* nodeToAdd);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
