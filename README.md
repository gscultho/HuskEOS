# HuskEOS
Real-Time Operating System

## General Information
 * Priority-based preemptive scheduler. 
 * Achieves 50,000 context switches per second on Cortex-M4 configured at 16MHz. 
 * Counting/binary semaphores, mutexes, queues, mailboxes, flags, and dynamic memory emulator. All primitives and resources support optional thread blocking with deterministic priority-based waking policy. 
 * 2.3 - 6KB flash footprint. 
 * Memory (RW + RO + ZI) footprint can be configured as low as 550 bytes. 
 * All memory statically allocated, no need for heap. 
 * Entire OS is hardware-agnostic aside from an internal OS/CPU interface layer for porting. Can use same application code for different ports.  
 * Public modules designed to have consistent API structures and naming conventions for ease of use. 
 * Stack overflow detection supported for each task with configurable fault handlers. 
 * Runtime data collected for debugging (can be configured off for less overhead).
 
## Repository and Contact Information

  | Information      | Link                                   |
  | :--------------- | :---------------                       |
  | Repository link  | https://github.com/gscultho/HuskEOS    |
  | Email            | gscultho@umich.edu                     |
  | Email            | djcicala@umich.edu                     |

## Services
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
