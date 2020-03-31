/*************************************************************************/
/*  File Name: memory.c                                                  */
/*  Purpose: Functions for user-controlled memory management.            */
/*  Created by: Darren Cicala on 7/13/19.                                */
/*  Copyright � 2019 Garrett Sculthorpe and Darren Cicala.              */
/*  All rights reserved.                                                 */
/*************************************************************************/

#include <rtos_cfg.h>

#if(RTOS_CFG_OS_MEM_ENABLED == RTOS_CONFIG_TRUE)

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include <memory.h>
#include <memory_internal_IF.h>
#include <sch_internal_IF.h> 
#include <sch.h>


/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define MEM_WATERMARK_SIZE    (2)
#define MEM_WATERMARK_VAL     (0xF0)


/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/
static OSMemPartition partitionList[MEM_MAX_NUM_PARTITIONS];


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
U1 u1_OSMem_findBlockSize(U1* blockStart, U1* err);


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/

/*************************************************************************/
/*  Function Name: u1_OSMem_PartitionInit                                */
/*  Purpose:       Add a memory partition to the memory manager.         */
/*  Arguments:     U1* partitionMatrix:                                  */
/*                     Desired memory partition to allocate.             */
/*                 U1  blockSize:                                        */
/*                     Size of each block (number of matrix columns).    */
/*                 U1  numBlocks:                                        */
/*                     Number of matrix rows.                            */
/*                 U1* err:                                              */
/*                     Error variable. Can be one of the following:      */
/*                     MEM_ERR_INVALID_SIZE_REQUEST, or                  */
/*                     MEM_ERR_HIT_PARTITION_MAX, or                     */
/*                     MEM_NO_ERROR                                      */
/*  Return:        U1  numPartitions - 1                                 */
/*                     Index of the newly allocated partition.           */
/*************************************************************************/
U1 u1_OSMem_PartitionInit(U1* partitionMatrix, U1 blockSize, U1 numBlocks, U1 *err)
{
	U1  blockIndex     = 0;
	U1* tempBlockStart = NULL;
	
	/* cant exceed the maximum block size or maximum number of blocks */
	if(blockSize > MEM_MAX_BLOCK_SIZE ||
		 numBlocks > MEM_MAX_NUM_BLOCKS)
	{
		*err = MEM_ERR_INVALID_SIZE_REQUEST;
	}
	/* we have a valid partition, so add its information to the list */
	else
	{
		OS_SCH_ENTER_CRITICAL();
		/* can't exceed the maximum number of partitions, needs to be critical because of the global variable */
		if(numPartitionsAllocated == MEM_MAX_NUM_PARTITIONS)
		{
			*err = MEM_ERR_HIT_PARTITION_MAX;
		}
		else
		{
			tempBlockStart = partitionMatrix; // capture the matrix start as a temp variable
			partitionList[numPartitionsAllocated].start           = partitionMatrix;
			partitionList[numPartitionsAllocated].blockSize       = blockSize;
			partitionList[numPartitionsAllocated].numBlocks       = numBlocks;
			partitionList[numPartitionsAllocated].numActiveBlocks = 0;
			
			/* loop over each row of the matrix, and create a new memory block, and assign default properties */
			for(blockIndex = 0; blockIndex < numBlocks; blockIndex++)
			{
				tempBlockStart += blockSize;
				partitionList[numPartitionsAllocated].blocks[blockIndex].blockSize   = blockSize;
				partitionList[numPartitionsAllocated].blocks[blockIndex].blockStatus = BLOCK_NOT_IN_USE;
				partitionList[numPartitionsAllocated].blocks[blockIndex].start       = tempBlockStart;
			}
			numPartitionsAllocated++; // increment the partition tracker to indicate we added a new partition
			*err = MEM_NO_ERROR;
			
			/* set the global largestBlockSize, slight runtime improvement later */
			if(blockSize > largestBlockSize)
			{
				largestBlockSize = blockSize;
			}
			
		}
		OS_SCH_EXIT_CRITICAL();
	}
	return (numPartitionsAllocated - 1); // return the index of the partition we just added 
}

