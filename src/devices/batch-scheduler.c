/* Tests cetegorical mutual exclusion with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */
#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "lib/random.h" //generate random numbers

#define BUS_CAPACITY 3
#define SENDER 0
#define RECEIVER 1
#define NORMAL 0
#define HIGH 1

// new varibles
struct semaphore bus_inuse;
struct semaphore bus_timeslot;

int bus_direction;
int high_num;
int same_direction;
int other_direction;
long priority_high_waiting = 0;
/*
 *	initialize task with direction and priority
 *	call o
 * */
typedef struct
{
    int direction;
    int priority;
} task_t;

void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
                    unsigned int num_priority_send, unsigned int num_priority_receive);

void senderTask(void *);
void receiverTask(void *);
void senderPriorityTask(void *);
void receiverPriorityTask(void *);

void oneTask(task_t task);      /*Task requires to use the bus and executes methods below*/
void getSlot(task_t task);      /* task tries to use slot on the bus */
void transferData(task_t task); /* task processes data on the bus either sending or receiving based on the direction*/
void leaveSlot(task_t task);    /* task release the slot */

/* initializes semaphores */
void init_bus(void)
{

    random_init((unsigned int)123456789);
    sema_init(&bus_inuse, 1);
    sema_init(&bus_timeslot, BUS_CAPACITY);

    bus_direction = 0;
    high_num = 0;
    same_direction = 0;
    other_direction = 0;
    //msg("NOT IMPLEMENTED");
    /* FIXME implement */
}

/*
 *  Creates a memory bus sub-system  with num_tasks_send + num_priority_send
 *  sending data to the accelerator and num_task_receive + num_priority_receive tasks
 *  reading data/results from the accelerator.
 *
 *  Every task is represented by its own thread. 
 *  Task requires and gets slot on bus system (1)
 *  process data and the bus (2)
 *  Leave the bus (3).
 */

void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
                    unsigned int num_priority_send, unsigned int num_priority_receive)
{
    unsigned int i;
    /* create sender threads */
    for (i = 0; i < num_tasks_send; i++)
        thread_create("sender_task", 1, senderTask, NULL);

    /* create receiver threads */
    for (i = 0; i < num_task_receive; i++)
        thread_create("receiver_task", 1, receiverTask, NULL);

    /* create high priority sender threads */
    for (i = 0; i < num_priority_send; i++)
        thread_create("prio_sender_task", 1, senderPriorityTask, NULL);

    /* create high priority receiver threads */
    for (i = 0; i < num_priority_receive; i++)
        thread_create("prio_receiver_task", 1, receiverPriorityTask, NULL);
}

/* Normal task,  sending data to the accelerator */
void senderTask(void *aux UNUSED)
{
    task_t task = {SENDER, NORMAL};
    oneTask(task);
}

/* High priority task, sending data to the accelerator */
void senderPriorityTask(void *aux UNUSED)
{
    task_t task = {SENDER, HIGH};
    oneTask(task);
}

/* Normal task, reading data from the accelerator */
void receiverTask(void *aux UNUSED)
{
    task_t task = {RECEIVER, NORMAL};
    oneTask(task);
}

/* High priority task, reading data from the accelerator */
void receiverPriorityTask(void *aux UNUSED)
{
    task_t task = {RECEIVER, HIGH};
    oneTask(task);
}

/* abstract task execution*/
void oneTask(task_t task)
{
    getSlot(task);
    transferData(task);
    leaveSlot(task);
}

/* task tries to get slot on the bus subsystem */
void getSlot(task_t task)
{

    while (task.direction != bus_direction)
        ;

    //printf("A task diraction:%d, priority:%d want to getSlot\n", task.direction, task.priority);
    if (task.priority == HIGH)
    {
        priority_high_waiting++;
    }
    else
    {
        while (priority_high_waiting != 0)
            ;
    }
    sema_down(&bus_timeslot);
    if (task.priority == HIGH)
    {
        priority_high_waiting--;
    }
    //printf("Priority: %d Get slot succeed, diraction:%d, remain slot: %d\n", task.priority, task.direction, bus_timeslot.value);

    /*
  if(task.direction != bus_direction){
    
    sema_down(&wakeup_lock);
    other_direction++;
    sema_up(&wakeup_lock);
    sema_down(&bus_inuse);

    sema_down(&number_lock);
    if(same_direction == 0){
      bus_direction = task.direction;
      sema_down(&wakeup_lock);
      for(int i = 0; i < other_direction-2; i++){
        sema_up(&bus_inuse);
        
      }
      other_direction = 0;
      sema_down(&wakeup_lock);
    }
    sema_up(&number_lock);
  }
  sema_down(&number_lock);
  same_direction++;
  sema_up(&number_lock);
  // number of same direction task

  if(task.priority == HIGH){
    // disable low prio task
    sema_down(&high_prio);
    if(high_num == 0){
      sema_down(&low_prio);
    }
    high_num++;
    sema_up(&high_prio);
    sema_down(&bus_timeslot);
  } else {
    sema_down(&low_prio);
    sema_down(&bus_timeslot);
    sema_up(&low_prio);
  }
  */
}

/* task processes data on the bus send/receive */
void transferData(task_t task)
{
    int64_t randomNumber = (int64_t)random_ulong() % 3;
    randomNumber++;
    //printf("slee %d \n",randomNumber);
    timer_sleep(randomNumber);
}

/* task releases the slot */
void leaveSlot(task_t task)
{
    //printf("A slot leaved\n");
    if (bus_timeslot.value < BUS_CAPACITY)
    {
        sema_up(&bus_timeslot);
        //printf("bus timeslot %d\n", bus_timeslot.value);
    }
    if (bus_timeslot.value >= BUS_CAPACITY)
    {
        if (bus_direction == 0)
        {

            bus_direction = 1;
            //printf("dir changed to %d\n", bus_direction);
        }
        else
        {

            bus_direction = 0;
            //printf("dir changed to %d\n", bus_direction);
        }
    }
}
