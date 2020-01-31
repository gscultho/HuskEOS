# HuskEOS
Real-Time Operating System

## General Information
 * Priority-based preemptive scheduler. 
 * 2.3 - 6KB flash footprint. 
 * Originally developed for Arm Cortex-M4.
 * All memory statically allocated, no need for heap. Memory module supports "dynamic" memory emulation. 
 * Entire OS is hardware-agnostic aside from a small OS/CPU interface layer for porting. Application calls to hardware-dependent
   services are mapped through OS/CPU interface.  
 * Public modules designed to have similar API functionality and naming conventions for ease of use. 
 * Stack overflow detection supported for each task with configurable fault handlers. 
 * Configured Keil project included in repository. 
 
## Repository and Contact Information

  | Repository link  | https://github.com/gscultho/HuskEOS    |
  | :--------------- | :---------------                       |
  | Email            | gscultho@umich.edu                     |
  | :--------------- | :---------------                       |

## Modules
 * ### Scheduler
   * Task scheduling and state handling. 
   * Priority-based preemptive scheduler.
   * O(n) scheduler (n being the number of tasks in "wait" state) runs at configurable frequency. 
   * O(1) dispatcher for deterministic performance between system ticks. 
   * Supports some real-time debugging data, such as CPU load. 
   * Hook functions built in for modifications to OS behavior (i.e. when CPU goes to sleep/wakes up). 
  
 * ### Flags
   * Event flag objects. 
   * APIs support task blocking with optional timeout. Priority-based policy for task waking when multiple tasks are blocked on the same 
     object.  
   * Tasks can block on any event out of a set of specified events, or on an exact combination of events.  
  
 * ### Mailbox
   * Used to pass single pieces of data between tasks.  
   * APIs support task blocking with optional timeout. 
   * Data type passed through mailbox is configurable.
  
 * ### Memory
   * Emulates dynamic memory allocation. Allocates blocks of configurable size for use by application.   
   * Overflow/underflow (outside of memory block) detection supported.  
   * Work in progress at this time. 
  
 * ### Semaphore
   * Designed for counting and signaling purposes. For mutual exclusion see Mutex.    
   * APIs support task blocking with optional timeout and priority-based waking policy. 
   
 * ### Mutex
   * Mutual exclusion semaphore.    
   * Supports priority inheritance to eliminate possibility of priority inversion.
   * APIs support task blocking with optional timeout and priority-based waking policy. 
  
 * ### Queue
   * Fully configurable FIFO message queues.    
   * Configurable data type for messages.
   * APIs support task blocking with optional timeout and priority-based waking policy. 

## Current/Future Work
 * Support for multicore systems.
   * In design stage for supporting symmetric multiprocessing with optional core affinity. 
   * HuskEOS should presently be able to run in an asymmetric multiprocessing architecture on any number of cores, although this needs 
     to be tested. 
 * Memory module is currently being tested before final source is pushed. 