/*************************************************************************/
/*  Function Name: pu1_OSMem_malloc                                      */
/*  Purpose:       Find and return first available memory block.         */
/*  Arguments:     U1  sizeRequested:                                    */
/*                     Desired memory amount to allocate.                */
/*                 U1* err:                                              */
/*                     Error variable. Can be one of the following:      */
/*                     MEM_ERR_INVALID_SIZE_REQUEST, or                  */
/*                     MEM_ERR_MALLOC_NO_BLOCKS_AVAIL, or                */
/*                     MEM_NO_ERROR                                      */
/*  Return:        U1* Pointer to the allocated memory block, or         */
/*                     NULL                                              */
/*************************************************************************/
U1* pu1_OSMem_malloc(U1 sizeRequested, U1* err)
{
	U1 partitionIndex = 0;
	U1 blockIndex     = 0;
	U1 byteIndex      = 0;
	
	/* check to see if the user requested more memory than possible */
	if(sizeRequested > largestBlockSize)
	{
		*err = MEM_ERR_INVALID_SIZE_REQUEST;
		return NULL;
	}
	
	/* loop over all partitions */
	for(partitionIndex = 0; partitionIndex < numPartitionsAllocated; partitionIndex++)
	{
		/* size requested should be smaller than the block size minus the watermark size */
		if(sizeRequested > partitionList[partitionIndex].blockSize - MEM_WATERMARK_SIZE)
		{
			continue;
		}
		/* otherwise, valid partition found, so check to see if there are available blocks */
		else
		{
			OS_SCH_ENTER_CRITICAL();
			
			/* if there are no open blocks in this partition, go to the next partition */
			if(partitionList[partitionIndex].numActiveBlocks == partitionList[partitionIndex].numBlocks)
			{
				OS_SCH_EXIT_CRITICAL();
				continue;
			}
			/* otherwise, loop for all blocks in this partition */
			else
			{
				for(blockIndex = 0; blockIndex < partitionList[partitionIndex].numBlocks; blockIndex++)
				{
					/* if this block is not in use, it meets all the criteria, so return it */
					if(partitionList[partitionIndex].blocks[blockIndex].blockStatus != BLOCK_IN_USE)
					{
						partitionList[partitionIndex].blocks[blockIndex].blockStatus = BLOCK_IN_USE;
						
						/* assign all bytes to the watermark value (garbage) because this isn't calloc */
						for(byteIndex = 0; byteIndex < partitionList[partitionIndex].blockSize; byteIndex++)
						{
							partitionList[partitionIndex].blocks[blockIndex].start[byteIndex] = MEM_WATERMARK_VAL;
						}
						partitionList[partitionIndex].numActiveBlocks++;
						OS_SCH_EXIT_CRITICAL();
						
						*err = MEM_NO_ERROR;
						return partitionList[partitionIndex].blocks[blockIndex].start;
					}
				}
			}
		}
	}
	/* if we made it here, there are no blocks available for the given size */
	*err = MEM_ERR_MALLOC_NO_BLOCKS_AVAIL;
	return NULL;
}

