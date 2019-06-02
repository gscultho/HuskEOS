/*************************************************************************/
/*  File Name: cpu_os_interface.c                                        */
/*  Purpose: APIs for scheduler to interface with hardware.              */
/*  Compiler: ARM C/C++ Compiler, 5.03 [Build 76]                        */
/*  Created by: Garrett Sculthorpe on 2/12/19.                           */
/*************************************************************************/

/*************************************************************************/
/*  Includes                                                             */
/*************************************************************************/
#include "rtos_cfg.h"
#include "cpu_defs.h"
#include "cpu_os_interface.h"

/*************************************************************************/
/*  External References                                                  */
/*************************************************************************/
extern U1   MaskInterrupt(U1);
extern void UnmaskInterrupt(U1);

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define CPU_TRUE                      (1)
#define CPU_FALSE                     (0)
#define SYSTICK_CONTROL_R             (NVIC_ST_CTRL_R) 
#define SYSTICK_RELOAD_R              (NVIC_ST_RELOAD_R) 
#define SYSTICK_CURRENT_COUNT_R       (NVIC_ST_CURRENT_R) 
#define SYSTICK_CALBIRATION_R         (NVIC_ST_CALIBRATE_R)
#define SYSTICK_PRIORITY_SET_R        (NVIC_ST_PRIORITY_R)
#define PENDSV_PRIORITY_SET_R         (NVIC_PENDSV_PRIORITY_R)
#define TIME_CAL_10_TO_1_MS           (10)
#define SYSTICK_DISABLED              (0x00000007)
#define SYSTICK_24_BIT_MASK           (0x00FFFFFF)
#define INTERRUPT_NEST_COUNT_ZERO     (0)
#define SYSTICK_CTRL_EXTERNAL_CLK     (0x03)

/*************************************************************************/
/*  Data Structures                                                      */
/*************************************************************************/


/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/


/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
static void vd_cpu_sysTickSet(U4 numMs);


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/
static U4 u4_periodMs;
static U1 u1_intNestCounter;

/*************************************************************************/

/*************************************************************************/
/*  Function Name: vd_cpu_init                                           */
/*  Purpose:       Initialize scheduler interrupts.                      */
/*  Arguments:     U4 numMs:                                             */
/*                    Period for scheduler IRQ to be triggered.          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_cpu_init(U4 numMs)
{
  u4_periodMs         = ZERO;
  u1_intNestCounter   = ZERO;
  
  SYSTICK_PRIORITY_SET_R |= OS_TICK_PRIORITY;
  PENDSV_PRIORITY_SET_R  |= PENDSV_PRIORITY;
  vd_cpu_sysTickSet(numMs);
}

/*************************************************************************/
/*  Function Name: vd_cpu_disableInterrupts                              */
/*  Purpose:       Enter critical section by disabling interrupts.       */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
#pragma push
#pragma O0
void vd_cpu_disableInterrupts(void)
{
  U1 u1_t_newIntNestCntr;

  do
  {
    u1_t_newIntNestCntr = __ldrex(&u1_intNestCounter);
  }
  while(__strex((++u1_t_newIntNestCntr), &u1_intNestCounter));
  
  DisableInterrupts();
}
#pragma pop

/*************************************************************************/
/*  Function Name: vd_cpu_exitCritical                                   */
/*  Purpose:       Exit critical section by enabling interrupts.         */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
#pragma push
#pragma O0
void vd_cpu_enableInterrupts(void)
{
  U1 u1_t_newIntNestCntr;

  do
  {
    u1_t_newIntNestCntr = __ldrex(&u1_intNestCounter);
  }
  while(__strex((--u1_t_newIntNestCntr), &u1_intNestCounter));

  
  if(u1_t_newIntNestCntr == (U1)INTERRUPT_NEST_COUNT_ZERO)
  {
    EnableInterrupts();
  }
}
#pragma pop

