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
#define STACK_FRAME_PSR_INIT          (0x01000000)
#define END_OF_REG_STACK_FRAME        (-16)
#define PSR_REGISTER_SLOT             (-1)
#define GENERAL_PURPOSE_REG_START     (-2)

/*************************************************************************/
/*  Data Structures                                                      */
/*************************************************************************/


/*************************************************************************/
/*  Global Variables, Constants                                          */
/*************************************************************************/
static clockReg reg_s_currentReloadVal;

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
  u4_periodMs         = (U4)ZERO;
  u1_intNestCounter   = (U1)ZERO;
  
  reg_s_currentReloadVal  = (U4)ZERO;
  SYSTICK_PRIORITY_SET_R |= (U1)OS_TICK_PRIORITY;
  PENDSV_PRIORITY_SET_R  |= (U1)PENDSV_PRIORITY;
  
  vd_cpu_disableInterruptsOSStart();
  vd_cpu_sysTickSet(numMs);
}

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
OS_STACK* sp_cpu_taskStackInit(void (*newTaskFcn)(void), OS_STACK* sp)
{
  S1        s1_t_index;
  OS_STACK *os_t_p_stackFrame;
  OS_STACK *os_t_p_sp;

  os_t_p_stackFrame = sp;
  
  /* Decrement to move upwards in stack */
  os_t_p_stackFrame[ZERO]                = (U4)STACK_FRAME_PSR_INIT; 
  os_t_p_stackFrame[PSR_REGISTER_SLOT]   = (U4)newTaskFcn;
  
  for(s1_t_index = (S1)GENERAL_PURPOSE_REG_START; s1_t_index > (S1)END_OF_REG_STACK_FRAME; --s1_t_index)
  {
    os_t_p_stackFrame[s1_t_index] = (U4)ZERO;
  }
  
   os_t_p_sp = &os_t_p_stackFrame[s1_t_index + ONE]; /* index is -16 at this point, want -15 */
  
  return (os_t_p_sp);
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
/*  Function Name: u1_cpu_getPercentOfTick                               */
/*  Purpose:       Return number of clock cycles done in current tick.   */
/*  Arguments:     N/A                                                   */
/*  Return:        Number of clock cycles.                               */
/*************************************************************************/
U1 u1_cpu_getPercentOfTick(void)
{
  U4 u4_t_calculation;
  
  if(reg_s_currentReloadVal != (U4)ZERO)
  {
    u4_t_calculation = (reg_s_currentReloadVal - (SYSTICK_24_BIT_MASK & SYSTICK_CURRENT_COUNT_R))*100;
    
    return(u4_t_calculation/reg_s_currentReloadVal);
  }
  else return ((U1)ZERO);
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
  
  u4_t_scale  = SYSTICK_CALBIRATION_R + ONE;
  u4_t_scale &= SYSTICK_24_BIT_MASK;
  
  /* Get scale to 1 ms */
  u4_t_scale = u4_t_scale/TIME_CAL_10_TO_1_MS;
  numMs      = numMs*u4_t_scale;
  
  /* SysTick overflow check */
  numMs &= (U4)SYSTICK_24_BIT_MASK;
  
  SYSTICK_CONTROL_R      &= ~(SYSTICK_DISABLED);            // 1) disable SysTick during setup 
  SYSTICK_RELOAD_R        = --numMs;                        // 2) Reload value 
  SYSTICK_CURRENT_COUNT_R = CPU_FALSE;                      // 3) any write to CURRENT clears it  
  SYSTICK_CONTROL_R      |= (U1)SYSTICK_CTRL_EXTERNAL_CLK;  // 4) enable SysTick with core clock
  reg_s_currentReloadVal  = numMs;
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
/* 0.5                6/22/19     API for CPU load calculation support.                        */
/*                                                                                             */
/*                                                                                             */
