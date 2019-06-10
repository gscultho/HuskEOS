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
; disable interrupts per mask given ONLY IF new mask is higher priority (lower value) than current mask
; inputs:  R0 - New BASEPRI mask
; outputs: R0 - Previous BASEPRI mask
MaskInterrupt  PROC
        MRS    R1, BASEPRI
        CMP    R1, #0
        BEQ    Mask                ;Note: BASEPRI = 0 means no mask currently
        CMP    R0, R1              ;      0x20 is mask of second highest priority, 0x40 third, 0x60 fourth,...
        BHI    DoNotMask
Mask    MSR    BASEPRI, R0
DoNotMask        
        MOV    R0, R1
        BX     LR
      ENDP
  
;*********** UnmaskInterrupt ***************
; disable interrupts per mask given
; inputs:  R0 - New BASEPRI mask
; outputs: none
UnmaskInterrupt  PROC
        MSR    BASEPRI, R0
        BX     LR
      ENDP
        
;*********** WaitForInterrupt ************************
; go to low power mode while waiting for the next interrupt
; inputs:  none
; outputs: none
WaitForInterrupt  PROC
        WFI
        BX     LR
      ENDP
            
;*********** OSTaskFault ************************
; Scheduler reached fault condition. Condition may be stack overflow detected.
; inputs:  none
; outputs: none
OSTaskFault   PROC
        EXPORT  OSTaskFault          [WEAK]
        B       .
      ENDP

      ALIGN 
            
      END
