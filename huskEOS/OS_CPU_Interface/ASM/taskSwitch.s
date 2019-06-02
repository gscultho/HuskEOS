;*************************************************************************/
;*  File Name: taskSwitch.s                                              */
;*  Purpose: Perform RTOS context switch.                                */
;*  Assembler: ARM Assembler, 5.03                                       */
;*  Created by: Garrett Sculthorpe on 2/29/19.                           */
;*************************************************************************/

    
        EXTERN  vd_g_p_currentTaskBlock
        EXTERN  vd_g_p_nextTaskBlock

;******************************************************************************
;Allocate Variables            
;******************************************************************************
        AREA |.data|, DATA, READWRITE 
        ALIGN

;******************************************************************************

        AREA |.TEXT|, CODE, READONLY, ALIGN=3
        THUMB
        PRESERVE8

;******************************************************************************
;******************************************************************************
;******************************************************************************

;******************************************************************************
;*  Routine: PendSV_Handler                                                   *
;*  Purpose: Perform context switch.                                          *
;*  Registers: N/A. Pushes and restores entire context.                       *
;******************************************************************************
PendSV_Handler PROC
                EXPORT  PendSV_Handler            [WEAK]    
                    CPSID   I  
                    
                    ;Save current task context and update task stack pointer in TCB
                    ;Note that the first entry in each TCB is its stack pointer
                    MOV     R0, SP
                    STMDB   R0!,{R4-R11}                          
                    LDR     R1, =vd_g_p_currentTaskBlock
                    LDR     R2, [R1]
                    STR     R0, [R2]
                    
                    ;Switch TCB pointers
                    LDR     R3, =vd_g_p_nextTaskBlock
                    LDR     R0, [R3]                     ; safe to use R5 at this point
                    STR     R0, [R1]
                    
                    ;Pull stack pointer from new task TCB and restore context
                    LDR     R0,  [R0]
                    LDMIA   R0!, {R4-R11}
                    MOV     SP,  R0     
                    LDR     LR,  =0xFFFFFFF9
                    CPSIE   I
                    
                    BX LR
              ENDP   
;******************************************************************************                    
        ALIGN
            
;******************************************************************************
;
; Useful functions. Taken from startup.s provided by Texas Instruments.
;
;******************************************************************************
        EXPORT  WaitForInterrupt
        EXPORT  MaskInterrupt
        EXPORT  UnmaskInterrupt
    
;*********** MaskInterrupt ***************
; disable interrupts per mask given
; inputs:  R0 - New BASEPRI mask
; outputs: R0 - Previous BASEPRI mask
MaskInterrupt
        MRS    R1, BASEPRI
        MSR    BASEPRI, R0
        MOV    R0, R1
        BX     LR
  
;*********** UnmaskInterrupt ***************
; disable interrupts per mask given
; inputs:  R0 - New BASEPRI mask
; outputs: none
UnmaskInterrupt
        MSR    BASEPRI, R0
        BX     LR
        
;*********** WaitForInterrupt ************************
; go to low power mode while waiting for the next interrupt
; inputs:  none
; outputs: none
WaitForInterrupt
        WFI
        BX     LR
        
        ALIGN 
            
        END