/*************************************************************************/
/*  Function Name: pu1_OSMem_calloc                                      */
/*  Purpose:       Return first available memory block, filled with 0's. */
/*  Arguments:     U1  sizeRequested:                                    */
/*                     Desired memory amount to allocate.                */
/*                 U1* err:                                              */
/*                     Error variable. Can be one of the following:      */
/*                     MEM_ERR_INVALID_SIZE_REQUEST, or                  */
/*                     MEM_ERR_MALLOC_NO_BLOCKS_AVAIL, or                */
/*                     MEM_NO_ERROR                                      */
/*  Return:        U1* Pointer to the allocated memory block, or         */
/*                     NULL                                              */
/*************************************************************************/
U1* pu1_OSMem_calloc(U1 sizeRequested, U1* err)
{
	U1 partitionIndex = 0;
	U1 blockIndex     = 0;
	U1 byteIndex      = 0;
	
	/* check to see if the user requested more memory than possible */
	if(sizeRequested > largestBlockSize)
	{
		*err = MEM_ERR_INVALID_SIZE_REQUEST;
		return NULL;
	}
	
	/* loop over all partitions */
	for(partitionIndex = 0; partitionIndex < numPartitionsAllocated; partitionIndex++)
	{
		/* size requested should be smaller than the block size minus the watermark size */
		if(sizeRequested > partitionList[partitionIndex].blockSize - MEM_WATERMARK_SIZE)
		{
			continue;
		}
		/* otherwise, valid partition found, so check to see if there are available blocks */
		else
		{
			OS_SCH_ENTER_CRITICAL();
			
			/* if there are no open blocks in this partition, go to the next partition */
			if(partitionList[partitionIndex].numActiveBlocks == partitionList[partitionIndex].numBlocks)
			{
				OS_SCH_EXIT_CRITICAL();
				continue;
			}
			/* loop for all blocks in this partition */
			for(blockIndex = 0; blockIndex < partitionList[partitionIndex].numBlocks; blockIndex++)
			{
				/* if this block is not in use, it meets all the criteria, so return it */
				if(partitionList[partitionIndex].blocks[blockIndex].blockStatus != BLOCK_IN_USE)
				{
					partitionList[partitionIndex].blocks[blockIndex].blockStatus = BLOCK_IN_USE;
					
					/* assign the last two bytes to the watermark value, and the rest to zeroes */
					for(byteIndex = 0; byteIndex < partitionList[partitionIndex].blockSize; byteIndex++)
					{
						if(byteIndex > partitionList[partitionIndex].blockSize - MEM_WATERMARK_SIZE)
						{
							partitionList[partitionIndex].blocks[blockIndex].start[byteIndex] = MEM_WATERMARK_VAL;
						}
						else
						{
							partitionList[partitionIndex].blocks[blockIndex].start[byteIndex] = 0;
						}
					}
					partitionList[partitionIndex].numActiveBlocks++;
					OS_SCH_EXIT_CRITICAL();
					
					*err = MEM_NO_ERROR;
					return partitionList[partitionIndex].blocks[blockIndex].start;
				}
			}
		}
	}
	/* no blocks available for the given size */
	*err = MEM_ERR_MALLOC_NO_BLOCKS_AVAIL;
	return NULL;
}

/*************************************************************************/
/*  Function Name: v_OSMem_free                                          */
/*  Purpose:       Destroy the passed-in pointer and free the memblock.  */
/*  Arguments:     U1** memToFree:                                       */
/*                      Pointer to the memory contained in the memblock. */
/*                 U1*  err:                                             */
/*                     Error variable. Can be one of the following:      */
/*                     MEM_ERR_FREE_NOT_FOUND, or                        */
/*                     MEM_NO_ERROR                                      */
/*  Return:        void                                                  */
/*************************************************************************/
void v_OSMem_free(U1** memToFree, U1* err)
{
	U1 foundMemoryBlock = 0;
	U1 partitionIndex = 0;
	U1 blockIndex     = 0;
	U1* blockStart    = NULL;
	
	/* find the block currently in use by the heap */
	for(partitionIndex = 0; partitionIndex < numPartitionsAllocated; partitionIndex++)
	{
		U1 numBlocks = partitionList[partitionIndex].numBlocks;
		for(blockIndex = 0; blockIndex < numBlocks; blockIndex++)
		{
			blockStart = partitionList[partitionIndex].blocks[blockIndex].start;
			
			/* compare the pointer address to see if they are te same */
			if(*memToFree == blockStart)
			{
				OS_SCH_ENTER_CRITICAL();
				
				partitionList[partitionIndex].blocks[blockIndex].blockStatus = BLOCK_NOT_IN_USE;
				*memToFree = NULL; // terminate the existing pointer 
				*err = MEM_NO_ERROR;
				foundMemoryBlock = 1;
				
				OS_SCH_EXIT_CRITICAL();
				break;
			}
		}
		/* runtime improvement: dont consider other partitions after we already freed the pointer */
		if(foundMemoryBlock == 1)
		{
			break;
		}
	}
	/* set an error if we never found the passed-in pointer */
	if(foundMemoryBlock == 0)
	{
		*err = MEM_ERR_FREE_NOT_FOUND;
	}
}


