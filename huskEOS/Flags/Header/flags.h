/*************************************************************************/
/*  File Name:  flags.h                                                  */
/*  Purpose:    Public header file for flags module.                     */
/*  Created by: Garrett Sculthorpe on 3/24/19                            */
/*************************************************************************/

#ifndef flags_h /* Protection from declaring more than once */
#define flags_h

#include "flags_internal_IF.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define FLAGS_WRITE_SET                 (1)
#define FLAGS_WRITE_CLEAR               (0)
#define FLAGS_WRITE_COMMAND_INVALID     (255) 
#define FLAGS_WRITE_SUCCESS             (1)


/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef FlagsObj OSFlagsObj;

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_OSflags_init(FlagsObj* flags, U1 flagInitValues);
U1   u1_OSflags_pendOnFlags(FlagsObj* flags, U1 eventMask, U4 timeOut);
U1   u1_OSflags_postFlags(FlagsObj* flags, U1 flagMask, U1 set_clear); 
void vd_OSflags_clearAll(FlagsObj* flags);
U1   u1_OSflags_checkFlags(FlagsObj* flags);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif /* End conditional declaration for flags_h */