/*************************************************************************/
/*  Function Name: u1_cpu_maskInterrupts                                 */
/*  Purpose:       Mask interrupts up to a specified priority.           */
/*  Arguments:     U4 setMask:                                           */
/*                    Interrupt priority mask.                           */
/*  Return:        ut_t_interruptMask: Previous interrupt mask.          */
/*************************************************************************/
U1 u1_cpu_maskInterrupts(U1 setMask)
{
  U1 ut_t_interruptMask;
  
  vd_cpu_disableInterrupts();
  
  ut_t_interruptMask = MaskInterrupt(setMask);
  
  vd_cpu_enableInterrupts();
  
  return (ut_t_interruptMask);
}

/*************************************************************************/
/*  Function Name: vd_cpu_unmaskInterrupts                               */
/*  Purpose:       Restore previous interrupt mask.                      */
/*  Arguments:     U4 setMask:                                           */
/*                    Interrupt priority mask.                           */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_cpu_unmaskInterrupts(U1 setMask)
{    
  vd_cpu_disableInterrupts();

  UnmaskInterrupt(setMask);
  
  vd_cpu_enableInterrupts();
}

/*************************************************************************/
/*  Function Name: u4_cpu_getCurrentMsPeriod                             */
/*  Purpose:       Returns current scheduler period in ms.               */
/*  Arguments:     N/A                                                   */
/*  Return:        U4 u4_prev_periodMs                                   */
/*************************************************************************/
U4 u4_cpu_getCurrentMsPeriod(void)
{
  return (u4_periodMs);
}

/*************************************************************************/
/*  Function Name: vd_cpu_suspendScheduler                               */
/*  Purpose:       Resets and turns off scheduler interrupts.            */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_cpu_suspendScheduler(void)
{
  SYSTICK_CONTROL_R &= ~(SYSTICK_DISABLED); 
}

/*************************************************************************/
/*  Function Name: vd_cpu_setNewSchedPeriod                              */
/*  Purpose:       Set scheduler interrupts to new speified period.      */
/*  Arguments:     U4 numMs:                                             */
/*                    Period for scheduler IRQ to be triggered.          */
/*  Return:        N/A                                                   */
/*************************************************************************/
void vd_cpu_setNewSchedPeriod(U4 numMs)
{
  vd_cpu_sysTickSet(numMs); 
}

/*************************************************************************/
/*  Function Name: vd_cpu_sysTickSe                                      */
/*  Purpose:       Configure SysTick registers.                          */
/*  Arguments:     U4 numMs:                                             */
/*                    Period for scheduler IRQ to be triggered.          */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void vd_cpu_sysTickSet(U4 numMs)
{
  U4 u4_t_scale;
  
  u4_t_scale  = SYSTICK_CALBIRATION_R + 1;
  u4_t_scale &= SYSTICK_24_BIT_MASK;
  
  /* Get scale to 1 ms */
  u4_t_scale = u4_t_scale/TIME_CAL_10_TO_1_MS;
  numMs      = numMs*u4_t_scale;
  
  /* SysTick overflow check */
  numMs &= (U4)SYSTICK_24_BIT_MASK;
  
  SYSTICK_CONTROL_R      &= ~(SYSTICK_DISABLED);       // 1) disable SysTick during setup 
  SYSTICK_RELOAD_R        = --numMs;                   // 2) Reload value 
  SYSTICK_CURRENT_COUNT_R = CPU_FALSE;                 // 3) any write to CURRENT clears it 
  
  SYSTICK_CONTROL_R |= (U1)SYSTICK_CTRL_EXTERNAL_CLK;  // 4) enable SysTick with core clock
}


/***********************************************************************************************/
/* History                                                                                     */
/***********************************************************************************************/
/* Version            Date        Description                                                  */
/*                                                                                             */
/* 0.1                3/25/19     Created module to add APIs for handling scheduler interrupt  */
/*                                frequency. Suspend, resume, setNew APIs added.               */
/* 0.2                4/1/19      Added APIs to pause and resume SysTick without resetting it. */
/* 0.3                5/29/19     Removed several APIs not needed.                             */
/* 0.4                5/30/19     Added APIs for handling interrupt enabling/masking           */ 
/*                                                                                             */