/*************************************************************************/
/*  Function Name: pu1_OSMem_realloc                                     */
/*  Purpose:       Free the current block and re-allocate a new block.   */
/*  Arguments:     U1*  oldPointer:                                      */
/*                      Pointer to memory to reallocate.                 */
/*                 U1   newSize:                                         */
/*                      Desired new size of the memblock.                */
/*                 U1*  err:                                             */
/*                      Error variable. Can be one of the following:     */
/*                      MEM_ERR_INVALID_SIZE_REQUEST, or                 */
/*                      MEM_ERR_REALLOC_GEN_FAULT, or                    */
/*                      MEM_ERR_FREE_NOT_FOUND, or                       */
/*                      MEM_ERR_BLOCK_NOT_FOUND, or                      */
/*                      MEM_NO_ERROR                                     */
/*  Return:        U1* Pointer to the allocated memory block, or         */
/*                     NULL                                              */
/*************************************************************************/
U1* pu1_OSMem_realloc(U1* oldPointer, U1 newSize, U1* err)
{
	U1  localError          = 0;
	U1  loopLength          = 0;
	U1  byteIndex           = 0;	
	U1* newPointer          = NULL;
	
	/* find size of old memory block */
	U1 oldBlockSize = u1_OSMem_findBlockSize(oldPointer, &localError);
	
	/* case if the user requests a size zero */
	if(newSize == 0)
	{
		/* free the old pointer and check for error */
		v_OSMem_free(&oldPointer, &localError);
		if(localError == MEM_NO_ERROR)
		{
			*err = MEM_NO_ERROR;
		}
		else
		{
			*err = localError;
		}
		return NULL;
	}
	
	/* case if the user requests a size outside of the possible range, or the same size as the old block */
	else if (  newSize > largestBlockSize
		      || newSize == oldBlockSize)
	{
		/* just return an error, don't do anything to the old pointer */
		*err = MEM_ERR_INVALID_SIZE_REQUEST;
		return NULL;
	}
		
	/* case new size is valid */
	else
	{
		/* first we try to allocate a new memory block */
		newPointer = pu1_OSMem_malloc(newSize, &localError);
		
		/* next make sure there are no error codes */
		if(  localError == MEM_NO_ERROR 
			&& newPointer != NULL)
		{
			/* choose the smallest of the two loop lengths */
			if(newSize > oldBlockSize)
			{
				loopLength = oldBlockSize;
			}
			else
			{
				loopLength = newSize;
			}
			
			OS_SCH_ENTER_CRITICAL();
			/* begin transfer of memory from old block to new block */
			for(byteIndex = 0; byteIndex < loopLength; byteIndex++)
			{
				if(byteIndex > (loopLength - MEM_WATERMARK_SIZE))
				{
					newPointer[byteIndex] = MEM_WATERMARK_VAL;
				}
				else if (byteIndex > oldBlockSize)
				{
					newPointer[byteIndex] = MEM_WATERMARK_VAL;
				}
				else
				{
					newPointer[byteIndex] = oldPointer[byteIndex];
				}
			}
			
			OS_SCH_EXIT_CRITICAL();
			v_OSMem_free(&oldPointer, &localError);
			*err = localError;
			return newPointer;
		}
		else
		{
			*err = localError;
			return NULL;
		}
	}
}

