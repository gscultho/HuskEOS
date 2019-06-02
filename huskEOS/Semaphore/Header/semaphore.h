/*************************************************************************/
/*  File Name:  semaphore.h                                              */
/*  Purpose:    Header file for semaphore module.                        */
/*  Created by: Garrett Sculthorpe on 3/3/19                             */
/*************************************************************************/

#ifndef semaphore_h /* Protection from declaring more than once */
#define semaphore_h

#include "semaphore_internal_IF.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define SEMA_SEMAPHORE_SUCCESS         (1)
#define SEMA_SEMAPHORE_TAKEN           (0)  


/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void vd_OSsema_init(Semaphore* semaphore); 
U1   u1_OSsema_wait(Semaphore* semaphore, U4 blockPeriod); 
U1   u1_OSsema_post(Semaphore* semaphore);
U1   u1_OSsema_check(Semaphore* semaphore);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif /* End conditional declaration for semaphore_h */
