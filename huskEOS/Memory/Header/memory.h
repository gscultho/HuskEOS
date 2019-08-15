/*************************************************************************/
/*  File Name:  memory.h                                                 */
/*  Purpose:    Public header file for memory module.                    */
/*  Created by: Darren Cicala on 7/13/19.                                */
/*  Copyright � 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
/*************************************************************************/

#include "memory_internal_IF.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/


/* API Error Codes */
#define MEM_NO_ERROR                     (0)
#define MEM_ERR_MALLOC_NO_BLOCKS_AVAIL   (1)
#define MEM_ERR_BLOCK_NOT_FOUND          (2)
#define MEM_ERR_REALLOC_NO_BLOCKS_AVAIL  (3)
#define MEM_ERR_REALLOC_NO_SM_BLKS_AVL   (4)
#define MEM_ERR_HEAP_OUT_OF_RANGE        (255)


/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/



/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/



/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/