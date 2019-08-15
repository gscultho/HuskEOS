/*************************************************************************/
/*  File Name: memory.c                                                  */
/*  Purpose: Functions for user-controlled memory management.            */
/*  Created by: Darren Cicala on 7/13/19.                                */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
/*************************************************************************/

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include <memory.h>

#include <sch_internal_IF.h>
#include <sch.h>

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define MEM_U4_WATERMARK_SIZE (sizeof(U4))
#define MEM_WATERMARK_VAL     (0xF0)

/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/
static OSMemBlock as_MemHeap[RTOS_CFG_MAX_NUM_MEM_BLOCKS];
static U1         u1_NumBlocksAllocated = 0;

/*************************************************************************/
/*  Function Name: v_OSMemBlockInit                                      */
/*  Purpose:       Add a memory block of the given size and number       */
/*                 to the heap.                                          */
/*  Arguments:     U1  u1_size:                                          */
/*                     Desired memory amount to allocate.                */
/*                 U1  u1_number:                                        */
/*                     Number of blocks to create.                       */
/*                 U1* pu1_err:                                          */
/*                     Pointer variable for error code.                  */
/*  Return:        void                                                  */
/*************************************************************************/
void v_OSMemBlockInit(U1 u1_size, U1 u1_number, U1 *pu1_err)
{
	/* loop iterators */
	U1 u1_NumMemBlocksIterator   = 0;
	U1 u1_WatermarkSizeIterator  = 0;
	
	/* this is the total amount of memory to allocate */
	U1 u1_SizeWithWatermark      = u1_size + MEM_U4_WATERMARK_SIZE;
	
	/* loop for the user's desired amount of blocks of the given size */
	for(u1_NumMemBlocksIterator = 0; u1_NumMemBlocksIterator < u1_number; u1_NumMemBlocksIterator++)
	{
		if(u1_NumBlocksAllocated > RTOS_CFG_MAX_NUM_MEM_BLOCKS)
		{
			*pu1_err = MEM_ERR_HEAP_OUT_OF_RANGE;
			return;
		}
		else
		{
			OS_SCH_ENTER_CRITICAL();
			
			/* initialize block parameters */
			as_MemHeap[RTOS_CFG_MAX_NUM_MEM_BLOCKS].blockSize   = u1_size;
			as_MemHeap[RTOS_CFG_MAX_NUM_MEM_BLOCKS].blockStatus = BLOCK_NOT_IN_USE;
			as_MemHeap[RTOS_CFG_MAX_NUM_MEM_BLOCKS].start       = (U1*) malloc(u1_SizeWithWatermark * sizeof(U1));
			
			/* assign the last four bytes of the block to the watermarked value */
			for(u1_WatermarkSizeIterator = 0; u1_WatermarkSizeIterator < MEM_U4_WATERMARK_SIZE; u1_WatermarkSizeIterator++)
			{
				as_MemHeap[RTOS_CFG_MAX_NUM_MEM_BLOCKS].start[u1_size + u1_WatermarkSizeIterator] = MEM_WATERMARK_VAL;
			}
			++u1_NumBlocksAllocated;
			
			OS_SCH_EXIT_CRITICAL();
		}
	}
	*pu1_err = MEM_NO_ERROR;
}



