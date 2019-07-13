/*************************************************************************/
/*  File Name:  flags_internal_IF.h                                      */
/*  Purpose:    Kernel access definitions and routines for flags.        */
/*  Created by: Garrett Sculthorpe on 5/20/19                            */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

#ifndef flags_internal_IF_h /* Protection from declaring more than once */
#define flags_internal_IF_h

#include "cpu_defs.h"
#include "rtos_cfg.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/



/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct 
{
  U1 flags;
  U1 pendedTaskID;
  U1 pendEvent;
}
FlagsObj;


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_flags_pendTimeout(FlagsObj* flags);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif /* End conditional declaration for flags_internal_IF_h */
