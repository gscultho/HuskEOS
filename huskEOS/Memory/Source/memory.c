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
static U1         u1_LargestBlockSize   = 0;


/*************************************************************************/
/*  Function Name: u1_FindHeapIndex                                      */
/*  Purpose:       Find the index of the memory block with the same      */
/*                 passed in start pointer.                              */
/*  Arguments:     U1* pu1_BlockStart:                                   */
/*                     Pointer to find in the heap.                      */
/*                 U1* pu1_err:                                          */
/*                     Pointer variable for error code.                  */
/*  Return:        U1                                                    */
/*************************************************************************/
U1 u1_FindHeapIndex(U1* pu1_BlockStart, U1* pu1_err)
{
	U1 u1_HeapIndex = 0;
	
	/* loop through the structure and find the index */
	for(u1_HeapIndex = 0; u1_HeapIndex < u1_NumBlocksAllocated; u1_HeapIndex++)
	{
		/* return the current index if the pointers match */
		if(as_MemHeap[u1_HeapIndex].start == pu1_BlockStart)
		{
			*pu1_err = MEM_NO_ERROR;
			return u1_HeapIndex;
		}
		else
		{
			continue;
		}
	}
	/* otherwise, set an error code and return 255 */
	*pu1_err = MEM_ERR_BLOCK_NOT_FOUND;
	return 255;
}

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
		if(u1_NumBlocksAllocated >= RTOS_CFG_MAX_NUM_MEM_BLOCKS)
		{
			*pu1_err = MEM_ERR_HEAP_OUT_OF_RANGE;
			return;
		}
		else
		{
			OS_SCH_ENTER_CRITICAL();
			
			/* initialize block parameters */
			as_MemHeap[u1_NumBlocksAllocated].blockSize   = u1_size;
			as_MemHeap[u1_NumBlocksAllocated].blockStatus = BLOCK_NOT_IN_USE;
			as_MemHeap[u1_NumBlocksAllocated].start       = (U1*) malloc(u1_SizeWithWatermark * sizeof(U1));
			
			/* assign the last four bytes of the block to the watermarked value */
			for(u1_WatermarkSizeIterator = 0; u1_WatermarkSizeIterator < MEM_U4_WATERMARK_SIZE; u1_WatermarkSizeIterator++)
			{
				as_MemHeap[u1_NumBlocksAllocated].start[u1_size + u1_WatermarkSizeIterator] = MEM_WATERMARK_VAL;
			}
			++u1_NumBlocksAllocated;
			
			/* set the global block size parameter */
			if(u1_LargestBlockSize < u1_size)
			{
				u1_LargestBlockSize = u1_size;
			}
			else
			{
			}
			
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
			/* enter a critical section here to make sure no other process or */
			/* interrupt claims this block at the same time                   */
			OS_SCH_ENTER_CRITICAL();
			
			/* check to make sure the block is the proper size */
			if(as_MemHeap[u1_HeapIndex].blockSize >= u1_size)
			{
				
				/* set block status to true and return pointer to memblock */
				as_MemHeap[u1_HeapIndex].blockStatus = BLOCK_IN_USE;
				
				OS_SCH_EXIT_CRITICAL();
				
				*pu1_err = MEM_NO_ERROR;
				return as_MemHeap[u1_HeapIndex].start;
			}
			else
			{
				OS_SCH_EXIT_CRITICAL();
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
			/* enter a critical section here to make sure no other process or */
			/* interrupt claims this block at the same time                   */
			OS_SCH_ENTER_CRITICAL();
			
			/* check to make sure the block is the proper size */
			if(as_MemHeap[u1_HeapIndex].blockSize >= u1_size)
			{
				
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
				OS_SCH_EXIT_CRITICAL();
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
void v_OSFree(U1** pu1_BlockStart, U1* pu1_err)
{
	U1 u1_HeapIndex  = 0;
	U1 u1_LocalError = 0;
	U1 u1_BlockIndex = 0;
	
	u1_BlockIndex = u1_FindHeapIndex(*pu1_BlockStart, &u1_LocalError);
	
	if(u1_LocalError == MEM_NO_ERROR)
	{
		OS_SCH_ENTER_CRITICAL();
		
		/* free the memory block by clearing its block status */
		as_MemHeap[u1_BlockIndex].blockStatus = BLOCK_NOT_IN_USE;
	
		OS_SCH_EXIT_CRITICAL();
		
		/* destroy the pointer and set the relevant error codes */
		*pu1_BlockStart = NULL;
		*pu1_err = MEM_NO_ERROR;
		return;
	}
	else
	{
		*pu1_err = MEM_ERR_BLOCK_NOT_FOUND;
		return;
	}
}


/*************************************************************************/
/*  Function Name: pu1_OSRealloc                                         */
/*  Purpose:       Free the current block and re-allocate a new block.   */
/*  Arguments:     U1** pu1_OldPointer:                                  */
/*                      Pointer to memory to reallocate.                 */
/*                 U1   u1_newSize:                                      */
/*                      Desired new size of the memblock.                */
/*                 U1*  pu1_err:                                         */
/*                      Pointer variable for error code.                 */
/*  Return:        U1*                                                   */
/*************************************************************************/
U1* pu1_OSRealloc(U1* pu1_OldPointer, U1 u1_NewSize, U1* pu1_err)
{
	U1 u1_LocalError       = 0;
	U1 u1_OldBlockIndex    = 0;
	U1 u1_DataTransferIdx  = 0;
	
	/* case if the user requests a size zero */
	if(u1_NewSize == 0)
	{
		/* free the old pointer and check for error */
		v_OSFree(&pu1_OldPointer, &u1_LocalError);
		if(u1_LocalError == MEM_NO_ERROR)
		{
			*pu1_err = MEM_NO_ERROR;
		}
		else
		{
			*pu1_err = u1_LocalError;
		}
		return NULL;
	}
	
	/* case if the user requests a size outside of the possible range */
	else if ( u1_NewSize < 0
		     || u1_NewSize > u1_LargestBlockSize)
	{
		/* just return an error */
		*pu1_err = MEM_ERR_INVALID_SIZE_REQUEST;
		return NULL;
	}
	
	/* case new size is valid */
	else
	{
		/* first we try to allocate a new memory block */
		U1* pu1_NewPointer = pu1_OSMalloc(u1_NewSize, &u1_LocalError);
		
		/* next make sure there are no error codes */
		if(  u1_LocalError == MEM_NO_ERROR 
			&& pu1_NewPointer != NULL)
		{
			/* begin transfer of memory from old block to new block */
			/* find index of old memory block */
			u1_OldBlockIndex = u1_FindHeapIndex(*pu1_OldPointer, &u1_LocalError);
			
			/* compare new size and old size, determine which is bigger */
			if(u1_LocalError == MEM_NO_ERROR)
			{
				/* specific case where the user requests a smaller size than 
				   the current allocation */
				if(as_MemHeap[u1_OldBlockIndex].blockSize > u1_NewSize)
				{
					OS_SCH_ENTER_CRITICAL();
					
					for(u1_DataTransferIdx = 0; u1_DataTransferIdx < u1_NewSize; u1_DataTransferIdx++)
					{
						pu1_NewPointer[u1_DataTransferIdx] = pu1_OldPointer[u1_DataTransferIdx];
					}
					
					OS_SCH_EXIT_CRITICAL();
					/* free the old pointer, set a warning, and return the new pointer */
					v_OSFree(&pu1_OldPointer, &u1_LocalError);
					*pu1_err = MEM_WARN_REALLOC_SMALLER_BLOCK;
					return pu1_NewPointer;
				}
				
				/* most cases where the user wants more data than previously allocated */
				else
				{
          OS_SCH_ENTER_CRITICAL();
					
					for(u1_DataTransferIdx = 0; u1_DataTransferIdx < as_MemHeap[u1_OldBlockIndex].blockSize; u1_DataTransferIdx++)
					{
						pu1_NewPointer[u1_DataTransferIdx] = pu1_OldPointer[u1_DataTransferIdx];
					}
					
					OS_SCH_EXIT_CRITICAL();
					
					/* free the old pointer, set an error if any, and return the new pointer */
					v_OSFree(&pu1_OldPointer, &u1_LocalError);
					*pu1_err = u1_LocalError;
					return pu1_NewPointer;
				}									
			}
		}
		
		/* else any error occurred in the allocation */
		else
		{
			*pu1_err = MEM_ERR_REALLOC_NO_BLOCKS_AVAIL;
			return pu1_OldPointer;
		}
	}
}


/*************************************************************************/
/*  Function Name: u1_MemMaintenance                                     */
/*  Purpose:       Check the status of all the memblocks to ensure that  */
/*                 the user hasn't overwritten any of the watermarks.    */
/*  Arguments:     None.                                                 */
/*  Return:        MEM_MAINT_NO_ERROR (0) or                             */
/*                 MEM_MAINT_ERROR    (1)                                */
/*************************************************************************/
U1 u1_MemMaintenance()
{
	/* local variables */
	U1 u1_HeapIndex        = 0;
	U1 u1_WatermarkIndex   = 0;
	U1 u1_CurrentBlockSize = 0;
	
	/* iterate through the heap */
	for(u1_HeapIndex = 0; u1_HeapIndex < u1_NumBlocksAllocated; u1_HeapIndex++)
	{
		u1_CurrentBlockSize = as_MemHeap[u1_HeapIndex].blockSize;
		
		/* iterate through the trailing watermarks at the end of the memory block*/
		for(u1_WatermarkIndex = u1_CurrentBlockSize; u1_WatermarkIndex < MEM_U4_WATERMARK_SIZE; u1_WatermarkIndex++)
		{
			/* each watermark should be 0xF0 */
			if(as_MemHeap[u1_HeapIndex].start[u1_WatermarkIndex] != MEM_WATERMARK_VAL)
			{
				return MEM_MAINT_ERROR; 
			}
			else
			{
				continue;
			}
		}
	}
	return MEM_MAINT_NO_ERROR;
}

/******************************* end file ********************************/