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

struct lock bus_direction_lock;
struct condition bus_direction_cond[2];
struct condition low_priority_waiting_cond;
int cars_on_tbe_bus;
int bus_direction;
int high_num;
int same_direction;
int other_direction;
int waiters[2];
int high_priority_waiters = 0;
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

    lock_init(&bus_direction_lock);
    cond_init(&low_priority_waiting_cond);
    cond_init(&bus_direction_cond[0]);
    cond_init(&bus_direction_cond[1]);
    waiters[0] = 0;
    waiters[1] = 0;
    high_priority_waiters = 0;
    bus_direction = 0;
    cars_on_tbe_bus = 0;
    high_num = 0;
    same_direction = 0;
    other_direction = 0;
    cars_on_tbe_bus = 0;
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
    lock_acquire(&bus_direction_lock);

    while (cars_on_tbe_bus >= 3 || (cars_on_tbe_bus != 0 && bus_direction != task.direction))
    {
        
        if (task.priority == HIGH)
        {
            high_priority_waiters++;
        }
        else
        {
            while (high_priority_waiters > 0)
            {
                cond_wait(&low_priority_waiting_cond, &bus_direction_lock);
            }
            //printf("Low p %d got, hwaiter:%d\n",task.priority,high_priority_waiters);
        }
        waiters[task.direction]++;
        cond_wait(&bus_direction_cond[task.direction], &bus_direction_lock);
        waiters[task.direction]--;
        if (task.priority == HIGH)
        {
            high_priority_waiters--;
        }
        if (high_priority_waiters == 0)
        {
            cond_broadcast(&low_priority_waiting_cond, &bus_direction_lock);
        }
    }

    //printf("Got Pri: %d\n", task.priority);
    cars_on_tbe_bus++;
    bus_direction = task.direction;
    lock_release(&bus_direction_lock);
    return;
}

/* task processes data on the bus send/receive */
void transferData(task_t task)
{
    int64_t randomNumber = (int64_t)random_ulong() % 3;
    randomNumber++;
    //printf("slee %d \n", randomNumber);
    timer_sleep(randomNumber);
}

/* task releases the slot */
void leaveSlot(task_t task)
{
    lock_acquire(&bus_direction_lock);
    cars_on_tbe_bus--;
    if (waiters[bus_direction] > 0)
    {
        cond_signal(&bus_direction_cond[bus_direction], &bus_direction_lock);
    }
    else if (cars_on_tbe_bus == 0)
    {
        cond_broadcast(&bus_direction_cond[bus_direction], &bus_direction_lock);
    };
    lock_release(&bus_direction_lock);
}
