/*************************************************************************/
/*  File Name:  cpu_os_interface.h                                       */
/*  Purpose:    Scheduler HW interface.                                  */
/*  Created by: Garrett Sculthorpe on 2/10/2019.                         */
/*************************************************************************/

#ifndef cpu_os_interface_h /* Protection from declaring more than once */
#define cpu_os_interface_h

#include "cpu_defs.h"

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define OS_TICK_PRIORITY                       (0xC0)
#define PENDSV_PRIORITY                        (0xE0)
#define OS_INT_NO_MASK                         (0)
#define CPU_PENDSV_LOAD_MASK                   (0x10000000)

/*************************************************************************/
/*  Macros                                                               */
/*************************************************************************/
#define EnableInterrupts(c)                     __asm("CPSIE i")
#define DisableInterrupts(c)                    __asm("CPSID i")

/*************************************************************************/
/*  Interface with scheduler                                             */
/*************************************************************************/
#define OS_CPU_ENTER_CRITICAL(void)            (vd_cpu_disableInterrupts(void))
#define OS_CPU_EXIT_CRITICAL(void)             (vd_cpu_enableInterrupts(void)) 
#define vd_cpu_enableInterruptsOSStart()        EnableInterrupts(c)
#define vd_cpu_disableInterruptsOSStart()       DisableInterrupts(c)
#define OS_CPU_MASK_SCHEDULER_TICK(c)          (u1_cpu_maskInterrupts(OS_TICK_PRIORITY))
#define OS_CPU_UNMASK_SCHEDULER_TICK(c)        (vd_cpu_unmaskInterrupts(c))
#define OS_CPU_TRIGGER_DISPATCHER()            ((SYS_REG_ICSR_ADDR) |= CPU_PENDSV_LOAD_MASK)

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
void      vd_cpu_init(U4 numMs);
OS_STACK* sp_cpu_taskStackInit(void (*newTaskFcn)(void), OS_STACK* sp);
void      vd_cpu_suspendScheduler(void);
U4        u4_cpu_getCurrentMsPeriod(void);
void      vd_cpu_setNewSchedPeriod(U4 numMs);
void      vd_cpu_enableInterrupts(void);
void      vd_cpu_disableInterrupts(void);
U1        u1_cpu_maskInterrupts(U1 setMask);
void      vd_cpu_unmaskInterrupts(U1 setMask);
 
/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif /* End conditional declaration for cpu_os_interface */
