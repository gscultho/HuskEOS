/*************************************************************************/
/*  File Name:  cpu_os_interface.h                                       */
/*  Purpose:    Scheduler HW interface. Routines for stack init, critical*/
/*              sections, system tick, etc.                              */
/*  Created by: Garrett Sculthorpe on 2/10/2019.                         */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

#ifndef cpu_os_interface_h 
#define cpu_os_interface_h

#include "rtos_cfg.h"

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
#define vd_OSsch_systemTick_ISR(void)           (SysTick_Handler(void))

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/
/*************************************************************************/
/*  Function Name: vd_cpu_init                                           */
/*  Purpose:       Initialize registers for scheduler interrupts.        */
/*  Arguments:     U4 numMs:                                             */
/*                    Period for scheduler IRQ to be triggered.          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_cpu_init(U4 numMs);

/*************************************************************************/
/*  Function Name: sp_cpu_taskStackInit                                  */
/*  Purpose:       Initialize relevant parameters in task stack.         */
/*  Arguments:     void* newTaskFcn:                                     */
/*                       Function pointer to task routine.               */
/*                 OS_STACK* sp:                                         */
/*                       Pointer to bottom of task stack (highest mem.   */
/*                       address).                                       */
/*  Return:        os_t_p_sp:                                            */
/*                       New stack pointer.                              */
/*************************************************************************/
OS_STACK* sp_cpu_taskStackInit(void (*newTaskFcn)(void), OS_STACK* sp);

/*************************************************************************/
/*  Function Name: vd_cpu_disableInterrupts                              */
/*  Purpose:       Enter critical section by disabling interrupts.       */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_cpu_disableInterrupts(void);

/*************************************************************************/
/*  Function Name: vd_cpu_enableInterrupts                               */
/*  Purpose:       Exit critical section by enabling interrupts.         */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_cpu_enableInterrupts(void);

/*************************************************************************/
/*  Function Name: u1_cpu_maskInterrupts                                 */
/*  Purpose:       Mask interrupts up to a specified priority.           */
/*  Arguments:     U1 setMask:                                           */
/*                    Interrupt priority mask.                           */
/*  Return:        ut_t_interruptMask: Previous interrupt mask.          */
/*************************************************************************/
U1 u1_cpu_maskInterrupts(U1 setMask);

/*************************************************************************/
/*  Function Name: vd_cpu_unmaskInterrupts                               */
/*  Purpose:       Restore previous interrupt mask.                      */
/*  Arguments:     U4 setMask:                                           */
/*                    Interrupt priority mask.                           */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_cpu_unmaskInterrupts(U1 setMask);

/*************************************************************************/
/*  Function Name: u4_cpu_getCurrentMsPeriod                             */
/*  Purpose:       Returns current scheduler period in ms.               */
/*  Arguments:     N/A                                                   */
/*  Return:        U4 u4_prev_periodMs                                   */
/*************************************************************************/
U4 u4_cpu_getCurrentMsPeriod(void);

/*************************************************************************/
/*  Function Name: vd_cpu_suspendScheduler                               */
/*  Purpose:       Turns off scheduler interrupts.                       */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_cpu_suspendScheduler(void);

/*************************************************************************/
/*  Function Name: vd_cpu_setNewSchedPeriod                              */
/*  Purpose:       Set scheduler interrupts to new speified period.      */
/*  Arguments:     U4 numMs:                                             */
/*                    Period for scheduler IRQ to be triggered.          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_cpu_setNewSchedPeriod(U4 numMs);

/*************************************************************************/
/*  Function Name: u1_cpu_getPercentOfTick                               */
/*  Purpose:       Return number of clock cycles done in current tick.   */
/*  Arguments:     N/A                                                   */
/*  Return:        U1: Number of clock cycles.                           */
/*************************************************************************/
U1 u1_cpu_getPercentOfTick(void);
 
/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif 
