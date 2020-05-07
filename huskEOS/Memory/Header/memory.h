/*************************************************************************/
/*  File Name:  memory.h                                                 */
/*  Purpose:    Public header file for memory module.                    */
/*  Created by: Darren Cicala on 7/13/19.                                */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
/*************************************************************************/

#ifndef memory_h
#if(RTOS_CFG_OS_MEM_ENABLED == RTOS_CONFIG_TRUE)
#define memory_h

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/

#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/

/* API Error Codes */
#define MEM_NO_ERROR                     (0)   // system working properly
#define MEM_ERR_MALLOC_NO_BLOCKS_AVAIL   (1)   // no blocks available when attempting to allocate memory for a given size
#define MEM_ERR_FREE_NOT_FOUND           (2)   // passed in ptr to OSFree could not be found in the list
#define MEM_ERR_REALLOC_NO_BLOCKS_AVAIL  (3)   // no blocks of the targeted size available when attempting a realloc
#define MEM_ERR_INVALID_SIZE_REQUEST     (4)   // user attempted to request invalid size (larger than largest block, or negative)
#define MEM_ERR_REALLOC_GEN_FAULT        (5)   // generic realloc error occurred (this SHOULD never happen)
#define MEM_ERR_BLOCK_NOT_FOUND          (6)   // when searching for a block using findBlockSize, the pointer passed in was not found
#define MEM_ERR_HIT_PARTITION_MAX        (255) // the user attempted to add more blocks than the maximum number of blocks

/* Maintenance Error Codes */
#define MEM_MAINT_NO_ERROR               (0)   // user has not overwritten the watermarks (=0xF0)
#define MEM_MAINT_ERROR                  (1)   // user has overwritten the watermarks (!=0xF0)

#define MEM_MAX_NUM_PARTITIONS           (RTOS_CFG_MAX_NUM_MEM_PARTITIONS)
#define MEM_MAX_NUM_BLOCKS               (RTOS_CFG_MAX_NUM_MEM_BLOCKS)
#define MEM_MAX_BLOCK_SIZE               (RTOS_CFG_MAX_MEM_BLOCK_SIZE)


/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/

typedef struct Block     OSMemBlock;
typedef struct Partition OSMemPartition;

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
U1 u1_OSMem_PartitionInit(U1* partitionMatrix, U1 blockSize, U1 numBlocks, U1 *err);

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
U1* pu1_OSMem_malloc(U1 sizeRequested, U1* err);

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
U1* pu1_OSMem_calloc(U1 sizeRequested, U1* err);

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
void v_OSMem_free(U1** memToFree, U1* err);

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
U1* pu1_OSMem_realloc(U1* oldPointer, U1 newSize, U1* err);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/
#else
#warning "MEMORY MODULE NOT ENABLED"

#endif
#endif

/******************************* end file ********************************/
