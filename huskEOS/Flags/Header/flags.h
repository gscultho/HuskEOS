/*************************************************************************/
/*  File Name:  flags.h                                                  */
/*  Purpose:    Public header file for flags module.                     */
/*  Created by: Garrett Sculthorpe on 3/24/19                            */
/*  Copyright © 2019 Garrett Sculthorpe. All rights reserved.            */
/*************************************************************************/

#ifndef flags_h 
#define flags_h


/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define FLAGS_WRITE_SET                 (1)
#define FLAGS_WRITE_CLEAR               (0)
#define FLAGS_WRITE_COMMAND_INVALID     (255) 
#define FLAGS_WRITE_SUCCESS             (1)
#define FLAGS_NO_OBJ_AVAILABLE          (0)
#define FLAGS_INIT_SUCCESS              (1)
#define FLAGS_PEND_LIST_FULL            (0)
#define FLAGS_PEND_SUCCESS              (1)
#define FLAGS_EVENT_ANY                 (1)
#define FLAGS_EVENT_EXACT               (2)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef struct FlagsObj OSFlagsObj;

/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: u1_OSflags_init                                       */
/*  Purpose:       Initialize flags object.                              */
/*  Arguments:     FlagsObj** flags:                                     */
/*                        Address of flags object.                       */
/*                 U1 flagInitValues:                                    */
/*                        Initial values for flags.                      */
/*  Return:        U1: FLAGS_NO_OBJ_AVAILABLE     OR                     */
/*                     FLAGS_INIT_SUCCESS                                */
/*************************************************************************/
U1 u1_OSflags_init(struct FlagsObj** flags, U1 flagInitValues);

/*************************************************************************/
/*  Function Name: u1_OSflags_postFlags                                  */
/*  Purpose:       Write to flags object as specified.                   */
/*  Arguments:     FlagsObj* flags:                                      */
/*                    Pointer to flags object.                           */
/*                 U1 flagMask:                                          */
/*                    Mask to set/clear.                                 */
/*                 U1 set_clear:                                         */
/*                    FLAGS_WRITE_SET                 OR                 */
/*                    FLAGS_WRITE_CLEAR                                  */
/*  Return:        U4 FLAGS_WRITE_COMMAND_INVALID     OR                 */
/*                    FLAGS_WRITE_SUCCESS                                */
/*************************************************************************/
U1 u1_OSflags_postFlags(struct FlagsObj* flags, U1 flagMask, U1 set_clear); 

/*************************************************************************/
/*  Function Name: u1_OSflags_pendOnFlags                                */
/*  Purpose:       Set task to pend on certain flags. When task is woken,*/
/*                 it can retrieve the waking event by calling           */
/*                 u1_OSsch_getReasonForWakeup()                         */
/*                                                                       */
/*  Arguments:     FlagsObj* flags:                                      */
/*                      Pointer to flags object.                         */
/*                 U1 eventMask:                                         */
/*                      Event that will cause wakeup.                    */
/*                 U4 timeOut:                                           */
/*                      Maximum wait time. If 0, then wait is indefinite.*/
/*                 U1 eventType:                                         */
/*                      Type of event - FLAGS_EVENT_ANY,                 */
/*                                      FLAGS_EVENT_EXACT                */
/*                                                                       */
/*  Return:        U1: FLAGS_PEND_LIST_FULL     OR                       */
/*                     FLAGS_PEND_SUCCESS                                */
/*************************************************************************/
U1 u1_OSflags_pendOnFlags(struct FlagsObj* flags, U1 eventMask, U4 timeOut, U1 eventType);

/*************************************************************************/
/*  Function Name: vd_OSflags_reset                                      */
/*  Purpose:       Reset flags object to init state.                     */
/*  Arguments:     Flags* flags:                                         */
/*                     Pointer to flags object.                          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSflags_reset(struct FlagsObj* flags);

/*************************************************************************/
/*  Function Name: vd_OSflags_clearAll                                   */
/*  Purpose:       Clear all flags.                                      */
/*  Arguments:     Flags* flags:                                         */
/*                     Pointer to flags object.                          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_OSflags_clearAll(struct FlagsObj* flags);

/*************************************************************************/
/*  Function Name: u1_flags_checkFlags                                   */
/*  Purpose:       Check flags but do not modify.                        */
/*  Arguments:     Flags* flags:                                         */
/*                     Pointer to flags object.                          */
/*  Return:        U1 u1_t_returnVal:                                    */
/*                    Value of flags at time they were read.             */
/*************************************************************************/
U1 u1_OSflags_checkFlags(struct FlagsObj* flags);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif
