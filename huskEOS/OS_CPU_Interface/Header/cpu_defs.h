/*************************************************************************/
/*  File Name:  cpu_defs.h                                               */
/*  Purpose:    Definitions for OS hardware use.                         */
/*  Created by: Garrett Sculthorpe on 2/10/2019.                         */
/*************************************************************************/

#ifndef cpu_defs_h /* Protection from declaring more than once */
#define cpu_defs_h

/*************************************************************************/
/*  Data Types                                                           */
/*************************************************************************/
typedef unsigned char  U1;
typedef unsigned short U2;
typedef unsigned int   U4;
typedef unsigned long  U8;

typedef signed char    S1;
typedef short          S2;
typedef int            S4;
typedef signed long    S8;

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define OS_UWORD                      U4   
#define OS_SWORD                      S4 
#define OS_STACK                      OS_UWORD
#define CPU_BUS_WORD_SIZE             (32)
#define ZERO                          (0)
#define NULL                          (0)
#define SYS_REG_ICSR_ADDR             ((U4*)0xE000ED04)
#define NVIC_ST_CTRL_R                (*((volatile U4 *)0xE000E010)) 
#define NVIC_ST_RELOAD_R              (*((volatile U4 *)0xE000E014)) 
#define NVIC_ST_CURRENT_R             (*((volatile U4 *)0xE000E018)) 
#define NVIC_ST_CALIBRATE_R           (*((volatile U4 *)0xE000E01C))
#define NVIC_ST_PRIORITY_R            (*((volatile U1 *)0xE000ED23))
#define NVIC_PENDSV_PRIORITY_R        (*((volatile U1 *)0xE000ED22))


/*************************************************************************/
/*  Public Functions                                                     */
/*************************************************************************/


/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/


#endif /* End conditional declaration for cpu_defs_h */
