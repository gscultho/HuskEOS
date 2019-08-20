/*************************************************************************/
/*  File Name:  memory.h                                                 */
/*  Purpose:    Public header file for memory module.                    */
/*  Created by: Darren Cicala on 7/13/19.                                */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
/*************************************************************************/

#include "memory_internal_IF.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/


/* API Error Codes */
#define MEM_NO_ERROR                     (0)   // system working properly
#define MEM_ERR_MALLOC_NO_BLOCKS_AVAIL   (1)   // no blocks available when attempting a malloc for a given size
#define MEM_ERR_BLOCK_NOT_FOUND          (2)   // passed in ptr to OSFree could not be found in the list
#define MEM_ERR_REALLOC_NO_BLOCKS_AVAIL  (3)   // no blocks available when attempting a realloc
#define MEM_ERR_INVALID_SIZE_REQUEST     (4)   // user attempted to request invalid  size (larger than largest block, or negative)
#define MEM_ERR_HEAP_OUT_OF_RANGE        (255) // the user attempted to add more blocks than the maximum number of blocks

/* API Warning Codes */
#define MEM_WARN_REALLOC_SMALLER_BLOCK   (100) // 


/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/



/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/



/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/