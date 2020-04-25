

/* OS includes */
#include "sch.h"



/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/
#define OS_TICK_MS             (1)
#define OS_TASK_STACK_SIZE     (200)

#define OS_TASK1_PRIO          (1)
#define OS_TASK2_PRIO          (2)
#define OS_TASK3_PRIO          (3)

#define OS_TASK1_PERIOD        (1)
#define OS_TASK2_PERIOD        (5)
#define OS_TASK3_PERIOD        (10)

/*************************************************************************/
/*  Private Function Prototypes                                          */
/*************************************************************************/
static void app_task1(void);
static void app_task2(void);
static void app_task3(void);

/*************************************************************************/
/*  Global Variables                                                     */
/*************************************************************************/
static U4 u4_taskStack [OS_TASK_STACK_SIZE];
static U4 u4_taskStack2[OS_TASK_STACK_SIZE];
static U4 u4_taskStack3[OS_TASK_STACK_SIZE];

/*************************************************************************/

/*************************************************************************/
/*  Function Name: main                                                  */
/*  Purpose:       Control should be handed to RTOS.                     */
/*                 at end of function.                                   */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
int main()
{
  /* Initialize OS with tick period. */
  vd_OS_init(OS_TICK_MS);
  
  /* Init task 1. */
  u1_OSsch_createTask(&app_task1,                             /* Pointer to function definition. */
                     &u4_taskStack[OS_TASK_STACK_SIZE - 1],   /* Pointer to highest memory address in stack. */
                     sizeof(u4_taskStack),                    /* Size of stack. */
                     OS_TASK1_PRIO,                           /* Priority of task. */
                     OS_TASK1_PRIO);                          /* ID number of task for some API calls. Kept same as priority for simplicity. */
  /* Init task 2. */
  u1_OSsch_createTask(&app_task2,                             /* Pointer to function definition. */
                     &u4_taskStack2[OS_TASK_STACK_SIZE - 1],  /* Pointer to highest memory address in stack. */
                     sizeof(u4_taskStack2),                   /* Size of stack. */
                     OS_TASK2_PRIO,                           /* Priority of task. */
                     OS_TASK2_PRIO);                          /* ID number of task for some API calls. Kept same as priority for simplicity. */
  /* Init task 3. */
  u1_OSsch_createTask(&app_task3,                             /* Pointer to function definition. */
                     &u4_taskStack3[OS_TASK_STACK_SIZE - 1],  /* Pointer to highest memory address in stack. */
                     sizeof(u4_taskStack3),                   /* Size of stack. */
                     OS_TASK3_PRIO,                           /* Priority of task. */
                     OS_TASK3_PRIO);                          /* ID number of task for some API calls. Kept same as priority for simplicity. */
                     
  /* Hand control to OS, will not return. */                   
  vd_OSsch_start();
}
  
/*************************************************************************/
/*  Function Name: app_task1                                             */
/*  Purpose:                                                             */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void app_task1(void)
{
  U4 u4_t_sleepTime;

  u4_t_sleepTime = OS_TASK1_PERIOD;
  
  while(1)
  {
    //
    
    vd_OSsch_taskSleep(u4_t_sleepTime);
  }    
  
}

/*************************************************************************/
/*  Function Name: app_task2                                             */
/*  Purpose:                                                             */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void app_task2(void)
{
  U4 u4_t_sleepTime;
  
  u4_t_sleepTime = OS_TASK2_PERIOD;

  while(1)
  {
    //
    
    vd_OSsch_taskSleep(u4_t_sleepTime);
  }    
  
}

/*************************************************************************/
/*  Function Name: app_task3                                             */
/*  Purpose:                                                             */
/*  Arguments:     N/A                                                   */
/*  Return:        N/A                                                   */
/*************************************************************************/
static void app_task3(void)
{
  U4 u4_t_sleepTime;
  
  u4_t_sleepTime = OS_TASK3_PERIOD;

  while(1)
  {
    //
    
    vd_OSsch_taskSleep(u4_t_sleepTime);
  }    
  
}
