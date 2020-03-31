/*************************************************************************/
/*  File Name:  memory_internal_IF.h                                     */
/*  Purpose:    Kernel access definitions and routines for memory.       */
/*  Created by: Darren Cicala on 7/13/19.                                */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
/*************************************************************************/

#ifndef memory_internal_IF_h
#if(RTOS_CFG_OS_MEM_ENABLED == RTOS_CONFIG_TRUE)
#define memory_internal_IF_h

#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/

#define BLOCK_IN_USE       (1)
#define BLOCK_NOT_IN_USE   (0)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/

/* Low level memory "block" */
typedef struct Block
{
	U1 *start;            // Starting address of the memory block. For the Cortex M4, this has a size of 4 bytes.
	U1 blockStatus;       // Status of this block, IN_USE or NOT_IN_USE
	U1 blockSize;         // Size of this block. Size will be the actual size minus the watermark size.
} // Total size: 6 bytes 
Block;

typedef struct Partition
{
	U1*    start;                               // Starting address of the memory partition (matrix).
	U1     numBlocks;                           // Number of blocks in this partition. Vanilla max is 16.
	U1     numActiveBlocks;                     // Number of active/in-use blocks in this partition.
	U1     blockSize;                           // Size of the blocks in this partition. Vanilla max is 62 bytes.
	Block  blocks[RTOS_CFG_MAX_NUM_MEM_BLOCKS]; // Array of block references in this partition.
} // Maximum size: 103 bytes.
Partition;

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
U1 u1_OSMem_maintenance(void);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/
static U1         numPartitionsAllocated = 0;   // the current number of partitions allocated. Cannot exceed RTOS_CFG_MAX_NUM_MEM_PARTITIONS.
static U1         largestBlockSize       = 0;   // Largest block size currently managed. Slight runtime improvement to store this variable.

#endif
#endif

/******************************* end file ********************************/
