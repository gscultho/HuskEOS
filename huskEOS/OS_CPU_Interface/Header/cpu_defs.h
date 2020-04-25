/*************************************************************************/
/*  File Name:  cpu_defs.h                                               */
/*  Purpose:    Definitions for OS hardware use.                         */
/*  Created by: Garrett Sculthorpe on 2/10/2019.                         */
/*  Copyright © 2019 Garrett Sculthorpe and Darren Cicala.               */
/*              All rights reserved.                                     */
/*************************************************************************/

#ifndef cpu_defs_h 
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
/* CPU Information */
#define OS_UWORD                      U4   
#define OS_SWORD                      S4 
#define STACK_DESCENDING              (0)
#define STACK_ASCENDING               (1)
#define OS_STACK                      OS_UWORD
#define STACK_GROWTH                  (STACK_DESCENDING)

/* General */
#define TWO                           (2)
#define ONE                           (1)
#define ZERO                          (0)
#define TEN                           (10)
#define NULL                          (0)
#define MAX_VAL_4BYTE                 ((U4)0xFFFFFFFF)

/* Registers used by OS */
#define SYS_REG_ICSR_ADDR             (*((volatile U4 *)0xE000ED04))
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


#endif 
