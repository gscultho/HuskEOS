# HuskEOS
Real-Time Operating System

## General Information
⋅⋅* Priority-based preemptive scheduler. 
⋅⋅* 2.3 - 6KB flash footprint. 
⋅⋅* All memory statically allocated, no need for heap. Memory module supports "dynamic" memory emulation. 
⋅⋅* Entire OS is hardware-agnostic aside from a small OS/CPU interface layer for porting. Application calls to hardware-dependent services      are mapped through OS/CPU interface.  
⋅⋅* Public modules designed to have similar API functionality and naming conventions for ease of use. 
⋅⋅* Stack overflow detection supported. 

## Modules
   * ### Scheduler
   * One module for task scheduling and state handling. 
   * Priority-based preemptive scheduler.
   * O(n) scheduler (n being the number of tasks in "wait" state) runs at configurable period. 
   * O(1) dispatcher for deterministic performance in between system ticks. 
   * Supports some real-time debugging data, such as CPU load. 
   * Wake/sleep hook functions supported for before/after the CPU goes to sleep/wakes up when idle. 
  
 * ### Flags
   * Byte-sized event flag objects. 
   * Tasks can block on a flags object to wait for an event. Blocking timeout and indefinite blocking both supported by APIs. 
   * Tasks can block on any event out of a set of specified events, or on an exact combination of events.  
  
 * ### Mailbox
   * Used to pass single pieces of data between tasks.  
   * APIs support task blocking with configurable timeout. 
   * Data type passed through mailbox is easily configurable.
  
 * ### Memory
   * Emulates dynamic memory allocation. Allocates blocks of configurable size.   
   * Overflow/underflow (outside of memory block) detection supported.  
   * Work in progress at this time. 
  
 * ###Semaphore
   * Designed for counting and signaling purposes. For mutual exclusion see Mutex.    
   * APIs support task blocking with configurable timeout with priority-based policy for task waking when multiple tasks are blocked on      the same semaphore.  

