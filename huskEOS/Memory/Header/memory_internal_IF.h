/*************************************************************************/
/*  File Name:  memory_internal_IF.h                                     */
/*  Purpose:    Kernel access definitions and routines for memory.       */
/*  Created by: Darren Cicala on 7/13/19.                                */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
/*************************************************************************/

#ifndef memory_internal_IF_h
#define memory_internal_IF_h

#include "cpu_defs.h"
#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/

#define BLOCK_IN_USE       (1)
#define BLOCK_NOT_IN_USE   (0)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/

//struct OSMemBlock;

/* Low level memory "block" */
typedef struct Block
{
	U1 *start;
	U1 blockStatus;
	U1 blockSize;
}
Block;

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/
static U1         u1_NumBlocksAllocated = 0;
static U1         u1_LargestBlockSize   = 0;

#endif


/******************************* end file ********************************/