/*************************************************************************/
/*  Private Functions                                                    */
/*************************************************************************/

/*************************************************************************/
/*  Function Name: u1_OSMem_findBlockSize                                */
/*  Purpose:       Find the index of the memory block with the same      */
/*                 passed in start pointer.                              */
/*  Arguments:     U1* blockStart:                                       */
/*                     Pointer to find in the partition list.            */
/*                 U1* err:                                              */
/*                      Error variable. Can be one of the following:     */
/*                      MEM_ERR_INVALID_SIZE_REQUEST, or                 */
/*                      MEM_NO_ERROR                                     */
/*  Return:        U1  Size of the memory block. 255 if error.           */
/*************************************************************************/
U1 u1_OSMem_findBlockSize(U1* blockStart, U1* err)
{
	U1 partitionIndex = 0;
	U1 blockIndex     = 0;
	/* loop through the structure and find the index */
	for(partitionIndex = 0; partitionIndex < numPartitionsAllocated; partitionIndex++)
	{
		OSMemPartition tempPartition = partitionList[partitionIndex];
		for(blockIndex = 0; blockIndex < tempPartition.numBlocks; blockIndex++)
		{
			if(blockStart == tempPartition.blocks[blockIndex].start)
			{
				*err = MEM_NO_ERROR;
				return tempPartition.blockSize - 2;
			}
			else
			{
			}
		}
	}
	/* otherwise, set an error code and return 255 */
	*err = MEM_ERR_BLOCK_NOT_FOUND;
	return 255;
}

/*************************************************************************/
/*  Function Name: u1_OSMem_maintenance                                  */
/*  Purpose:       Check the status of all the memblocks to ensure that  */
/*                 the user hasn't overwritten any of the watermarks.    */
/*  Arguments:     None.                                                 */
/*  Return:        MEM_MAINT_NO_ERROR (0) or                             */
/*                 MEM_MAINT_ERROR    (1)                                */
/*************************************************************************/
U1 u1_OSMem_maintenance()
{
	/* local variables */
	U1 partitionIndex   = 0;
	U1 blockIndex       = 0;
	U1 watermarkIndex   = 0;
	U1 currentBlockSize = 0;
	
	/* iterate through the list of partitions */
	for(partitionIndex = 0; partitionIndex < numPartitionsAllocated; partitionIndex++)
	{
		currentBlockSize = partitionList[partitionIndex].blockSize;
		
		/* iterate through all of the blocks */
		for(blockIndex = 0; blockIndex < partitionList[partitionIndex].numBlocks; blockIndex++)
		{
			if(partitionList[partitionIndex].blocks[blockIndex].blockStatus == BLOCK_IN_USE)
			{
				OSMemBlock tempBlock = partitionList[partitionIndex].blocks[blockIndex];
				for(watermarkIndex = (currentBlockSize - MEM_WATERMARK_SIZE); watermarkIndex < currentBlockSize; watermarkIndex++)
				{
					if(tempBlock.start[watermarkIndex] != MEM_WATERMARK_VAL)
					{
						return MEM_MAINT_ERROR;
					}
					else
					{
					}
				}
			}
			else
			{
			}
		}
	}
	return MEM_MAINT_NO_ERROR;
}

#endif /* conditional compile */

/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                08/14/19    Initial commit.                                              */
/*                                                                                             */
/* 0.2                08/19/19    Implemented realloc function.                                */
/*                                                                                             */
/* 0.3                08/24/19    Fixed bugs w/free and init. Added maintenance function.      */
/*                                                                                             */
/* 0.4                08/25/19    Fixed bug with realloc where pointer would get destroyed.    */
/*                                                                                             */
/* 1.0                08/26/19    Reorganized memory module according to standard.             */
/*                                                                                             */
/* 1.1                09/04/19    Fixed warnings at compile time.                              */
/*                                                                                             */
/* 2.0                02/22/20    Rewrite to remove malloc syscall references.                 */