/*************************************************************************/
/*  Function Name: pu1_OSMalloc                                          */
/*  Purpose:       Find and return first available memory block.         */
/*  Arguments:     U1  u1_size:                                          */
/*                     Desired memory amount to allocate.                */
/*                 U1* pu1_err:                                          */
/*                     Pointer variable for error code.                  */
/*  Return:        U1*     OR                                            */
/*                       NULL                                            */
/*************************************************************************/
U1* pu1_OSMalloc(U1 u1_size, U1* pu1_err)
{
	U1 u1_HeapIndex;
	
	/* loop for the length of the memory heap */
	for(u1_HeapIndex = 0; u1_HeapIndex < u1_NumBlocksAllocated; u1_HeapIndex++)
	{
		/* check for block availability*/
		if(as_MemHeap[u1_HeapIndex].blockStatus == BLOCK_NOT_IN_USE)
		{
			/* check to make sure the block is the proper size */
			if(as_MemHeap[u1_HeapIndex].blockSize >= u1_size)
			{
				
				OS_SCH_ENTER_CRITICAL();
				
				/* set block status to true and return pointer to memblock */
				as_MemHeap[u1_HeapIndex].blockStatus = BLOCK_IN_USE;
				
				OS_SCH_EXIT_CRITICAL();
				
				*pu1_err = MEM_NO_ERROR;
				return as_MemHeap[u1_HeapIndex].start;
			}
			
	    /* elses for standard compliance */
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}
	/* no blocks available for the given size */
	*pu1_err = MEM_ERR_MALLOC_NO_BLOCKS_AVAIL;
	return NULL;
}

/*************************************************************************/
/*  Function Name: pu1_OSCalloc                                          */
/*  Purpose:       Find and return first available memory block for      */
/*                 the specified size. Initialize all bytes to zero.     */
/*  Arguments:     U1  u1_size:                                          */
/*                     Desired memory amount to allocate.                */
/*                 U1* pu1_err:                                          */
/*                     Pointer variable for error code.                  */
/*  Return:        U1*     OR                                            */
/*                       NULL                                            */
/*************************************************************************/
U1* pu1_OSCalloc(U1 u1_size, U1* pu1_err)
{
	U1 u1_HeapIndex    = 0;
	U1 u1_ByteIndex    = 0;
	U1 u1_MemBlockSize = 0;
	
	/* loop for the length of the memory heap */
	for(u1_HeapIndex = 0; u1_HeapIndex < u1_NumBlocksAllocated; u1_HeapIndex++)
	{
		/* check for block availability*/
		if(as_MemHeap[u1_HeapIndex].blockStatus == BLOCK_NOT_IN_USE)
		{
			/* check to make sure the block is the proper size */
			if(as_MemHeap[u1_HeapIndex].blockSize >= u1_size)
			{
				
				OS_SCH_ENTER_CRITICAL();
				
				/* set block status to true and return pointer to memblock */
				as_MemHeap[u1_HeapIndex].blockStatus = BLOCK_IN_USE;
				u1_MemBlockSize = as_MemHeap[u1_HeapIndex].blockSize;
				
				/* iterate through memblock and assign each value to zero */
				for(u1_ByteIndex = 0; u1_ByteIndex < u1_MemBlockSize; u1_ByteIndex++)
				{
					as_MemHeap[u1_HeapIndex].start[u1_ByteIndex] = 0;
				}
				
				OS_SCH_EXIT_CRITICAL();
				*pu1_err = MEM_NO_ERROR;
				return as_MemHeap[u1_HeapIndex].start;
			}
			
	    /* elses for standard compliance */
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}
	/* no blocks available for the given size */
	*pu1_err = MEM_ERR_MALLOC_NO_BLOCKS_AVAIL;
	return NULL;
}
/*************************************************************************/
/*  Function Name: v_OSFree                                              */
/*  Purpose:       Destroy the passed-in pointer and free the memblock.  */
/*  Arguments:     U1* pu1_BlockStart:                                   */
/*                     Pointer to the memory contained in the memblock.  */
/*                 U1* pu1_err:                                          */
/*                     Pointer variable for error code.                  */
/*  Return:        void                                                  */
/*************************************************************************/
void v_OSFree(U1* pu1_BlockStart, U1* pu1_err)
{
	U1 u1_HeapIndex = 0;
	U1 u1_FoundFlag = 0;
	
	/* find the memory block with the matching start pointer */
	for(u1_HeapIndex = 0; u1_HeapIndex < u1_NumBlocksAllocated; u1_HeapIndex++)
	{
		if(as_MemHeap[u1_HeapIndex].start == pu1_BlockStart)
		{
			
			OS_SCH_ENTER_CRITICAL();
			
			/* free the memory block by clearing its block status */
			as_MemHeap[u1_HeapIndex].blockStatus = BLOCK_NOT_IN_USE;
		
			OS_SCH_EXIT_CRITICAL();
			
			pu1_BlockStart = NULL;
			u1_FoundFlag = 1;
		}
		else
		{
			continue;
		}
	}
	if(!u1_FoundFlag)
	{
		*pu1_err = MEM_ERR_BLOCK_NOT_FOUND;
	}
	else
	{
		*pu1_err = MEM_NO_ERROR;
	}
}


/*************************************************************************/
/*  Function Name: pu1_OSRealloc                                         */
/*  Purpose:       Free the current block and re-allocate a new block.   */
/*  Arguments:     OSMemBlock* ps_memBlock:                              */
/*                     Memory block to reallocate.                       */
/*                 U1* u1_newSize:                                       */
/*                     Desired new size of the memblock.                 */
/*  Return:        U1*                                                   */
/*  Notes:         Debated on storing the index of the memory block      */ 
/*                 somewhere, perhaps in the block structure itself. Was */
/*                 hard to return both the array pointer and the index   */
/*                 at the same time, while keeping the functions         */
/*                 std-like. Possible future implementation?             */
/*************************************************************************/
U1* pu1_OSRealloc(U1* pu1_OldPointer, U1 u1_NewSize, U1* pu1_err)
{
	U1 u1_LocalError = 0;
	
	v_OSFree(pu1_OldPointer, &u1_LocalError);
	
	if(u1_LocalError == 0)
	{
		/* DJCs Stop Point */
		//pu1_OSMemAlloc
	}
}


/******************************* end file ********************************/