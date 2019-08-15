/*************************************************************************/
/*  File Name:  memory_internal_IF.h                                     */
/*  Purpose:    Kernel access definitions and routines for memory.       */
/*  Created by: Darren Cicala on 7/13/19.                                */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*  All rights reserved.                                                 */
/*************************************************************************/

#include "cpu_defs.h"
#include "rtos_cfg.h"
#include "semaphore.h"

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
typedef struct OSMemBlock
{
	U1 *start;
	U1 blockStatus;
	U1 blockSize;
}OSMemBlock;

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/



/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/